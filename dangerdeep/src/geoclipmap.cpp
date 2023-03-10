/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2020  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "geoclipmap.h"

#include "global_data.h"

#include <fstream>
#include <memory>

/*
Note: the geoclipmap renderer code can't handle levels < 0 yet, so no
extra geometry is rendered that is finer than the stored data and thus
generated. However there is no difference between using level=0 for some value
x=L or to use e.g. level=-y and L=x*2^-y. So the renderer doesn't care which
levels are real existing data or generated. Thus we could change the
height_generator interface again to use at least detail level 0 for geometry.
For normals and colors this would be the same, except that some minimum levels
are given (-1 for normals, and -1 or less for colors), or alternativly, detail
level 0 for normals delivers 2x2 the number of normals than geometry and level
0 is also minimum. Same for colors. The multiplicator of resolution geometry vs.
normals or geometry vs. colors may be also runtime configurable for the
geoclipmap renderer (request it from height generator). The factor would be a
power of two. Thus requests for detail level 0 give 2^p*2^p normals or colors
for each vertex, when p is the factor.
*/

#undef DEBUG_INDEX_USAGE
#define DEBUG_INDEX_EXTRA 1000

#define DYNAMIC_GROW_INDEX_VBO

geoclipmap::geoclipmap(
    unsigned nr_levels,
    unsigned resolution_exp,
    height_generator& hg) :
    resolution((1 << resolution_exp) - 2),
    resolution_vbo(1 << resolution_exp), resolution_vbo_mod(resolution_vbo - 1),
    L(hg.get_sample_spacing()),
    color_res_fac(1 << hg.get_log2_color_res_factor()),
    log2_color_res_fac(hg.get_log2_color_res_factor()),
    vboscratchbuf(
        (resolution_vbo + 2) * (resolution_vbo + 2)
        * geoclipmap_fperv), // 4 floats per VBO sample (x,y,z,zc)
    // ^ extra space for normals
    texnormalscratchbuf_3f(resolution_vbo * 2 * resolution_vbo * 2 * 3),
    texnormalscratchbuf(resolution_vbo * 2 * resolution_vbo * 2 * 3),
    idxscratchbuf(
        2 * (resolution_vbo + 4) * (resolution_vbo + 4) // patch triangles
        + 2 * 4 * resolution_vbo                        // T-junction triangles
        + 4 * 2 * resolution_vbo                        // outmost tri-fan
        + 32 * resolution_vbo // some extra rest for striping etc.
#ifdef DEBUG_INDEX_USAGE
        + DEBUG_INDEX_EXTRA
#endif
        ),
    levels(nr_levels), height_gen(hg), wireframe(false)
{
    // initialize vertex VBO and all level VBOs
    for (unsigned lvl = 0; lvl < levels.size(); ++lvl)
    {
        levels[lvl] =
            std::make_unique<level>(*this, lvl, lvl + 1 == levels.size());
    }

    myshader[0] = std::make_unique<glsl_shader_setup>(
        get_shader_dir() + "geoclipmap.vshader",
        get_shader_dir() + "geoclipmap.fshader");
    glsl_shader::defines_list defines;
    defines.push_back("MIRROR");
    myshader[1] = std::make_unique<glsl_shader_setup>(
        get_shader_dir() + "geoclipmap.vshader",
        get_shader_dir() + "geoclipmap.fshader",
        defines);

    // do not use too high w_fac with too small resolutions.
    // Otherwise the decaying transition factor (going from 1.0 at outer border
    // down to 0.0 at center), won't have reached 0.0 at inner border,
    // leading to visible gaps.

    const float w_fac = resolution_vbo < 128 ? 0.1f : 0.2f;
    for (unsigned i = 0; i < 2; ++i)
    {
        myshader[i]->use();
        myshader_vattr_z_c_index[i] =
            myshader[i]->get_vertex_attrib_index("z_c");
        loc_texnormal[i]   = myshader[i]->get_uniform_location("texnormal");
        loc_texnormal_c[i] = myshader[i]->get_uniform_location("texnormal_c");
        loc_w_p1[i]        = myshader[i]->get_uniform_location("w_p1");
        loc_w_rcp[i]       = myshader[i]->get_uniform_location("w_rcp");
        loc_viewpos[i]     = myshader[i]->get_uniform_location("viewpos");
        loc_viewpos_offset[i] =
            myshader[i]->get_uniform_location("viewpos_offset");
        loc_xysize2[i]    = myshader[i]->get_uniform_location("xysize2");
        loc_L_l_rcp[i]    = myshader[i]->get_uniform_location("L_l_rcp");
        loc_N_rcp[i]      = myshader[i]->get_uniform_location("N_rcp");
        loc_texcshift[i]  = myshader[i]->get_uniform_location("texcshift");
        loc_texcshift2[i] = myshader[i]->get_uniform_location("texcshift2");
        loc_tex_stretch_factor[i] =
            myshader[i]->get_uniform_location("tex_stretch_factor");
        loc_above_water[i] = myshader[i]->get_uniform_location("above_water");
        myshader[i]->set_uniform(loc_w_p1[i], resolution_vbo * w_fac + 1.0f);
        myshader[i]->set_uniform(loc_w_rcp[i], 1.0f / (resolution_vbo * w_fac));
        myshader[i]->set_uniform(loc_N_rcp[i], 1.0f / resolution_vbo);

        loc_base_texture[i] = myshader[i]->get_uniform_location("base_texture");
        loc_sand_texture[i] = myshader[i]->get_uniform_location("sand_texture");
        loc_grass_texture[i] =
            myshader[i]->get_uniform_location("grass_texture");
        loc_mud_texture[i] = myshader[i]->get_uniform_location("mud_texture");
        loc_forest_texture[i] =
            myshader[i]->get_uniform_location("forest_texture");
        loc_rock_texture[i] = myshader[i]->get_uniform_location("rock_texture");
        loc_snow_texture[i] = myshader[i]->get_uniform_location("snow_texture");
        loc_noise_texture[i] =
            myshader[i]->get_uniform_location("noise_texture");
        loc_forest_brdf_texture[i] =
            myshader[i]->get_uniform_location("forest_brdf_texture");
        loc_rock_brdf_texture[i] =
            myshader[i]->get_uniform_location("rock_brdf_texture");
    }
    // set a texture for normals outside coarsest level, just 0,0,1
    std::vector<uint8_t> pxl(3, 128);
    pxl[2]         = 255;
    horizon_normal = std::make_unique<texture>(
        pxl, 1, 1, GL_RGB, texture::LINEAR, texture::REPEAT);
}

