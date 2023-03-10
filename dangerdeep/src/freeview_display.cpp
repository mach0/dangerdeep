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

// user display: free 3d view
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "freeview_display.h"

#include "airplane.h"
#include "depth_charge.h"
#include "frustum.h"
#include "game.h"
#include "global_data.h"
#include "gun_shell.h"
#include "image.h"
#include "model.h"
#include "particle.h"
#include "postprocessor.h"
#include "primitives.h"
#include "ship.h"
#include "sky.h"
#include "submarine.h"
#include "system_interface.h"
#include "texture.h"
#include "torpedo.h"
#include "user_interface.h"
#include "water.h"
#include "water_splash.h"

#include <algorithm>
#include <fstream>
#include <utility>

using std::vector;

/* idea about depth fog:
   every parts below water surface should be rendered with fog, but fog color
   variies with depth (and daytime in general).
   To render fog with variable color, we would need one more texture unit.
   Models use already 4, although we could combine specular map and normal map
   to one unit, and then have one free unit.
   Fog above water surface would have different color by direction (sky color)
   and variies a bit with height, underwater fog depends only on depth, so
   we need a 2d texmap with colors.
   Problem is the geoclipmap rendering, it already uses 4 units with 3 channels
   each. We could combine it to 3 units with 4 channels to have one free unit,
   but then we would force normal resolution to be equal to color resolution,
   which is not very easy to have. A simple color gradient for depth fog can be
   made in the shaders without extra texture map, but it costs some shader
   instructions.
*/

void freeview_display::pre_display() const { }

auto freeview_display::get_projection_data(game& gm) const
    -> freeview_display::projection_data
{
    projection_data pd;
    pd.x     = 0;
    pd.y     = 0;
    pd.w     = SYS().get_res_x();
    pd.h     = SYS().get_res_y();
    pd.fov_x = 70.0;
    pd.near_z =
        0.2; // fixme: should be 1.0, but new conning tower needs 0.1 or so
    pd.far_z      = gm.get_max_view_distance();
    pd.fullscreen = true;
    return pd;
}

void freeview_display::set_modelview_matrix(
    game& gm,
    const vector3& /*viewpos*/) const
{
    glLoadIdentity();

    // set up rotation (player's view direction)
    glRotated(-ui.get_elevation().value(), 1, 0, 0);

    // if we're aboard the player's vessel move the world instead of the ship
    if (aboard)
    {
        // This should be a negative angle, but nautical view dir is clockwise,
        // OpenGL uses ccw values, so this is a double negation
        glRotated(ui.get_relative_bearing().value(), 0, 0, 1);
        gm.get_player()->get_orientation().conj().rotmat4().multiply_gl();
    }
    else
    {
        // This should be a negative angle, but nautical view dir is clockwise,
        // OpenGL uses ccw values, so this is a double negation
        glRotated(ui.get_absolute_bearing().value(), 0, 0, 1);
    }

    // set up modelview matrix as if player is at position (0, 0, 0), so do NOT
    // set a translational part. This is done to avoid rounding errors caused by
    // large x/y values (modelview matrix seems to store floats, but coordinates
    // are in real meters, so float is not precise enough).
}

void freeview_display::post_display() const
{
    SYS().prepare_2d_drawing();
    ui.draw_infopanel(true);
    SYS().unprepare_2d_drawing();
}

freeview_display::freeview_display(
    user_interface& ui_,
    const char* display_name) :
    user_display(ui_, display_name),
    aboard(false), withunderwaterweapons(true), drawbridge(false),
    conning_tower(nullptr)
{
    auto* sub     = dynamic_cast<submarine*>(ui_.get_game().get_player());
    add_pos       = sub->get_freeview_position();
    conning_tower = modelcache().ref(sub->get_bridge_filename());

    texturecache().ref("splashring.png");
    conning_tower->register_layout();
    conning_tower->set_layout();
    add_loading_screen("conning tower model loaded");
    // valgrind reports lost memory in the following line, but why?!
    std::unique_ptr<texture> uwbt(new texture(
        get_texture_dir() + "underwater_background.png",
        texture::LINEAR,
        texture::CLAMP));
    texturecache().ref("underwater_background.png", uwbt.get());
    underwater_background = uwbt.release();
}

