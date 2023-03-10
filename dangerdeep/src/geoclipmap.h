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

#pragma once

#define geoclipmap_fperv 4

#include "color.h"
#include "datadirs.h"
#include "fractal.h"
#include "frustum.h"
#include "height_generator.h"
#include "shader.h"
#include "simplex_noise.h"
#include "texture.h"
#include "vertexbufferobject.h"

#include <sstream>

class geoclipmap
{
  public:
    /// create geoclipmap data
    ///@param nr_levels - number of levels
    ///@param resolution_exp - power of two of resolution factor "N"
    ///@param hg - instance of height generator object
    geoclipmap(
        unsigned nr_levels,
        unsigned resolution_exp,
        height_generator& hg);

    /// d'tor
    ~geoclipmap();

    /// set/change viewer position
    void set_viewerpos(const vector3& viewpos);

    /// render the view (will only fetch the vertex/index data, no texture
    /// setup)
    void display(
        const frustum& f,
        const vector3& view_delta = vector3(),
        bool is_mirror            = false,
        int above_water           = 0) const;

  protected:
    // "N", must be power of two
    const unsigned resolution;         // resolution of triangles in VBO buffer
    const unsigned resolution_vbo;     // resolution of VBO buffer
    const unsigned resolution_vbo_mod; // resolution of VBO buffer - 1
    // distance between each vertex on finest level in real world space
    const double L;

    // resolution factor vertex to color
    // fixme: is this still needed?
    const unsigned color_res_fac;
    const unsigned log2_color_res_fac;

    // base viewerpos in 2d
    vector2 base_viewpos;

    // scratch buffer for VBO data, for transmission
    std::vector<float> vboscratchbuf;

    // scratch buffer for texture data, for transmission
    std::vector<vector3f> texnormalscratchbuf_3f;
    std::vector<uint8_t> texnormalscratchbuf;

    // scratch buffer for index generation, for transmission
    std::vector<uint32_t> idxscratchbuf;

    struct area
    {
        // bottom left and top right vertex coordinates
        // so empty area is when tr < bl
        vector2i bl, tr;

        area() : bl(0, 0), tr(-1, -1) { }
        area(const vector2i& a, const vector2i& b) : bl(a), tr(b) { }
        [[nodiscard]] area intersection(const area& other) const
        {
            return area(bl.max(other.bl), tr.min(other.tr));
        }
        [[nodiscard]] vector2i size() const
        {
            return {tr.x - bl.x + 1, tr.y - bl.y + 1};
        }
        [[nodiscard]] bool empty() const
        {
            vector2i sz = size();
            return sz.x <= 0 || sz.y <= 0;
        }
    };

    /// per-level data
    class level
    {
        geoclipmap& gcm;
        /// distance between samples of that level
        const double L_l;
        // resolution factor vertex to color
        const unsigned color_res_fac;
        const unsigned log2_color_res_fac;
        /// level index
        const unsigned index;
        /// vertex data
        vertexbufferobject vertices;
        /// store here for reuse ? we have 4 areas for indices...
        mutable vertexbufferobject indices;
        /// which coordinate area is stored in the VBO, in per-level coordinates
        area vboarea;
        /// offset in VBO of bottom left (vboarea.bl) data sample
        /// (dataoffset.x/y in [0...N] range)
        vector2i dataoffset;
        /// size of VBO data
        mutable unsigned vbo_data_size;

        mutable area tmp_inner, tmp_outer;
        bool outmost;

        unsigned generate_indices(
            const frustum& f,
            uint32_t* buffer,
            unsigned idxbase,
            const vector2i& offset,
            const vector2i& size,
            const vector2i& vbooff) const;
        unsigned generate_indices2(
            uint32_t* buffer,
            unsigned idxbase,
            const vector2i& size,
            const vector2i& vbooff) const;
        unsigned generate_indices_T(uint32_t* buffer, unsigned idxbase) const;
        unsigned
        generate_indices_horizgap(uint32_t* buffer, unsigned idxbase) const;
        void update_region(const geoclipmap::area& upar);
        void update_VBO_and_tex(
            const vector2i& scratchoff,
            int scratchmod,
            const vector2i& sz,
            const vector2i& vbooff);

        texture::ptr normals;
        texture::ptr colors;

      public:
        level(geoclipmap& gcm_, unsigned idx, bool outmost_level);
        area set_viewerpos(
            const vector3& new_viewpos,
            const geoclipmap::area& inner);
        void display(const frustum& f, bool is_mirror = false) const;
        texture& normals_tex() const { return *normals; }
        texture& colors_tex() const { return *colors; }
        void clear_area();
    };

    std::vector<std::unique_ptr<level>> levels;
    height_generator& height_gen;

    [[nodiscard]] int mod(int n) const { return n & resolution_vbo_mod; }

    [[nodiscard]] vector2i clamp(const vector2i& v) const
    {
        return {mod(v.x), mod(v.y)};
    }

    /*mutable*/ std::unique_ptr<glsl_shader_setup> myshader[2];
    // mutable glsl_shader_setup myshader_mirror;
    unsigned myshader_vattr_z_c_index[2];
    unsigned loc_texnormal[2];
    unsigned loc_texnormal_c[2];
    unsigned loc_w_p1[2];
    unsigned loc_w_rcp[2];
    unsigned loc_viewpos[2];
    unsigned loc_viewpos_offset[2];
    unsigned loc_xysize2[2];
    unsigned loc_L_l_rcp[2];
    unsigned loc_N_rcp[2];
    unsigned loc_texcshift[2];
    unsigned loc_texcshift2[2];
    unsigned loc_tex_stretch_factor[2];
    unsigned loc_above_water[2];
    unsigned loc_base_texture[2], loc_sand_texture[2], loc_grass_texture[2],
        loc_mud_texture[2], loc_forest_texture[2], loc_rock_texture[2],
        loc_snow_texture[2], loc_noise_texture[2], loc_forest_brdf_texture[2],
        loc_rock_brdf_texture[2];

    texture::ptr horizon_normal;
    texture::ptr noise_texture;

  public:
    bool wireframe; // for testing purposes only
};