geoclipmap::~geoclipmap() = default;

void geoclipmap::set_viewerpos(const vector3& new_viewpos)
{
    // check for a total reset of base_viewpos
    if (new_viewpos.xy().distance(base_viewpos) > 10000.0)
    {
        for (auto& level : levels)
        {
            level->clear_area();
        }
        base_viewpos = new_viewpos.xy();
    }

    // for each level compute clip area for that new viewerpos
    // for each level compute area that needs to get updated and do that

    // empty area for innermost level
    area levelborder;

    // compute lowest level number by height
    // draw level if new_viewpos.z <= 0.4 * resolution * L_l
    // L_l = L * 2^level
    // that is if log2(new_viewpos.z / (0.4*resolution*L)) <= level
    // so compute floor(log2(new_viewpos.z/(0.4*resolution*L))) as minimum level
    // fixme: later test to begin drawing at min_level
#if 1
    unsigned min_level = 0;
#else
    unsigned min_level = unsigned(
        std::max(floor(log2(new_viewpos.z / (0.4 * resolution * L))), 0.0));
#endif
    // log_debug("min_level=" << min_level);

    for (unsigned lvl = min_level; lvl < levels.size(); ++lvl)
    {
        levelborder = levels[lvl]->set_viewerpos(new_viewpos, levelborder);
        // next level has coordinates with half resolution
        // let outer area of current level be inner area of next level
        levelborder.bl.x /= 2;
        levelborder.bl.y /= 2;
        levelborder.tr.x /= 2;
        levelborder.tr.y /= 2;
    }

    myshader[0]->use();
    myshader[0]->set_uniform(loc_viewpos[0], new_viewpos - base_viewpos.xy0());
    myshader[0]->set_uniform(loc_viewpos_offset[0], new_viewpos);
    myshader[1]->use();
    myshader[1]->set_uniform(loc_viewpos[1], new_viewpos - base_viewpos.xy0());
    myshader[1]->set_uniform(loc_viewpos_offset[1], new_viewpos);
}