freeview_display::~freeview_display()
{
    texturecache().unref("splashring.png");
    modelcache().unref(conning_tower);
    texturecache().unref(underwater_background);
}

auto freeview_display::get_viewpos(class game& gm) const -> vector3
{
    return gm.get_player()->get_pos() + add_pos;
}

void freeview_display::display() const
{
    // glClear or not, background drawing
    pre_display();

    // render scene
    // std::cout << "add_pos is " << add_pos << " playerpos " <<
    // gm.get_player()->get_pos() << " viewpos " << get_viewpos(gm) << " aboard:
    // " << aboard << "\n";
    draw_view(ui.get_game(), get_viewpos(ui.get_game()));

    // e.g. drawing of infopanel or 2d effects, background mask etc.
    post_display();
}

auto freeview_display::handle_key_event(const key_data& k) -> bool
{
    if (k.down())
    {
        glPushMatrix();
        glLoadIdentity();
        set_modelview_matrix(
            ui.get_game(),
            vector3()); // position doesn't matter, only direction.
        matrix4 viewmatrix = matrix4::get_gl(GL_MODELVIEW_MATRIX);
        glPopMatrix();
        vector3 sidestep = viewmatrix.row3(0);
        vector3 upward   = viewmatrix.row3(1);
        vector3 forward  = -viewmatrix.row3(2);

        switch (k.keycode)
        {
            case key_code::KP_8:
                add_pos -= forward * 15;
                break;
            case key_code::KP_2:
                add_pos += forward * 15;
                break;
            case key_code::KP_4:
                add_pos -= sidestep * 15;
                break;
            case key_code::KP_6:
                add_pos += sidestep * 15;
                break;
            case key_code::KP_1:
                add_pos -= upward * 15;
                break;
            case key_code::KP_3:
                add_pos += upward * 15;
                break;
            case key_code::KP_5:
                std::cout << ui.get_game().get_player()->get_pos() << std::endl;
                break;
            case key_code::w:
                ui.switch_geo_wire();
                break;
            default:
                break;
        }
        return true;
    }
    return false;
}

auto freeview_display::handle_mouse_motion_event(const mouse_motion_data& m)
    -> bool
{
    if (m.left())
    {
        ui.add_bearing(0.5 * m.relative_motion_2d.x);
        ui.add_elevation(-0.5 * m.relative_motion_2d.y);
        // fixme handle clamping of elevation at +-90deg
        return true;
    }
    return false;
}

auto freeview_display::handle_mouse_wheel_event(const mouse_wheel_data& m)
    -> bool
{
    glPushMatrix();
    glLoadIdentity();
    set_modelview_matrix(
        ui.get_game(), vector3()); // position doesn't matter, only direction.
    matrix4 viewmatrix = matrix4::get_gl(GL_MODELVIEW_MATRIX);
    glPopMatrix();
    vector3 forward = -viewmatrix.row3(2);

    if (m.up())
    {
        add_pos += forward * 15;
        return true;
    }
    else if (m.down())
    {
        add_pos -= forward * 15;
        return true;
    }
    return false;
}

void freeview_display::draw_objects(
    game& gm,
    const vector3& viewpos,
    const vector<const sea_object*>& objects,
    const colorf& light_color,
    const bool under_water,
    bool mirrorclip) const
{
    // simulate horizon: d is distance to object (on perimeter of earth)
    // z is additional height (negative!), r is earth radius
    // z = r*sin(PI/2 - d/r) - r
    // d = PI/2*r - r*arcsin(z/r+1), fixme implement

    sea_object* player = gm.get_player();

    for (auto object : objects)
    {
        bool istorp = (dynamic_cast<const torpedo*>(object) != nullptr);
        if (istorp && !withunderwaterweapons)
        {
            continue;
        }

        if (aboard && object == player)
        {
            continue;
        }
        glPushMatrix();

        if (mirrorclip && !istorp)
        {
            // viewpos.z is already mirrored...
            vector3 pos = object->get_pos();
            glTranslated(pos.x - viewpos.x, pos.y - viewpos.y, -viewpos.z);
            // orientation affects tex#1 matrix, for the code below
            glActiveTexture(GL_TEXTURE1);
            glMatrixMode(GL_TEXTURE);
            // fixme: this influences only model rendering or should at least
            // do only so. replace texture matrix use here!
            // this isn't easy as the texture matrix is silently set up as
            // modelview-like matrix and any commands to the modelview transform
            // that display() could call, go to the texture matrix.
            // the matrix is used for a double modelview transform, to transform
            // from object space to clip space via texture matrix, and from
            // there to eye space with the modelview matrix. we could implement
            // generic plane clipping and spare the matrix there... hmm it
            // inflicts geoclipmap rendering as well...
            glTranslated(0, 0, pos.z);
        }
        else
        {
            vector3 pos = object->get_pos() - viewpos;
            // pos.z += EARTH_RADIUS * (sin(M_PI/2 -
            // pos.xy().length()/EARTH_RADIUS) - 1.0);
            glTranslated(pos.x, pos.y, pos.z);
        }
        const ship* shp = dynamic_cast<const ship*>(object);
        if (shp)
        {
            shp->get_orientation().rotmat4().multiply_gl();
        }
        if (mirrorclip)
        {
            // torpedoes are normally fully underwater and thus need not to get
            // rendered for mirror images
            if (!istorp)
            {
                // finished modifying tex#1 matrix
                glMatrixMode(GL_MODELVIEW);
                object->display_mirror_clip();
            }
            // cleanup
            glActiveTexture(GL_TEXTURE1);
            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
        }
        else
        {
            object->display(
                under_water ? ui.get_caustics().get_map() : nullptr);
        }
        glPopMatrix();
    }

#if 0
	double tt = helper::mod(gm.get_time(), 10.0);
	water_splash wsp(vector3(), gm.get_time() - tt);
	glPushMatrix();
	glTranslatef(-viewpos.x, -viewpos.y, -viewpos.z);
	// fixme: alpha-wert mit der Zeit runterdrehen?
	// fixme: wenn alpha, dann nach allen anderen sea-objects rendern, oder
	// alle sea_objects mit alpha sortieren...
	wsp.display(gm.get_time());
	glPopMatrix();
#endif

    if (withunderwaterweapons)
    {
        auto depth_charges = gm.visible_depth_charges(player);
        for (auto it : depth_charges)
        {
            glPushMatrix();
            vector3 pos = it->get_pos() - viewpos;
            glTranslated(pos.x, pos.y, pos.z);
            glRotatef(-it->get_heading().value(), 0, 0, 1);
            it->display(under_water ? ui.get_caustics().get_map() : nullptr);
            glPopMatrix();
        }
    }

    auto gun_shells = gm.visible_gun_shells(player);
    for (auto it : gun_shells)
    {
        glPushMatrix();
        vector3 pos = it->get_pos() - viewpos;
        glTranslated(pos.x, pos.y, pos.z);
        glRotatef(-it->get_heading().value(), 0, 0, 1);
        it->display();
        glPopMatrix();
    }

    auto particles = gm.visible_particles(player);
    particle::display_all(particles, viewpos, gm, light_color);

    glDepthMask(GL_FALSE);
    // render all visible splashes. must alpha sort them, and not write to
    // z-buffer.
    auto water_splashes   = gm.visible_water_splashes(player);
    const auto& playerpos = player->get_pos().xy();
    std::sort(
        water_splashes.begin(),
        water_splashes.end(),
        [&playerpos](const auto* a, const auto* b) {
            return a->get_pos().xy().square_distance(playerpos)
                   > b->get_pos().xy().square_distance(playerpos);
        });

    // sort that array by square of distance to player, with std::sort and
    // compare function
    for (auto water_splashe : water_splashes)
    {
        glPushMatrix();
        vector3 pos = water_splashe->get_pos() - viewpos;
        glTranslated(pos.x, pos.y, pos.z);
        // rotational invariant.
        water_splashe->display();
        glPopMatrix();
    }
    glDepthMask(GL_TRUE);
}