void geoclipmap::display(
    const frustum& f,
    const vector3& view_delta,
    bool is_mirror,
    int above_water) const
{
    if (wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    // display levels from inside to outside
    // unsigned min_level =
    // unsigned(std::max(floor(log2(new_viewpos.z/(0.4*resolution*L))), 0.0));
    unsigned si = is_mirror ? 1 : 0;
    myshader[si]->use();
    glPushMatrix();
    vector3 translation = base_viewpos.xy0() + view_delta;
    glTranslated(translation.x, translation.y, translation.z);
    frustum f2 = f;
    if (is_mirror)
    {
        f2 = f.get_mirrored();
    }
    myshader[si]->set_uniform(loc_above_water[si], above_water);

    myshader[si]->set_gl_texture(
        height_gen.get_sand_texture(), loc_sand_texture[si], 4);
    myshader[si]->set_gl_texture(
        height_gen.get_mud_texture(), loc_mud_texture[si], 5);
    myshader[si]->set_gl_texture(
        height_gen.get_forest_texture(), loc_forest_texture[si], 6);
    myshader[si]->set_gl_texture(
        height_gen.get_rock_texture(), loc_rock_texture[si], 7);
    myshader[si]->set_gl_texture(
        height_gen.get_snow_texture(), loc_snow_texture[si], 8);
    myshader[si]->set_gl_texture(
        height_gen.get_grass_texture(), loc_grass_texture[si], 9);
    myshader[si]->set_gl_texture(
        height_gen.get_rock_brdf_texture(), loc_rock_brdf_texture[si], 10);
    myshader[si]->set_gl_texture(
        height_gen.get_forest_brdf_texture(), loc_forest_brdf_texture[si], 11);
    myshader[si]->set_gl_texture(
        height_gen.get_noise_texture(), loc_noise_texture[si], 12);
    myshader[si]->set_gl_texture(
        height_gen.get_base_texture(), loc_base_texture[si], 3);

    for (unsigned lvl = 0; lvl < levels.size(); ++lvl)
    {
        myshader[si]->set_uniform(
            loc_tex_stretch_factor[si],
            height_gen.get_tex_stretch_factor() / pow(2.0, int(lvl)));
        myshader[si]->set_gl_texture(
            levels[lvl]->normals_tex(), loc_texnormal[si], 0);
        if (lvl + 1 < levels.size())
        {
            myshader[si]->set_gl_texture(
                levels[lvl + 1]->normals_tex(), loc_texnormal_c[si], 1);
        }
        else
        {
            myshader[si]->set_gl_texture(
                *horizon_normal, loc_texnormal_c[si], 1);
        }
        levels[lvl]->display(f2, is_mirror);
    }
    glPopMatrix();
    if (wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

geoclipmap::level::level(geoclipmap& gcm_, unsigned idx, bool outmost_level) :
    gcm(gcm_), L_l(gcm.L * double(1 << idx)), color_res_fac(gcm.color_res_fac),
    log2_color_res_fac(gcm.log2_color_res_fac), index(idx), vertices(false),
    indices(true), vbo_data_size(0), outmost(outmost_level)
{
    // mostly static...
    vertices.init_data(
        gcm.resolution_vbo * gcm.resolution_vbo * geoclipmap_fperv * 4
            + (outmost ? 8 * geoclipmap_fperv * 4 : 0),
        nullptr,
        GL_STATIC_DRAW);
    // size of T-junction triangles: 4 indices per triangle (3 + 2 degen. - 1),
    // 4*N/2 triangles max. size for normal triangles + T-junc.: 2*N^2 - 4*N +
    // 8*N - 16 = 2*N^2 + 4*N - 16 size for multiple columns: 2 per new column,
    // at most N columns, mostly less. however because of post-TL cache usage
    // and line-to-line degenerated triangles we need much more indices. Precise
    // numbers are hard to compute, so we take a value slightly larger than what
    // we need. it seems that on innermost level we need more than we give, but
    // the extra space for the outer tri-fan compensates it. If we would render
    // only one level, we could run out of space again. We need to compute the
    // exact maximum per level, fixme! Innermost levels needs much more than the
    // rest, so we have much waste here!
#ifndef DYNAMIC_GROW_INDEX_VBO
    indices.init_data(
        gcm.idxscratchbuf.size() * 4 /*GLuint*/, 0, GL_STATIC_DRAW);
#endif
    // create space for normal texture
    std::vector<uint8_t> pxl(
        3 * gcm.resolution_vbo * gcm.resolution_vbo * 2 * 2);
    normals = std::make_unique<texture>(
        pxl,
        gcm.resolution_vbo * 2,
        gcm.resolution_vbo * 2,
        GL_RGB,
        texture::LINEAR,
        texture::REPEAT);

    // fixme: is this still needed?
    if (color_res_fac == 2)
    {
        // reuse pxl
        memset(&pxl[0], index * 30, pxl.size());
    }
    else
    {
        pxl.clear();
        pxl.resize(
            color_res_fac * gcm.resolution_vbo * color_res_fac
                * gcm.resolution_vbo * 3,
            index * 30);
    }
}

auto geoclipmap::level::set_viewerpos(
    const vector3& new_viewpos,
    const geoclipmap::area& inner) -> geoclipmap::area
{
    // x_base/y_base tells offset in sample data according to level and
    // viewer position (new_viewpos)
    // this multiply with 0.5 then round then *2 lets the patches map to
    // "even" vertices and must be used to determine which patch number to
    // render. we need to make sure that clip area is at max N vertices! We use
    // a round function that always rounds in same direction, i.e. when 3.5 is
    // rounded up to 4.0, then -3.5 is rounded up to -3.0. this is done by using
    // round(x) := floor(x + 0.5) so when this gives -3.5....3.5 we take
    // floor(-3.0)....floor(4.0) -> -3...4
    area outer(
        vector2i(
            int(floor(0.5 * new_viewpos.x / L_l - 0.25 * gcm.resolution + 0.5))
                * 2,
            int(floor(0.5 * new_viewpos.y / L_l - 0.25 * gcm.resolution + 0.5))
                * 2),
        vector2i(
            int(floor(0.5 * new_viewpos.x / L_l + 0.25 * gcm.resolution + 0.5))
                * 2,
            int(floor(0.5 * new_viewpos.y / L_l + 0.25 * gcm.resolution + 0.5))
                * 2));
    tmp_inner = inner;
    tmp_outer = outer;
    // log_debug("index="<<index<<" area inner="<<inner.bl<<"|"<<inner.tr<<"
    // outer="<<outer.bl<<"|"<<outer.tr);
    // for vertex updates we only need to know the outer area...
    // compute part of "outer" that is NOT covered by old outer area,
    // this gives a rectangular or L-shaped form, but this can not be expressed
    // as area, only with at least 2 areas...
    if (vboarea.empty() || vboarea.intersection(outer).empty())
    {
        vboarea    = outer; // set this to make the update work correctly
        dataoffset = gcm.clamp(outer.bl);
        update_region(outer);
    }
    else
    {
        area outercmp       = outer;
        unsigned nr_updates = 0;
        if (outercmp.bl.y < vboarea.bl.y)
        {
            update_region(
                area(outercmp.bl, vector2i(outercmp.tr.x, vboarea.bl.y - 1)));
            outercmp.bl.y = vboarea.bl.y;
            ++nr_updates;
        }
        if (vboarea.tr.y < outercmp.tr.y)
        {
            update_region(
                area(vector2i(outercmp.bl.x, vboarea.tr.y + 1), outercmp.tr));
            outercmp.tr.y = vboarea.tr.y;
            ++nr_updates;
        }
        if (outercmp.bl.x < vboarea.bl.x)
        {
            update_region(
                area(outercmp.bl, vector2i(vboarea.bl.x - 1, outercmp.tr.y)));
            outercmp.bl.x = vboarea.bl.x;
            ++nr_updates;
        }
        if (vboarea.tr.x < outercmp.tr.x)
        {
            update_region(
                area(vector2i(vboarea.tr.x + 1, outercmp.bl.y), outercmp.tr));
            outercmp.tr.x = vboarea.tr.x;
            ++nr_updates;
        }
        if (nr_updates > 2)
        {
            THROW(error, "got more than 2 update regions?! BUG!");
        }
    }
    // we updated the vertices, so update area/offset
    dataoffset = gcm.clamp(outer.bl - vboarea.bl + dataoffset);
    vboarea    = outer;

    if (outmost)
    {
        // give 8 vertices to fill horizon gap
        static const int dx[8] = {-1, 0, 1, 1, 1, 0, -1, -1};
        static const int dy[8] = {-1, -1, -1, 0, 1, 1, 1, 0};
        for (unsigned i = 0; i < 8; ++i)
        {
            // 21km in x and y dir gives total length of < 30km
            gcm.vboscratchbuf[4 * i + 0] =
                new_viewpos.x + 21000 * dx[i] - gcm.base_viewpos.x;
            gcm.vboscratchbuf[4 * i + 1] =
                new_viewpos.y + 21000 * dy[i] - gcm.base_viewpos.y;
            gcm.vboscratchbuf[4 * i + 2] =
                0; // fixme: later give +- 10 for land/sea
            gcm.vboscratchbuf[4 * i + 3] = 0; // same value here
        }
        vertices.init_sub_data(
            gcm.resolution_vbo * gcm.resolution_vbo * geoclipmap_fperv * 4,
            8 * geoclipmap_fperv * 4,
            &gcm.vboscratchbuf[0]);
    }

    return outer;
}

void geoclipmap::level::update_region(const geoclipmap::area& upar)
{
    if (upar.empty())
    {
        THROW(error, "update area empty?! BUG!"); // continue; // can happen on
    }
    // initial update
    vector2i sz = upar.size();
    // height data coordinates upar.bl ... upar.tr need to be updated, but what
    // VBO offset? since data is stored toroidically in VBO, a rectangle can be
    // split into up to 4 rectangles... we need to call get_height function with
    // index as detail and the area as parameter... prepare a N*N height
    // lookup/scratch buffer, fill in w*h samples, then feed the correct samples
    // to the GPU maybe better prepare a VBO scratch buffer, maybe even one per
    // level (doesn't cost too much ram), and store the fixed VBO data there
    // that is x,y,height and height_c. the get_height function must write the
    // correct values then (with stride) or enters x,y,h_c as well. yes, if
    // get_height could give h and h_c at once, it would be better, and this can
    // easily be done. get_height: takes upar and a target buffer and enters
    // realx,realy,realz,realz_c z_c means z of coarser level, we must give some
    // helper values to generate realx/realy or write it ourselfs (maybe
    // better). data per vertex? 4 floats x,y,z,zc plus normal? or normal as
    // texmap? normal computation is not trivial!

    // update VBO toroidically

    // compute the heights first (+1 in every direction to compute normals too)
    vector2i upcrd = upar.bl + vector2i(-1, -1);
    // idea: fill in height here only from current level
    // fill in every 2nd x/y value by height of coarser level (we need m/2+1
    // values) interpolate missing heights manually here for that scratchbuffer
    // needs to get enlarged possibly it depends on wether upcrd.xy is odd or
    // even where to add lines for z_c computation later we need to compute n_c
    // as well, so we need even more lines. note! do not call "x / 2" for int x,
    // its more efficient to use "x >> 1"! It is even WRONG to call x / 2
    // sometimes, as -1 / 2 gives 0 and not -1 as the shift method does, when we
    // want to round down...
    gcm.height_gen.compute_heights(
        index,
        upcrd,
        sz + vector2i(2, 2),
        &gcm.vboscratchbuf[2],
        geoclipmap_fperv,
        geoclipmap_fperv * (sz.x + 2));
    unsigned ptr = 0;
    for (int y = 0; y < sz.y + 2; ++y)
    {
        vector2i upcrd2 = upcrd;
        for (int x = 0; x < sz.x + 2; ++x)
        {
            gcm.vboscratchbuf[ptr + 0] = upcrd2.x * L_l - gcm.base_viewpos.x;
            gcm.vboscratchbuf[ptr + 1] = upcrd2.y * L_l - gcm.base_viewpos.y;
            ptr += geoclipmap_fperv;
            ++upcrd2.x;
        }
        ++upcrd.y;
    }

    // get heights of coarser level, only for values inside original scratchbuf
    // area (without the +1 perimeter for normals). Note that upar.bl has always
    // even numbers here, as border to next coarser level has always even number
    // of vertices.
    upcrd.x =
        upar.bl.x >> 1; // need to shift here because values could be negative!
    upcrd.y = upar.bl.y >> 1;
    // sz.xy can be odd, so we must compute size with rounding up tr...
    const vector2i szc(
        ((upar.tr.x + 1) >> 1) - upcrd.x + 1,
        ((upar.tr.y + 1) >> 1) - upcrd.y + 1);
    unsigned ptr3 = ptr =
        ((sz.x + 2) * (1 - (upar.bl.y & 1)) + (1 - (upar.bl.x & 1)))
        * geoclipmap_fperv;
    gcm.height_gen.compute_heights(
        index + 1,
        upcrd,
        szc,
        &gcm.vboscratchbuf[ptr + 3],
        2 * geoclipmap_fperv,
        geoclipmap_fperv * (sz.x + 2) * 2);

    // interpolate z_c, first fill in missing columns on even rows
    for (int y = 0; y < szc.y; ++y)
    {
        unsigned ptr2 = ptr;
        for (int x = 0; x < szc.x - 1; ++x)
        {
            float f0 = gcm.vboscratchbuf[ptr2 + 3];
            float f1 = gcm.vboscratchbuf[ptr2 + 2 * geoclipmap_fperv + 3];
            gcm.vboscratchbuf[ptr2 + geoclipmap_fperv + 3] = (f0 + f1) * 0.5f;
            ptr2 += 2 * geoclipmap_fperv;
        }
        ptr += 2 * (sz.x + 2) * geoclipmap_fperv;
    }

    // interpolate z_c, first fill in missing rows
    /* fixme: this isn't always exactly the same as the video card renders, but
       it should be. This may be the cause for some terrain "popping" that can
       be seen on mountain tops, when watching them against the horizon/sky.
       Problem is that when a level is rendered like this:
       z0---z1
       |   /|
       |  / |
       | /  |
       |/   |
       z2---z3
       the z_c values of the next finer levels should be the same as the
       rendering, that is for the middle of that rectangle, z_c would be
       (z1+z2)/2 or if the triangulation is opposite it would be (z0+z3)/2. In
       fact this code computes it as (z0+z1+z2+z3)/4. The effect is hardly
       visible, but a bit ugly...
    */
    ptr = ptr3 + (sz.x + 2) * geoclipmap_fperv;
    for (int y = 0; y < szc.y - 1; ++y)
    {
        unsigned ptr2 = ptr;
        for (int x = 0; x < szc.x * 2 - 1; ++x)
        { // here we could spare 1 column
            float f0 =
                gcm.vboscratchbuf[ptr2 - (sz.x + 2) * geoclipmap_fperv + 3];
            float f1 =
                gcm.vboscratchbuf[ptr2 + (sz.x + 2) * geoclipmap_fperv + 3];
            gcm.vboscratchbuf[ptr2 + 3] = (f0 + f1) * 0.5f;
            ptr2 += geoclipmap_fperv;
        }
        ptr += 2 * (sz.x + 2) * geoclipmap_fperv;
    }

    // compute normals
    unsigned tptr = 0, tptr2 = 0;
    // log_debug("tex scratch sz="<<sz);
    // first retrieve vector3f normals, then transform them to RGB normals
    // index-1 because normals have double resolution as geometry
    gcm.height_gen.compute_normals(
        int(index) - 1, upar.bl * 2, sz * 2, &gcm.texnormalscratchbuf_3f[0]);
    for (int y = 0; y < sz.y * 2; ++y)
    {
        for (int x = 0; x < sz.x * 2; ++x)
        {
            const vector3f& nm = gcm.texnormalscratchbuf_3f[tptr2++];
            gcm.texnormalscratchbuf[tptr + 0] = uint8_t(nm.x * 127 + 128);
            gcm.texnormalscratchbuf[tptr + 1] = uint8_t(nm.y * 127 + 128);
            gcm.texnormalscratchbuf[tptr + 2] = uint8_t(nm.z * 127 + 128);
            tptr += 3;
        }
    }

    ptr = 0;
    geoclipmap::area vboupdate(
        gcm.clamp(upar.bl - vboarea.bl + dataoffset),
        gcm.clamp(upar.tr - vboarea.bl + dataoffset));
    // check for continuous update areas
    // log_debug("vboupdate area="<<vboupdate.bl<<" | "<<vboupdate.tr);
    // fixme: since texture/VBO updates are line by line anyway, we don't need
    // to handle the y-wrap here...
    if (vboupdate.tr.x < vboupdate.bl.x)
    {
        // area crosses VBO border horizontally
        // area 1 width gcm.resolution_vbo - bl.xy
        // area 2 width tr.xy + 1
#if 0
		if (vboupdate.tr.y < vboupdate.bl.y) {
			// area crosses VBO border horizontally and vertically
			int szx = gcm.resolution_vbo - vboupdate.bl.x;
			int szy = gcm.resolution_vbo - vboupdate.bl.y;
			update_VBO_and_tex(vector2i(0, 0), sz.x, vector2i(szx, szy), vboupdate.bl);
			update_VBO_and_tex(vector2i(szx, 0), sz.x, vector2i(vboupdate.tr.x + 1, szy),
					   vector2i(gcm.mod(vboupdate.bl.x + szx), vboupdate.bl.y));
			update_VBO_and_tex(vector2i(0, szy), sz.x, vector2i(szx, vboupdate.tr.y + 1),
					   vector2i(vboupdate.bl.x, gcm.mod(vboupdate.bl.y + szy)));
			update_VBO_and_tex(vector2i(szx, szy), sz.x, vector2i(vboupdate.tr.x + 1, vboupdate.tr.y + 1),
					   vector2i(gcm.mod(vboupdate.bl.x + szx), gcm.mod(vboupdate.bl.y + szy)));
		} else {
#endif
        // area crosses VBO border horizontally
        int szx = gcm.resolution_vbo - vboupdate.bl.x;
        update_VBO_and_tex(
            vector2i(0, 0), sz.x, vector2i(szx, sz.y), vboupdate.bl);
        update_VBO_and_tex(
            vector2i(szx, 0),
            sz.x,
            vector2i(vboupdate.tr.x + 1, sz.y),
            vector2i(gcm.mod(vboupdate.bl.x + szx), vboupdate.bl.y));
#if 0
		}
	} else if (vboupdate.tr.y < vboupdate.bl.y) {
		// area crosses VBO border vertically
		int szy = gcm.resolution_vbo - vboupdate.bl.y;
		update_VBO_and_tex(vector2i(0, 0), sz.x, vector2i(sz.x, szy), vboupdate.bl);
		update_VBO_and_tex(vector2i(0, szy), sz.x, vector2i(sz.x, vboupdate.tr.y + 1),
				   vector2i(vboupdate.bl.x, gcm.mod(vboupdate.bl.y + szy)));
#endif
    }
    else
    {
        // no border crossed
        update_VBO_and_tex(vector2i(0, 0), sz.x, sz, vboupdate.bl);
    }
}

void geoclipmap::level::update_VBO_and_tex(
    const vector2i& scratchoff,
    int scratchmod,
    const vector2i& sz,
    const vector2i& vbooff)
{
    /* fixme: CPU load is high, especially kernel part, could be cause of
       these calls to GL.
       We could at least reduce the glTexSubImage2D calls by copying the
       texture data to a consecutive block. But then we need to do y-clipping
       in the calling function.
       And we could try buffer-mapping instead of many glBufferSubData calls
    */
    // log_debug("update2 off="<<scratchoff<<" sz="<<sz<<" vbooff="<<vbooff);
    // copy data to normals texture
    // we need to do it line by line anyway, as the source is not packed.
    // maybe we can get higher frame rates if it would be packed...
    // test showed that this is negligible
    glActiveTexture(GL_TEXTURE1);
    normals->set_gl_texture();
    for (int y = 0; y < sz.y * 2; ++y)
    {
        // log_debug("update texture xy off
        // ="<<vbooff.x<<"|"<<gcm.mod(vbooff.y+y)<<"
        // idx="<<((scratchoff.y+y)*scratchmod+scratchoff.x)*3);
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0 /* mipmap level */,
            vbooff.x * 2,
            (vbooff.y * 2 + y) & (gcm.resolution_vbo_mod * 2 + 1),
            sz.x * 2,
            1,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            &gcm.texnormalscratchbuf
                 [((scratchoff.y * 2 + y) * scratchmod * 2 + scratchoff.x * 2)
                  * 3]);
    }
    // copy data to real VBO.
    // we need to do it line by line anyway.
    for (int y = 0; y < sz.y; ++y)
    {
        vertices.init_sub_data(
            (vbooff.x + gcm.mod(vbooff.y + y) * gcm.resolution_vbo)
                * geoclipmap_fperv * 4,
            sz.x * geoclipmap_fperv * 4,
            &gcm.vboscratchbuf
                 [((scratchoff.y + y + 1) * (scratchmod + 2) + 1 + scratchoff.x)
                  * geoclipmap_fperv]);
    }
}

static const int geoidx[2 * 4 * 6] = {
    0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 2, 0, 2, 1, 1, 1, 2, 0, 3, 0, 3, 1, 2, 1,
    3, 0, 0, 0, 0, 1, 3, 1, 0, 1, 1, 1, 2, 1, 3, 1, 0, 0, 3, 0, 2, 0, 1, 0,
};

// give real world offset (per-level coordinates) here as well as frustum-ref
auto geoclipmap::level::generate_indices(
    const frustum& f,
    uint32_t* buffer,
    unsigned idxbase,
    const vector2i& offset, // in per-level coordinates, global
    const vector2i& size,   // size of patch
    const vector2i& vbooff) const -> unsigned // offset in VBO
{
    // log_debug("genindi="<<index<<" size="<<size);
    if (size.x <= 1 || size.y <= 1)
    {
        return idxbase;
    }

    /* how clipping works:
       each patch forms a rectangle in xy-plane. Together with min. and max.
       height of the terrain this forms an axis aligned box (bounding box of
       patch). We create 6 polygons to represent that box and clip them by the
       viewing frustum. The resulting points are projected to the xy plane and
       their min. and max. xy coordinates are computed. That area is used to
       clip the patch area.
    */
    // box base points in ccw order
    vector2 cv[4] = {
        vector2(offset.x * L_l, offset.y * L_l),
        vector2((offset.x + size.x) * L_l, offset.y * L_l),
        vector2((offset.x + size.x) * L_l, (offset.y + size.y) * L_l),
        vector2(offset.x * L_l, (offset.y + size.y) * L_l)};
    double minmaxz[2];
    gcm.height_gen.get_min_max_height(minmaxz[0], minmaxz[1]);
    vector2 minv(1e30, 1e30), maxv(-1e30, -1e30);
    // if viewer is inside the bounding box, use its position as additional
    // bound point.
    const double eps = 0.5;
    if (f.viewpos.z > minmaxz[0] - eps && f.viewpos.z < minmaxz[1] + eps)
    {
        minv = maxv = f.viewpos.xy();
    }
    bool allempty = true;
    for (unsigned i = 0; i < 6; ++i)
    {
        polygon p = f.clip(polygon(
            cv[geoidx[i * 8 + 0]].xyz(minmaxz[geoidx[i * 8 + 1]]),
            cv[geoidx[i * 8 + 2]].xyz(minmaxz[geoidx[i * 8 + 3]]),
            cv[geoidx[i * 8 + 4]].xyz(minmaxz[geoidx[i * 8 + 5]]),
            cv[geoidx[i * 8 + 6]].xyz(minmaxz[geoidx[i * 8 + 7]])));
        if (!p.empty())
        {
            allempty = false;
            for (auto& point : p.points)
            {
                minv = minv.min(point.xy());
                maxv = maxv.max(point.xy());
            }
        }
    }

    if (allempty)
    {
        return idxbase;
    }

    // convert coordinates back to integer values with rounding down/up
    vector2i minvi(int(floor(minv.x / L_l)), int(floor(minv.y / L_l)));
    vector2i maxvi(int(ceil(maxv.x / L_l)), int(ceil(maxv.y / L_l)));
    vector2i newoffset = minvi, newsize = maxvi - minvi + vector2i(1, 1);
    // avoid size/offset to move out of what was given...
    // size seems to be at most 1 more than old size, offset at most 1 less.
    // this can be explained with float rounding errors.
    //	if (newoffset.x<offset.x||newoffset.y<offset.y)
    //		log_debug("offset diff, old="<<offset<<" new="<<newoffset);
    newoffset = newoffset.max(offset);
    // 	if (newsize.x>offset.x + size.x - newoffset.x||newsize.y>offset.y +
    // size.y - newoffset.y) 		log_debug("size diff,
    // old="<<offset+size-newoffset<<" new="<<newsize);
    newsize = newsize.min(offset + size - newoffset);
    // 	log_debug("clip area, vbooff="<<vbooff<<" now "<<vbooff + newoffset -
    // offset
    // 		  <<" size="<<size<<" now "<<newsize);
    // 	log_debug("offset shift: "<<newoffset-offset<<" sizechange:
    // "<<size-newsize);

    // modify vbooff accordingly (no clamping needed, is done later anyway)
    vector2i vbooff2 = vbooff + newoffset - offset;
    vector2i size2   = newsize;
    // check again if patch is still valid
    if (size2.x <= 1 || size2.y <= 1)
    {
        return idxbase;
    }

#if 1
    // for innermost level, when size2=(res_vbo-1,res_vbo-1), how many indices
    // do we need? per call we need (szx*2+2)*(res_vbo-2)
    // we have res_vbo/16 calls, while a call crosses the border one more,
    // and szx is 16+1 each call, so total number is:
    // (res_vbo/16)*(17*2+2)*(res_vbo-2) =
    // res_vbo^2 * 36/16 - res_vbo * 72/16 =
    // res_vbo^2 * 9/4 - res_vbo * 9/2 =
    // res_vbo^2 * 2.25 - res_vbo * 4.5
    // so the factor for the square of res_vbo is greater than 2, but we only
    // use 2 in resizing the VBO and thus we need to compensate with much
    // extra space. If we need to add one more call, that can happen
    // on borders, we would have even more demand for space.
    // at the moment for res_vbo=256 we have ca. 148000 indices per level, that
    // is 578kB per level, at 7 levels thus nearly 4MB used for the index vbo.
    // Since other levels need 3/4 of it, we waste at most 1MB, so no problem
    // for now. We could also increase the VBO size on demand, but that means
    // repeated new alloc of the VBO on the card.
    const unsigned colw = 17;
    unsigned cols       = (size2.x + colw - 2) / (colw - 1);
    unsigned coloff     = 0;
    unsigned result     = idxbase;
    for (unsigned c = 0; c < cols; ++c)
    {
        unsigned szx = std::min(colw, unsigned(size2.x) - coloff);
        result       = generate_indices2(
            buffer,
            result,
            vector2i(szx, size2.y),
            vector2i(vbooff2.x + coloff, vbooff2.y));
        coloff += colw - 1;
    }
    return result;
#else
    return generate_indices2(buffer, idxbase, size2, vbooff2);
#endif
}

// needed indices for this call:
// (size2.x*2+2)*(size2.y-1)
auto geoclipmap::level::generate_indices2(
    uint32_t* buffer,
    unsigned idxbase,
    const vector2i& size2,
    const vector2i& vbooff2) const -> unsigned
{
    // fixme: according to the profiler, this eats 70% of cpu time!!!
    // gcm.mod costs 3 instructions because of dereferencing...
    // better handle/inc buffer pointer directly instead of maintaining
    // ptr-variable! to get the address of buffer the gcc uses many strange
    // instructions! it seems the writing to the VBO costs most... not really,
    // when writing to a victim buffer, total cpu usage goes from 70% down to
    // 43%. So VBO writing is 1-43/70=39% of the cpu time. It is doubtful that
    // glBufferSubData would be faster... so replace gcm.mod, we can't do much
    // more... done, now cpu load 65%, but still high. it variies with geometric
    // detail or viewer height, so its hard to tell but current version has
    // least number of instructions in inner loop that can be generated with
    // gcc, for lower numbers one needs assembler.

    // always put the first index of a row twice and also the last
    uint32_t* bufptr                  = buffer + idxbase;
    const unsigned resolution_vbo_mod = gcm.resolution_vbo_mod;
    int vbooffy0                      = vbooff2.y & resolution_vbo_mod;
    int vbooffy1                      = (vbooffy0 + 1) & resolution_vbo_mod;
    for (int y = 0; y + 1 < size2.y; ++y)
    {
        int vbooffy0_l = vbooffy0 * gcm.resolution_vbo;
        int vbooffy1_l = vbooffy1 * gcm.resolution_vbo;
        int vbooffx0   = vbooff2.x & resolution_vbo_mod;
        // store first index twice (line or patch transition)
        *bufptr++ = vbooffy1_l + vbooffx0;
#if 0
		for (int x = 0; x < size2.x; ++x) {
			//gcc does strange things with this line!
			//it is indexing it via the loop counter etc., it cant be optimal
			*bufptr++ = vbooffy1_l + vbooffx0;
			*bufptr++ = vbooffy0_l + vbooffx0;
			vbooffx0 = (vbooffx0 + 1) & resolution_vbo_mod;
		}
#else
        // try a loop with pointers directly
        for (uint32_t* limit = bufptr + size2.x * 2; bufptr != limit;
             bufptr += 2)
        {
            bufptr[0] = vbooffy1_l + vbooffx0;
            bufptr[1] = vbooffy0_l + vbooffx0;
            vbooffx0  = (vbooffx0 + 1) & resolution_vbo_mod;
        }
#endif
        uint32_t lastidx = vbooffy0_l + ((vbooffx0 - 1) & resolution_vbo_mod);
        // store last index twice (line or patch transition)
        *bufptr++ = lastidx;
        vbooffy0  = vbooffy1;
        vbooffy1  = (vbooffy1 + 1) & resolution_vbo_mod;
    }
    return bufptr - buffer;
}

auto geoclipmap::level::generate_indices_T(uint32_t* buffer, unsigned idxbase)
    const -> unsigned
{
    unsigned ptr = idxbase;
    vector2i v   = dataoffset;
    for (unsigned i = 0; i < gcm.resolution / 2; ++i)
    {
        buffer[ptr] = buffer[ptr + 1] = v.x + v.y * gcm.resolution_vbo;
        v.y                           = gcm.mod(v.y + 1);
        buffer[ptr + 2]               = v.x + v.y * gcm.resolution_vbo;
        v.y                           = gcm.mod(v.y + 1);
        buffer[ptr + 3]               = v.x + v.y * gcm.resolution_vbo;
        ptr += 4;
    }
    for (unsigned i = 0; i < gcm.resolution / 2; ++i)
    {
        buffer[ptr] = buffer[ptr + 1] = v.x + v.y * gcm.resolution_vbo;
        v.x                           = gcm.mod(v.x + 1);
        buffer[ptr + 2]               = v.x + v.y * gcm.resolution_vbo;
        v.x                           = gcm.mod(v.x + 1);
        buffer[ptr + 3]               = v.x + v.y * gcm.resolution_vbo;
        ptr += 4;
    }
    for (unsigned i = 0; i < gcm.resolution / 2; ++i)
    {
        buffer[ptr] = buffer[ptr + 1] = v.x + v.y * gcm.resolution_vbo;
        v.y                           = gcm.mod(v.y - 1);
        buffer[ptr + 2]               = v.x + v.y * gcm.resolution_vbo;
        v.y                           = gcm.mod(v.y - 1);
        buffer[ptr + 3]               = v.x + v.y * gcm.resolution_vbo;
        ptr += 4;
    }
    for (unsigned i = 0; i < gcm.resolution / 2; ++i)
    {
        buffer[ptr] = buffer[ptr + 1] = v.x + v.y * gcm.resolution_vbo;
        v.x                           = gcm.mod(v.x - 1);
        buffer[ptr + 2]               = v.x + v.y * gcm.resolution_vbo;
        v.x                           = gcm.mod(v.x - 1);
        buffer[ptr + 3]               = v.x + v.y * gcm.resolution_vbo;
        ptr += 4;
    }
    return ptr;
}

auto geoclipmap::level::generate_indices_horizgap(
    uint32_t* buffer,
    unsigned idxbase) const -> unsigned
{
    // repeat last index for degenerated triangle conjunction. legal, because
    // there are always indices in buffer from former generate_indices_T.
    unsigned ptr  = idxbase;
    buffer[ptr++] = buffer[idxbase - 1];
    // extra vertices base index
    unsigned evb      = gcm.resolution_vbo * gcm.resolution_vbo;
    const vector2i sz = tmp_outer.size();
    vector2i offset   = gcm.clamp(tmp_outer.bl - vboarea.bl + dataoffset);
    vector2i offset2  = gcm.clamp(tmp_outer.tr - vboarea.bl + dataoffset);

    // bottom edge
    // we need to repeat first index for degenerate triangle conjunction, and to
    // rearrange the tri-strip order, we need to do it twice
    buffer[ptr++] = evb + 2;
    buffer[ptr++] = evb + 2;
    buffer[ptr++] = evb + 2;
    buffer[ptr++] = offset2.x + gcm.resolution_vbo * offset.y;
    // we only need half resolution here, as we are operating on the
    // outside, where we already have half resolution and also the T-junction
    // faces.
    for (int i = 2; i < sz.x; i += 2)
    {
        buffer[ptr++] = evb + 1;
        buffer[ptr++] = gcm.mod(offset2.x - i) + gcm.resolution_vbo * offset.y;
    }
    // left edge
    buffer[ptr++] = evb + 0;
    buffer[ptr++] = offset.x + gcm.resolution_vbo * offset.y;
    for (int i = 2; i < sz.x; i += 2)
    {
        buffer[ptr++] = evb + 7;
        buffer[ptr++] = offset.x + gcm.resolution_vbo * gcm.mod(offset.y + i);
    }
    // top edge
    buffer[ptr++] = evb + 6;
    buffer[ptr++] = offset.x + gcm.resolution_vbo * offset2.y;
    for (int i = 2; i < sz.x; i += 2)
    {
        buffer[ptr++] = evb + 5;
        buffer[ptr++] = gcm.mod(offset.x + i) + gcm.resolution_vbo * offset2.y;
    }
    // right edge
    buffer[ptr++] = evb + 4;
    buffer[ptr++] = offset2.x + gcm.resolution_vbo * offset2.y;
    for (int i = 2; i < sz.x; i += 2)
    {
        buffer[ptr++] = evb + 3;
        buffer[ptr++] = offset2.x + gcm.resolution_vbo * gcm.mod(offset2.y - i);
    }
    // final vertex to close it
    buffer[ptr++] = evb + 2;

    return ptr;
}

void geoclipmap::level::display(const frustum& f, bool is_mirror) const
{
    vector2i outszi = tmp_outer.size();
    // we have to decrease the region by 0.1 to avoid alpha values
    // near level transition that are not 1.0 but very close to it
    // values < 1.0 give visible gaps, especially for low N values.
    vector2f outsz = vector2f(outszi.x - 1.1f, outszi.y - 1.1f) * 0.5f;
    unsigned si    = is_mirror ? 1 : 0;
    gcm.myshader[si]->set_uniform(gcm.loc_xysize2[si], outsz);
    gcm.myshader[si]->set_uniform(gcm.loc_L_l_rcp[si], 1.0f / L_l);
    vector2 texdelta = gcm.base_viewpos * (1.0 / (L_l * gcm.resolution_vbo));
    vector2f texdelta_f(
        myfrac(texdelta.x) + 0.5 / gcm.resolution_vbo,
        myfrac(texdelta.y) + 0.5 / gcm.resolution_vbo);
    gcm.myshader[si]->set_uniform(gcm.loc_texcshift[si], texdelta_f);
    texdelta   = gcm.base_viewpos * (0.5 / (L_l * gcm.resolution_vbo));
    texdelta_f = vector2f(
        myfrac(texdelta.x) + 0.5 / gcm.resolution_vbo,
        myfrac(texdelta.y) + 0.5 / gcm.resolution_vbo);
    gcm.myshader[si]->set_uniform(gcm.loc_texcshift2[si], texdelta_f);

    uint32_t* indexvbo = &gcm.idxscratchbuf[0];
#ifdef DEBUG_INDEX_USAGE
    for (unsigned i = gcm.idxscratchbuf.size() - DEBUG_INDEX_EXTRA;
         i < gcm.idxscratchbuf.size();
         ++i)
        gcm.idxscratchbuf[i] = 0xdeadbeef;
#endif

    // we need to compute up to four rectangular areas made up of tri-strips.
    // they need to get clipped to the viewing frustum later.
    // the four areas form the ring or torus, after clipping they are still
    // rectangles. We should build a rectangle of several columns of rectangles,
    // each with a width of 16 or 32 quads, to make best use of the
    // post-transform vertex cache. So we need a function to generate indices
    // for a rectangular patch, input is area size and offset in vertex VBO for
    // wrapping.
    unsigned nridx = 0;
    if (tmp_inner.empty())
    {
        nridx = generate_indices(
            f, indexvbo, nridx, tmp_outer.bl, tmp_outer.size(), dataoffset);
    }
    else
    {
        // 4 columns: L,U,D,R (left, up, down, right)
        // LUR
        // L R
        // LDR
        // left colum (L)
        vector2i patchsz(
            tmp_inner.bl.x - tmp_outer.bl.x + 1,
            tmp_outer.tr.y - tmp_outer.bl.y + 1);
        vector2i off = tmp_outer.bl;
        nridx = generate_indices(f, indexvbo, nridx, off, patchsz, dataoffset);
        // lower/down column (D)
        patchsz.x = tmp_inner.tr.x - tmp_inner.bl.x + 1;
        patchsz.y = tmp_inner.bl.y - tmp_outer.bl.y + 1;
        vector2i patchoff(tmp_inner.bl.x - tmp_outer.bl.x, 0);
        off.x += patchoff.x;
        nridx = generate_indices(
            f, indexvbo, nridx, off, patchsz, gcm.clamp(dataoffset + patchoff));
        // upper column (U)
        patchsz.y  = tmp_outer.tr.y - tmp_inner.tr.y + 1;
        patchoff.y = tmp_inner.tr.y - tmp_outer.bl.y;
        off.y += patchoff.y;
        nridx = generate_indices(
            f, indexvbo, nridx, off, patchsz, gcm.clamp(dataoffset + patchoff));
        // right column (R)
        patchsz.x = tmp_outer.tr.x - tmp_inner.tr.x + 1;
        patchsz.y = tmp_outer.tr.y - tmp_outer.bl.y + 1;
        off.x -= patchoff.x;
        off.y -= patchoff.y;
        patchoff.x = tmp_inner.tr.x - tmp_outer.bl.x;
        patchoff.y = 0;
        off.x += patchoff.x;
        off.y += patchoff.y;
        nridx = generate_indices(
            f, indexvbo, nridx, off, patchsz, gcm.clamp(dataoffset + patchoff));
    }
    // add t-junction triangles - they are never clipped against viewing
    // frustum, but there are only few
    nridx = generate_indices_T(indexvbo, nridx);

    // add horizon gap triangles if needed
    if (outmost)
    {
        nridx = generate_indices_horizgap(indexvbo, nridx);
    }

#ifdef DEBUG_INDEX_USAGE
    if (nridx > gcm.idxscratchbuf.size())
        printf("ERROR %i %i\n", nridx, gcm.idxscratchbuf.size());
#endif

#ifdef DYNAMIC_GROW_INDEX_VBO
    if (nridx > vbo_data_size)
    {
        static const unsigned roundup = 8191;
        // printf("REALLOC VBO level=%u old=%u new=%u (%u) scratchsize=%u\n",
        // index, vbo_data_size, nridx, (nridx + roundup) & ~roundup,
        // gcm.idxscratchbuf.size());
        vbo_data_size = (nridx + roundup) & ~roundup;
        indices.init_data(
            vbo_data_size * 4 /*GLuint*/, nullptr, GL_STATIC_DRAW);
    }
#endif
    indices.init_sub_data(0, nridx * 4, &gcm.idxscratchbuf[0]);

    // render the data
    if (nridx < 4)
    {
        return; // first index is remove always, and we need at least 3 for one
    }
    // triangle
    vertices.bind();
    glVertexPointer(3, GL_FLOAT, geoclipmap_fperv * 4, (float*) nullptr + 0);
    glVertexAttribPointer(
        gcm.myshader_vattr_z_c_index[si],
        1,
        GL_FLOAT,
        GL_FALSE,
        geoclipmap_fperv * 4,
        (float*) nullptr + 3);
    glEnableVertexAttribArray(gcm.myshader_vattr_z_c_index[si]);
    vertices.unbind();
    indices.bind();
    // we always skip the first index here, because it is identical to the
    // second, that is because of the line/patch transition code (it is easier)
    glDrawRangeElements(
        GL_TRIANGLE_STRIP,
        0 /*min_vertex_index*/,
        gcm.resolution_vbo * gcm.resolution_vbo - 1 /*max_vertex_index*/,
        nridx - 1,
        GL_UNSIGNED_INT,
        (unsigned*) nullptr + 1); // skip first index
    indices.unbind();
    glDisableVertexAttribArray(gcm.myshader_vattr_z_c_index[si]);

#ifdef DEBUG_INDEX_USAGE
    unsigned s = gcm.idxscratchbuf.size();
    unsigned e = 0;
    for (unsigned i = s - 1; i > 0; --i)
        if (gcm.idxscratchbuf[i] != 0xdeadbeef)
        {
            e = i;
            break;
        }
    // printf("first index end %u of %u\n",e,s);
    if (e + DEBUG_INDEX_EXTRA >= s)
        printf("ERROR %u\n", e + DEBUG_INDEX_EXTRA - s);
#endif
}

void geoclipmap::level::clear_area()
{
    vboarea    = area();
    dataoffset = vector2i(0, 0);
}