void freeview_display::draw_view(game& gm, const vector3& viewpos) const
{
    double max_view_dist = gm.get_max_view_distance();

    // check if we are below water surface, above or near it
    int above_water   = 1; // 1: above, 0: near, -1: below
    float waterheight = ui.get_water().get_height(viewpos.xy());
    if (viewpos.z < waterheight)
    {
        above_water = -1;
    }

    // the modelview matrix is set around the player's viewing position.
    // i.e. it has an translation part of zero.
    // this means all objects have to be drawn with an offset of (-viewpos)
    // this is done because the position's values can be rather large (global
    // coordinates!) which leads to rounding errors when storing them in the
    // OpenGL matrix. So we have to compute them partly manually with high(er)
    // precision (double). the real viewing position (global coordinates) is
    // stored in viewpos.

    sea_object* player = gm.get_player();

    projection_data pd = get_projection_data(gm);

    // *************** compute and set player pos
    // ****************************************
    set_modelview_matrix(gm, viewpos);

    // **************** prepare drawing
    // ***************************************************

    GLfloat horizon_color[4] = {
        0.050980392156862744f,
        0.054901960784313725f,
        0.27450980392156865f,
        0.0f /*this is bad*/};
    ui.get_sky().rebuild_colors(
        gm.compute_sun_pos(viewpos), gm.compute_moon_pos(viewpos), viewpos);
    if (1 == above_water)
    {
        ui.get_sky().get_horizon_color(gm, viewpos).store_rgba(horizon_color);
    }

    // compute light source position and brightness (must be set AFTER modelview
    // matrix)
    vector3 sundir       = gm.compute_sun_pos(viewpos).normal();
    GLfloat lposition[4] = {
        static_cast<GLfloat>(sundir.x),
        static_cast<GLfloat>(sundir.y),
        static_cast<GLfloat>(sundir.z),
        0.0f};

    // get light color, previously all channels were uniform, so we'll make a
    // function of elevation to have some variation

    colorf lightcol = gm.compute_light_color(viewpos);

    // ambient light intensity depends on time of day, maximum at noon
    // max. value 0.35. At sun rise/down we use 0.11, at night 0.05
    float ambient_intensity =
        (std::max(sundir.z, -0.25) + 0.25) * 0.3 / 1.25 + 0.05;
    // ambient/diffuse/speculars should be dependeng on light color obviously,
    // so we'll take our new color function into account
    GLfloat lambient[4] = {
        ambient_intensity * lightcol.r,
        ambient_intensity * lightcol.g,
        ambient_intensity * lightcol.b,
        1.0f};
    GLfloat ldiffuse[4] = {lightcol.r, lightcol.g, lightcol.b, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, lambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, ldiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, ldiffuse);

    // ************************* compute visible surface objects
    // *******************

    // compute visble ships/subs, needed for draw_objects and amount of foam
    // computation
    const auto& objects = player->get_visible_objects();
    // fixme: the lookout sensor must give all ships seens around, not cull away
    // ships out of the frustum, or their foam is lost as well, even it would be
    // visible...

    // ********************* draw mirrored scene

    // ************ compute water reflection
    // ****************************************** in theory we have to set up a
    // projection matrix with a slightly larger FOV than the scene projection
    // matrix, because the mirrored scene can show parts that are not seen in
    // normal view. However we use the reflection texture coordinates also for
    // foam, which should match the scene projection, so we do NOT change the
    // projection matrix here. the (old) code for change is shown here:
    // --------- old begin ----------
    // save projection matrix
    // glMatrixMode(GL_PROJECTION);
    // glLoadIdentity();
    // store the projection matrix
    // SYS().gl_perspective_fovx(pd.fov_x * 1.0, 1.0 /*aspect*/, pd.near_z,
    // pd.far_z); reflection_projmvmat = matrix4::get_gl(GL_PROJECTION_MATRIX) *
    // matrix4::get_gl(GL_MODELVIEW_MATRIX); glLoadIdentity();
    // set up projection matrix (new width/height of course) with a bit larger
    // fov
    // SYS().gl_perspective_fovx(pd.fov_x * 1.0, 1.0 /*aspect*/, pd.near_z,
    // pd.far_z);
    // -------- old end ------------

    ui.get_water().refltex_render_bind();

    // shear one clip plane to match world space z=0 plane
    // fixme, use shaders for that, Clip planes are often computed in software
    // and are too slow the question is how they interfere with shaders? yes
    // they are damn slow. give height of object (pos.z) to shader, it then
    // clips at -z. no, instead compute plane to clip to and clip in shader.
    // either clip to arbitrary plane (z=0 water plane in world space is not z=0
    // in eye space) or transform object space to world space, then clip at z=0
    // and then transform to eye space.

    {
        glPushMatrix();
        // flip geometry at z=0 plane
        glScalef(1.0f, 1.0f, -1.0f);
        glCullFace(GL_FRONT);

        // viewpos for drawing mirrored objects should/must be changed
        // to (vp.x, vp.y, -vp.z) (z coordinate negated).
        vector3 viewpos_mirror = vector3(viewpos.x, viewpos.y, -viewpos.z);

        // flip light!
        glLightfv(GL_LIGHT0, GL_POSITION, lposition);

        // draw all parts of the scene that are (partly) above the water:
        //   sky
        ui.get_sky().display(
            gm.compute_light_color(viewpos_mirror),
            viewpos_mirror,
            max_view_dist,
            true);

        glPopMatrix();

        //   terrain - it handles z-flipping itself
        ui.draw_terrain(
            viewpos_mirror,
            ui.get_absolute_bearing(),
            max_view_dist,
            true /*mirrored*/,
            above_water);

        glPushMatrix();
        // flip geometry at z=0 plane
        glScalef(1.0f, 1.0f, -1.0f);

        // fixme
        //   models/smoke
        // test hack. limit mirror effect only to near objects?
        // the drawn water is nearly a flat plane in the distance so mirroring
        // would be perfect which is highly unrealistic.
        // so remove entries that are too far away. Torpedoes can't be seen
        // so they don't need to get rendered.
        vector<const sea_object*> objects_mirror;
        objects_mirror.reserve(objects.size());
        const double MIRROR_DIST = 1000.0; // 1km or so...
        for (auto object : objects)
        {
            if (object->get_pos().xy().square_distance(viewpos.xy())
                < MIRROR_DIST * MIRROR_DIST)
            {
                objects_mirror.push_back(object);
            }
        }
        draw_objects(
            gm,
            viewpos_mirror,
            objects_mirror,
            lightcol,
            false /* under_water */,
            true /* mirror */);

        glCullFace(GL_BACK);

        glPopMatrix();
    }
    ui.get_water().refltex_render_unbind();

#if 0
	unsigned vps=512;
	vector<uint8_t> scrn(vps*vps*3);
	//unsigned t0 = SDL_GetTicks();
	glReadPixels(0, 0, vps, vps, GL_RGB, GL_UNSIGNED_BYTE, &scrn[0]);
	//unsigned t1 = SDL_GetTicks();
	//cout << "time for " << vps << " size tex: " << t1-t0 << "\n"; // 14-20 ms each, ~1-2ms for a 128pixel texture
	scrn[0] = 255;//test to see where is up
	std::ofstream oss("mirror.ppm");
	oss << "P6\n" << vps << " " << vps << "\n255\n";
	oss.write((const char*)(&scrn[0]), vps*vps*3);
#endif
    // ********* set fog for scene
    // ****************************************************
    glFogi(GL_FOG_MODE, GL_EXP);
    glFogfv(GL_FOG_COLOR, horizon_color);
    // values for fog density:
    // 0.0005 - 0.002 good weather, higher numbers give more haze
    if (1 == above_water)
    {
        glFogf(GL_FOG_DENSITY, 0.0005); // not used in linear mode
        glFogf(
            GL_FOG_START, max_view_dist * 0.75); // ships disappear earlier :-(
        glFogf(GL_FOG_END, max_view_dist);
    }
    else
    {
        float underwater_fog = 100.0f; // FIXME
        glFogf(GL_FOG_DENSITY, 0.005); // not used in linear mode
        glFogf(
            GL_FOG_START, underwater_fog * 0.75); // ships disappear earlier :-(
        glFogf(GL_FOG_END, underwater_fog);
    }

    glEnable(GL_FOG);

    // *************************** compute amount of foam for water display
    // *****************

    // compute foam values for water.
    // give pointers to all visible ships for foam calculation, that are all
    // ships, subs and torpedoes. Gun shell impacts/dc explosions will be added
    // later...
    // fixme: foam generated depends on depth of sub and type of torpedo etc.
    vector<const ship*> allships;
    allships.reserve(objects.size());
    for (auto object : objects)
    {
        auto* s = dynamic_cast<const ship*>(object);
        if (s)
        {
            auto* t = dynamic_cast<const torpedo*>(s);
            if (!t)
            {
                // do NOT store torpedoes here, because they have no foam trail,
                // they travel under water.
                // the bubble trail of G7a torpedoes is another story though.
                // But this routine will render wide trails dependant on speed,
                // which is only correct for surface ships.
                // fixme: for submerged subs we must not draw the trail, too.
                // fixme2: even more complicated, periscopes/snorkels cause
                // much less foam too...
                allships.push_back(s);
            }
        }
    }
    ui.get_water().compute_amount_of_foam_texture(gm, viewpos, allships);

#if 0
	unsigned vps=512;
	vector<uint8_t> scrn(vps*vps*3);
	//unsigned t0 = SDL_GetTicks();
	glReadPixels(0, 0, vps, vps, GL_RGB, GL_UNSIGNED_BYTE, &scrn[0]);
	//unsigned t1 = SDL_GetTicks();
	//cout << "time for " << vps << " size tex: " << t1-t0 << "\n"; // 14-20 ms each, ~1-2ms for a 128pixel texture
	scrn[0] = 255;//test to see where is up
	ofstream oss("mirror.ppm");
	oss << "P6\n" << vps << " " << vps << "\n255\n";
	oss.write((const char*)(&scrn[0]), vps*vps*3);
#endif

    // ************************** draw the real scene
    // ********************************

    postprocessor::instance().render2texture();

    glViewport(pd.x, pd.y, pd.w, pd.h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    SYS().gl_perspective_fovx(
        pd.fov_x, double(pd.w) / double(pd.h), pd.near_z, pd.far_z);
    glMatrixMode(GL_MODELVIEW);

    frustum viewfrustum = frustum::from_opengl();
    matrix4 mv          = matrix4::get_gl(GL_MODELVIEW_MATRIX);
    matrix4 prj         = matrix4::get_gl(GL_PROJECTION_MATRIX);
    matrix4 mvp         = prj * mv;
    matrix4 invmvp      = mvp.inverse();
    vector3 wblf        = invmvp * vector3(-1, -1, +0.99);
    vector3 wbrf        = invmvp * vector3(+1, -1, +0.99);
    vector3 wtlf        = invmvp * vector3(-1, +1, +0.99);
    vector3 wtrf        = invmvp * vector3(+1, +1, +0.99);
    polygon viewwindow_far(wblf, wbrf, wtrf, wtlf);

    // fixme: water reflections are brighter than the sky, so there must be a
    // difference between sky drawing and mirrored sky drawing... yes, because
    // sky is blended into background, and that color is different. mirror scene
    // background color is horizon color, so that in the mirrored image the
    // background pixels don't bleed into the sky/horizon pixels... shouldn't
    // matter anyway because of water fog... test this

    // Note! glClear() deletes whole buffer, not only viewport.
    // use glScissor with glClear here or clear with own command.
    if (pd.fullscreen)
    {
        //		glClearColor(0.015, 0.01, 0.055, 0);
        glClearColor(0.0, 0.0, 0.0, 0);
        // this color clear eats ~ 2 frames (52 to 50 on a gf4mx), but is needed
        // for star sky drawing
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    }
    else
    {
        glScissor(pd.x, pd.y, pd.w, pd.h);
        glEnable(GL_SCISSOR_TEST);
        //		glClearColor(0.015, 0.01, 0.055, 0);
        glClearColor(0.0, 0.0, 0.0, 0);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);
    }

    // set light!
    glLightfv(GL_LIGHT0, GL_POSITION, lposition);

    // ************ sky
    // ***************************************************************
    if (above_water >= 0)
    {
        ui.get_sky().display(
            gm.compute_light_color(viewpos), viewpos, max_view_dist, false);
    }

    // ******* water
    // ***************************************************************
    // ui.get_water().update_foam(1.0/25.0);  //fixme: deltat needed here
    // ui.get_water().spawn_foam(vector2(helper::mod(gm.get_time(),256.0),0));
    /* to render water below surface correctly, we have to do here:
       - switch to front culling when below the water surface
       - cull nothing if we are near the surface and we can see sky AND
       underwater space We have three spaces: above water, under water, and near
       water surface. we are above water, when viewer position is above water
       surface and the intersection of water surface with z-near plane is a line
       outside the viewing frustum. we are under water, if viewer position is
       under the surface and the rest like above water. we are near the surface
       else. above water: cull back-faces, nothing special (current rendering)
       under water:
       render deep blue background instead of stars and sky, render objects with
       deep blue close fog (and maybe also depth layered fog, and maybe
       caustics). add some specials like bubbles etc. near water surface: we
       have to disable face culling, so both sides are rendered. we have to draw
       stars/sky and also blue backplane under water. That is, render a polygon
       that is limited by horizon line and frustum. Render all objects with
       special shader that does fog when z is below zero. That is not very
       accurate, but we have no choice yet. for underwater rendering the shader
       is much different, so we may have to render the water twice, once with
       glCullFace(GL_BACK) and once with glCullFace(GL_FRONT) but each time with
       different shader. under water reflections are rather constant color and
       refractions depend on sun diffuse lighting, but normals and sun vector is
       reversed. This also means that when we are under water, we dont need to
       render the mirrored scene and save some computation time. we don't need
       to render foam as well then.
     */
    // under water...
    if (above_water <= 0)
    {
        // render water background plane
        // clip far plane frustum polygon with z=0 plane (water surface)
        //		polygon uwp = viewwindow_far.clip(plane(vector3(0, 0, -1), 0));
        // render polygon with tri-fan
        //		const double underwater_bg_maxz = -40;

        //		glDisable(GL_DEPTH_TEST);
        //		primitives trifan(GL_TRIANGLE_FAN, uwp.points.size(),
        ///**underwater_background*/ colorf(0.0f,0.0f,0.0f));
        /*		for (unsigned i = 0; i < uwp.points.size(); ++i) {
                    trifan.vertices[i].assign(uwp.points[i]);
                    trifan.texcoords[i].x = uwp.points[i].z/underwater_bg_maxz;
                }*/
        //		trifan.render();
        //		glEnable(GL_DEPTH_TEST);
        glCullFace(GL_FRONT);
        ui.get_water().display(viewpos, max_view_dist, true /* under water*/);
        glCullFace(GL_BACK);
    }
    else
    {
        ui.get_water().display(viewpos, max_view_dist);
    }

    // ******** terrain/land
    // ********************************************************
    //	glDisable(GL_FOG);	//testing with new 2d bspline terrain.
    ui.draw_terrain(
        viewpos,
        ui.get_absolute_bearing(),
        max_view_dist,
        false /*not mirrored*/,
        above_water);
    //	glEnable(GL_FOG);

    // ******************** ships & subs
    // *************************************************
    //	cout << "mv trans pos " <<
    // matrix4::get_gl(GL_MODELVIEW_MATRIX).column(3) << "\n";

    // substract player pos.
    draw_objects(
        gm,
        viewpos,
        objects,
        lightcol,
        (above_water < 0) ? true : false /* under water */,
        false /* mirrorclip */);

    // ******************** draw the bridge in higher detail
    if (aboard && drawbridge)
    {
        // after everything was drawn, draw conning tower
        vector3 conntowerpos = player->get_pos() - viewpos;
        glPushMatrix();
        // we would have to translate the conning tower, but the current model
        // is centered arount the player's view already, fixme.
        // glTranslated(conntowerpos.x, conntowerpos.y, conntowerpos.z);
        // glRotatef(-player->get_heading().value(),0,0,1);
        // fixme: rotate by player's orientation, but this looks strange, see
        // above why.
        player->get_orientation().rotmat4().multiply_gl();
        glTranslated(conntowerpos.x, conntowerpos.y, conntowerpos.z);
        conning_tower->display();
        glPopMatrix();
    }

    glDisable(GL_FOG);

    ui.draw_weather_effects();

    postprocessor::instance().process();
}
