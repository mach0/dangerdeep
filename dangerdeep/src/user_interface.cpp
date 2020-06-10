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

// user interface common code
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "oglext/OglExt.h"
#include <glu.h>

#include <iomanip>
#include <iostream>
#include <memory>

#include <sstream>
#include <utility>

#include "user_interface.h"
#include "system_interface.h"
#include "game.h"
#include "texts.h"
#include "model.h"
#include "airplane.h"
#include "depth_charge.h"
#include "gun_shell.h"
#include "vector3.h"
#include "widget.h"
#include "submarine_interface.h"
#include "submarine.h"	// needed for underwater sound reduction
//#include "ship_interface.h"
//#include "airplane_interface.h"
#include "sky.h"
#include "particle.h"
#include "water.h"
#include "matrix4.h"
#include "cfg.h"
#include "keys.h"
#include "global_data.h"
#include "music.h"
#include "log.h"
#include "primitives.h"
using namespace std;

const double message_vanish_time = 10;
const double message_fadeout_time = 2;

#undef  RAIN
#undef  SNOW

#define MAX_PANEL_SIZE 256


/*
	a note on our coordinate system (11/10/2003):
	We simulate earth by projecting objects according to curvature from earth
	space to Euclidian space. This projection is yet a identity projection, that means
	we ignore curvature yet.
	The map forms a cylinder around the earth, that means x,y position on the map translates
	to longitude,latitude values. Hence valid coordinates go from -20000km...20000km in x
	direction and -10000km to 10000km in y direction. (we could use exact values, around
	20015km). The wrap around is a problem, but that's somewhere in the Pacific ocean, so
	we just ignore it. This mapping leads to some distorsion and wrong distance values
	when coming to far north or south on the globe. We just ignore this for simplicity's
	sake. The effect shouldn't be noticeable.
*/

user_interface::user_interface(game& gm) :
	mygame(&gm),
	pause(false),
	time_scale(1),
	panel_visible(true),
	screen_selector_visible(false),
	playlist_visible(false),
	main_menu_visible(false),
	bearing(0),
	elevation(90),
	bearing_is_relative(true),
	current_display(0),
	current_popup(0),
	mycoastmap(get_map_dir() + "default.xml"),
	daymode(gm.is_day_mode())
{
	add_loading_screen("coast map initialized");
	mysky = std::make_unique<sky>();
	panel = std::make_unique<widget>(0, 768-32, 1024, 32, "");
	panel->set_background(nullptr);
	// ca. 1024-2*8 for 6 texts => 168 pix. for each text
	int paneltextnrs[6] = { 1, 4, 5, 2, 98, 61 };
	const char* paneltexts[6] = { "000", "000", "000", "000", "000", "00:00:00" };
	for (int i = 0; i < 6; ++i) {
		int off = 8 + i*(1024-2*8)/6;
		string tx = texts::get(paneltextnrs[i]);
		vector2i sz = widget::get_theme()->myfont->get_size(tx);
		panel->add_child(std::make_unique<widget_text>(off, 4, 0, 0, tx));
		panel_valuetexts[i] = &panel->add_child(std::make_unique<widget_text>(off + 8 + sz.x, 4, 0, 0, paneltexts[i]));
	}

	// create screen selector widget
	screen_selector = std::make_unique<widget>(0, 0, 256, 32, "");
	screen_selector->set_background(nullptr);

	// create playlist widget
	const auto music_playlist_width = 512;
	music_playlist = std::make_unique<widget>(0, 0, music_playlist_width, 512, texts::get(262));
	music_playlist->set_background(nullptr);
	struct musiclist : public widget_list
	{
		bool active;
		void on_sel_change() override {
			if (!active) return;
			int s = get_selected();
			if (s >= 0)
				music::instance().play_track(unsigned(s), 500);
		}
		musiclist(int x, int y, int w, int h) : widget_list(x, y, w, h), active(false) {}
	};
	auto& playlist = music_playlist->add_child_near_last_child(std::make_unique<musiclist>(0, 0, music_playlist_width, 512));
	music& m = music::instance();
	std::vector<std::string> mpl = m.get_playlist();
	for (const auto & it : mpl) {
		playlist.append_entry(it);
	}
	playlist_repeat_checkbox = &music_playlist->add_child_near_last_child(std::make_unique<widget_caller_checkbox<user_interface&>>(0, 0, music_playlist_width/2, 32, texts::get(263), nullptr, false, [](auto& ui) { ui.playlist_mode_changed(); }, *this));
	playlist_shuffle_checkbox = &music_playlist->add_child_near_last_child(std::make_unique<widget_caller_checkbox<user_interface&>>(0, 0, music_playlist_width/2, 32, texts::get(264), nullptr, false, [](auto& ui) { ui.playlist_mode_changed(); }, *this), 0, 1);
	playlist_mute_checkbox = &music_playlist->add_child_near_last_child(std::make_unique<widget_caller_checkbox<user_interface&>>(0, 0, music_playlist_width/2, 32, texts::get(265), nullptr, false, [](user_interface& ui) { ui.playlist_mute(); }, *this), 0);
	playlist_mute_checkbox->move_pos(vector2i(-music_playlist_width, 0));
	music_playlist->add_child_near_last_child(std::make_unique<widget_caller_button<bool&>>(0, 0, music_playlist_width, 32, texts::get(260), nullptr, [](auto& b) { b = false; }, playlist_visible));
	music_playlist->clip_to_children_area();
	music_playlist->set_pos(vector2i(0, 0));
	// enable music switching finally, to avoid on_sel_change changing the music track,
	// because on_sel_change is called above, when adding entries.
	playlist.active = true;

	// create main menu widget
	main_menu = std::make_unique<widget>(0, 0, 256, 128, texts::get(104));
	main_menu->set_background(nullptr);
	main_menu->add_child_near_last_child(std::make_unique<widget_caller_button<user_interface&>>(0, 0, 256, 32, texts::get(266), nullptr, [](auto& ui) { ui.show_screen_selector(); }, *this));
	main_menu->add_child_near_last_child(std::make_unique<widget_caller_button<user_interface&>>(0, 0, 256, 32, texts::get(267), nullptr, [](auto& ui) { ui.toggle_popup(); }, *this));
	main_menu->add_child_near_last_child(std::make_unique<widget_caller_button<user_interface&>>(0, 0, 256, 32, texts::get(261), nullptr, [](auto& ui) { ui.show_playlist(); }, *this));
	main_menu->add_child_near_last_child(std::make_unique<widget_caller_button<user_interface&>>(0, 0, 256, 32, texts::get(268), nullptr, [](auto& ui) { ui.toggle_pause(); }, *this));
	main_menu->add_child_near_last_child(std::make_unique<widget_caller_button<user_interface&>>(0, 0, 256, 32, texts::get(177), nullptr, [](auto& ui) { ui.request_abort(true); }, *this));
	main_menu->add_child_near_last_child(std::make_unique<widget_caller_button<bool&>>(0, 0, 256, 32, texts::get(260), nullptr, [](auto& b) { b = false; }, main_menu_visible));
	main_menu->clip_to_children_area();
	vector2i mmp = sys().get_res_2d() - main_menu->get_size();
	main_menu->set_pos(vector2i(mmp.x/2, mmp.y/2));

	// create weather effects textures

	// rain
#ifdef RAIN
#define NR_OF_RAIN_FRAMES 16
#define NR_OF_RAIN_DROPS 800
#define RAIN_TEX_W 256
#define RAIN_TEX_H 256
	raintex.resize(NR_OF_RAIN_FRAMES);
	vector<uint8_t> raintmptex(RAIN_TEX_W * RAIN_TEX_H * 2);

	for (unsigned j = 0; j < NR_OF_RAIN_FRAMES; ++j) {
		for (unsigned k = 0; k < RAIN_TEX_W * RAIN_TEX_H * 2; k += 2) {
			raintmptex[k + 0] = 128;
			raintmptex[k + 1] = 0;
		}
		for (unsigned i = 0; i < NR_OF_RAIN_DROPS; ++i) {
			vector2i pos(rnd(RAIN_TEX_W-2)+2, rnd(RAIN_TEX_H-2));
			uint8_t c = rnd(64)+128;
			raintmptex[(RAIN_TEX_W*pos.y + pos.x) * 2 + 0] = c;
			raintmptex[(RAIN_TEX_W*pos.y + pos.x) * 2 + 1] = 128;
			pos.x -= 1; pos.y += 1;
			raintmptex[(RAIN_TEX_W*pos.y + pos.x) * 2 + 0] = c;
			raintmptex[(RAIN_TEX_W*pos.y + pos.x) * 2 + 1] = 192;
			pos.x -= 1; pos.y += 1;
			raintmptex[(RAIN_TEX_W*pos.y + pos.x) * 2 + 0] = c;
			raintmptex[(RAIN_TEX_W*pos.y + pos.x) * 2 + 1] = 255;
		}
		raintex.reset(j, new texture(raintmptex, RAIN_TEX_W, RAIN_TEX_H, GL_LUMINANCE_ALPHA, texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT));
	}
#endif
	// snow
#ifdef SNOW
#define NR_OF_SNOW_FRAMES 23
#define NR_OF_SNOW_FLAKES 2000
#define SNOW_TEX_W 256
#define SNOW_TEX_H 256
	snowtex.resize(NR_OF_SNOW_FRAMES);
	vector<uint8_t> snowtmptex(SNOW_TEX_W * SNOW_TEX_H * 2, 255);
	vector<vector2i> snowflakepos(NR_OF_SNOW_FLAKES);
	vector<int> snowxrand(NR_OF_SNOW_FRAMES);

	// create random x coordinate sequence (perturbation)
	vector<unsigned> snowxtrans(SNOW_TEX_W);
	for (unsigned k = 0; k < SNOW_TEX_W; ++k) {
		snowxtrans[k] = k;
	}
	for (unsigned k = 0; k < SNOW_TEX_W * 20; ++k) {
		unsigned a = rnd(SNOW_TEX_W), b = rnd(SNOW_TEX_W);
		unsigned c = snowxtrans[a];
		snowxtrans[a] = snowxtrans[b];
		snowxtrans[b] = c;
	}

	for (unsigned j = 0; j < NR_OF_SNOW_FRAMES; ++j) {
		snowxrand[j] = rnd(3)-1;
	}
	snowflakepos[0] = vector2i(snowxtrans[0], 0);
	for (unsigned i = 1; i < NR_OF_SNOW_FLAKES; ++i) {
		vector2i oldpos = snowflakepos[i-1];
		for (unsigned j = 0; j < NR_OF_SNOW_FRAMES; ++j) {
			oldpos.x += snowxrand[(j+3*i)%NR_OF_SNOW_FRAMES];
			if (oldpos.x < 0) oldpos.x += SNOW_TEX_W;
			if (oldpos.x >= SNOW_TEX_W) oldpos.x -= SNOW_TEX_W;
			oldpos.y += 1;	// fixme add more complex "fall down" function
			if (oldpos.y >= SNOW_TEX_H) {
				oldpos.x = snowxtrans[oldpos.x];
				oldpos.y = 0;
			}
		}
		snowflakepos[i] = oldpos;
	}
	for (unsigned i = 0; i < NR_OF_SNOW_FRAMES; ++i) {
		for (unsigned k = 0; k < SNOW_TEX_W * SNOW_TEX_H * 2; k += 2)
			snowtmptex[k + 1] = 0;
		for (unsigned j = 0; j < NR_OF_SNOW_FLAKES; ++j) {
			snowtmptex[(SNOW_TEX_H*snowflakepos[j].y + snowflakepos[j].x) * 2 + 1] = 255;
			vector2i& oldpos = snowflakepos[j];
			oldpos.x += snowxrand[(j+3*i)%NR_OF_SNOW_FRAMES];
			if (oldpos.x < 0) oldpos.x += SNOW_TEX_W;
			if (oldpos.x >= SNOW_TEX_W) oldpos.x -= SNOW_TEX_W;
			oldpos.y += 1;	// fixme add more complex "fall down" function
			if (oldpos.y >= SNOW_TEX_H) {
				oldpos.x = snowxtrans[oldpos.x];
				oldpos.y = 0;
			}
		}
/*
		ostringstream oss;
		oss << "snowframe"<<i<<".pgm";
		ofstream osg(oss.str().c_str());
		osg << "P5\n"<<SNOW_TEX_W<<" "<<SNOW_TEX_H<<"\n255\n";
		osg.write((const char*)(&snowtmptex[0]), SNOW_TEX_W * SNOW_TEX_H);
*/
		snowtex.reset(i, new texture(snowtmptex, SNOW_TEX_W, SNOW_TEX_H, GL_LUMINANCE_ALPHA, texture::LINEAR_MIPMAP_LINEAR, texture::REPEAT));
	}
#endif

	particle::init();

	// level size is N * sample_spacing * 2^j, here we give n = log2(N)
	// where j is level number from 0 on.
	// so we compute number of levels:
	// 2^n * sample_spacing * 2^j_max <= z_far
	// j_max <= log2(z_far / (2^n * sample_spacing))
	// and #levels = j_max+1
	// so #levels = floor(log2(z_far / (2^n * sample_spacing))) + 1
	//const double z_far = 20000.0;

  add_loading_screen("user interface initialized");

	mygeoclipmap = std::make_unique<geoclipmap>(TERRAIN_NR_LEVELS, TERRAIN_RESOLUTION_N, mygame->get_height_gen());
  mygeoclipmap->set_viewerpos(gm.get_player()->get_pos());

	add_loading_screen("terrain loaded");
}



void user_interface::finish_construction()
{
	mycoastmap.finish_construction();
}



std::shared_ptr<user_interface> user_interface::create(game& gm)
{
	sea_object* p = gm.get_player();
	std::shared_ptr<user_interface> ui;
	// check for interfaces
	if (dynamic_cast<submarine*>(p)) ui = std::make_shared<submarine_interface>(gm);
#if 0
	else if (dynamic_cast<ship*>(p)) ui = std::make_shared<ship_interface>(gm);
	else if (dynamic_cast<airplane*>(p)) ui = std::make_shared<airplane_interface>(gm);
#endif
	if (ui) ui->finish_construction();
	return ui;
}



user_interface::~user_interface ()
{
	particle::deinit();
}



const water& user_interface::get_water() const
{
	return mygame->get_water();
}



angle user_interface::get_relative_bearing() const
{
	if (bearing_is_relative)
		return bearing;
	return bearing - mygame->get_player()->get_heading();
}



angle user_interface::get_absolute_bearing() const
{
	if (bearing_is_relative)
		return mygame->get_player()->get_heading() + bearing;
	return bearing;
}



angle user_interface::get_elevation() const
{
	return elevation;
}



void user_interface::add_bearing(angle a)
{
	bearing += a;
}



void user_interface::add_elevation(angle a)
{
	elevation += a;
}



void user_interface::display() const
{
	// fixme: brightness needs sun_pos, so compute_sun_pos() is called multiple times per frame
	// but is very costly. we could cache it.
	mygame->get_water().set_refraction_color(mygame->compute_light_color(mygame->get_player()->get_pos()));
	displays[current_display].display();

	// popups
	if (current_popup > 0)
		popups[current_popup-1].display();

	// draw screen selector if visible
	if (screen_selector_visible) {
		sys().prepare_2d_drawing();
		screen_selector->draw();
		sys().unprepare_2d_drawing();
	}

	// draw music playlist if visible
	if (playlist_visible) {
		sys().prepare_2d_drawing();
		music_playlist->draw();
		sys().unprepare_2d_drawing();
	}

	// draw main_menu if visible
	if (main_menu_visible) {
		sys().prepare_2d_drawing();
		main_menu->draw();
		sys().unprepare_2d_drawing();
	}
}



void user_interface::set_time(double tm)
{
	// if we switched from day to night mode or vice versa, reload current screen.
	if (mygame) {
		bool newdaymode = mygame->is_day_mode();
		if (newdaymode != daymode) {
			mygame->freeze_time();
			displays[current_display].leave();
			displays[current_display].enter(newdaymode);
			mygame->unfreeze_time();
		}
		daymode = newdaymode;
	}

	mysky->set_time(tm);
	mycaustics.set_time(tm);
	mygame->get_water().set_time(tm);
}



bool user_interface::handle_key_event(const key_data& k)
{
	if (k.down()) {
		if (is_configured_key(key_command::TOGGLE_RELATIVE_BEARING, k)) {
			bearing_is_relative = !bearing_is_relative;
			add_message(texts::get(bearing_is_relative ? 220 : 221));
			return true;
		} else if (is_configured_key(key_command::TOGGLE_POPUP, k)) {
			toggle_popup();
			return true;
		}
	}
	if (current_popup > 0) {
		if (popups[current_popup-1].handle_key_event(k)) {
			return true;
		}
	}
	return displays[current_display].handle_key_event(k);
}



bool user_interface::handle_mouse_button_event(const mouse_click_data& m)
{
	if (panel_visible) {
		if (panel->is_mouse_over(m.position_2d) && widget::handle_mouse_button_event(*panel, m)) {
			return true;
		}
	}

	if (main_menu_visible) {
		if (main_menu->is_mouse_over(m.position_2d) && widget::handle_mouse_button_event(*main_menu, m)) {
			return true;
		}
	}

	if (screen_selector_visible) {
		if (screen_selector->is_mouse_over(m.position_2d) && widget::handle_mouse_button_event(*screen_selector, m)) {
			return true;
		}
	}

	if (playlist_visible) {
		if (music_playlist->is_mouse_over(m.position_2d) && widget::handle_mouse_button_event(*music_playlist, m)) {
			return true;
		}
	}
	if (current_popup > 0) {
		if (popups[current_popup-1].handle_mouse_button_event(m)) {
			return true;
		}
	}
	return displays[current_display].handle_mouse_button_event(m);
}



bool user_interface::handle_mouse_motion_event(const mouse_motion_data& m)
{
	if (panel_visible) {
		if (panel->is_mouse_over(m.position_2d) && widget::handle_mouse_motion_event(*panel, m))
			return true;
	}

	if (main_menu_visible) {
		if (main_menu->is_mouse_over(m.position_2d) && widget::handle_mouse_motion_event(*main_menu, m)) {
			return true;
		}
	}

	if (screen_selector_visible) {
		if (screen_selector->is_mouse_over(m.position_2d) && widget::handle_mouse_motion_event(*screen_selector, m)) {
			// drag for the menu
			// fixme: drag&drop support should be in widget class...
			vector2i p = screen_selector->get_pos();
			vector2i s = screen_selector->get_size();
			// drag menu with left mouse button when on title or right mouse button else
			if (m.is_pressed(mouse_button::middle) || (m.is_pressed(mouse_button::right)
				&& m.position_2d.x >= p.x
				&& m.position_2d.y >= p.y
				&& m.position_2d.x < p.x + s.x
				&& m.position_2d.y < p.y + 32))
			{
				p += m.rel_motion_2d;
				p = p.max(vector2i(0, 0));
				p = p.min(sys().get_res_2d() - s);
				screen_selector->set_pos(p);
				return true;
			}
		}
	}

	if (playlist_visible) {
		if (music_playlist->is_mouse_over(m.position_2d) && widget::handle_mouse_motion_event(*music_playlist, m)) {
			// drag for the menu
			// fixme: drag&drop support should be in widget class...
			vector2i p = music_playlist->get_pos();
			vector2i s = music_playlist->get_size();
			// drag menu with left mouse button when on title or right mouse button else
			if (m.is_pressed(mouse_button::middle) || (m.is_pressed(mouse_button::right)
				&& m.position_2d.x >= p.x
				&& m.position_2d.y >= p.y
				&& m.position_2d.x < p.x + s.x
				&& m.position_2d.y < p.y + 32 + 8))
			{
				p += m.rel_motion_2d;
				if (p.x < 0) p.x = 0;
				if (p.y < 0) p.y = 0;
				// 2006-11-30 doc1972 negative pos and size of a playlist makes no sence, so we cast
				if ((unsigned int)(p.x + s.x) > sys().get_res_x_2d()) p.x = sys().get_res_x_2d() - s.x;
				if ((unsigned int)(p.y + s.y) > sys().get_res_y_2d()) p.y = sys().get_res_y_2d() - s.y;
				music_playlist->set_pos(p);
			}
		}
	}
	if (current_popup > 0) {
		if (popups[current_popup-1].handle_mouse_motion_event(m)) {
			return true;
		}
	}
	return displays[current_display].handle_mouse_motion_event(m);
}



bool user_interface::handle_mouse_wheel_event(const mouse_wheel_data& m)
{
	if (panel_visible && panel->is_mouse_over(m.position_2d)) {
		if (widget::handle_mouse_wheel_event(*panel, m)) {
			return true;
		}
	}

	if (main_menu_visible && main_menu->is_mouse_over(m.position_2d)) {
		if (widget::handle_mouse_wheel_event(*main_menu, m)) {
			return true;
		}
	}

	if (screen_selector_visible && screen_selector->is_mouse_over(m.position_2d)) {
		if (widget::handle_mouse_wheel_event(*screen_selector, m)) {
			return true;
		}
	}

	if (playlist_visible && music_playlist->is_mouse_over(m.position_2d)) {
		if (widget::handle_mouse_wheel_event(*music_playlist, m)) {
			return true;
		}
	}
	if (current_popup > 0) {
		if (popups[current_popup-1].handle_mouse_wheel_event(m)) {
			return true;
		}
	}
	return displays[current_display].handle_mouse_wheel_event(m);
}



void user_interface::show_target(double vx, double vy, double w, double h, const vector3& viewpos)
{
	if (mygame && mygame->get_player()->get_target()) {
		// draw red triangle below target
		// find screen position of target by projecting its position to screen
		// coordinates.
		vector4 tgtscr = (matrix4::get_glf(GL_PROJECTION_MATRIX)
				  * matrix4::get_glf(GL_MODELVIEW_MATRIX))
			* (mygame->get_player()->get_target()->get_pos() - viewpos).xyz0();
		if (tgtscr.z > 0) {
			// only when in front.
			// transform to screen coordinates, using the projection coordinates
			double x = (0.5 * tgtscr.x / tgtscr.w + 0.5) * w + vx;
			double y = sys().get_res_y_2d()
				- ((0.5 * tgtscr.y / tgtscr.w + 0.5) * h + vy);
			sys().prepare_2d_drawing();
			primitives::triangle(vector2f(x-10, y+20),
					     vector2f(x   , y+10),
					     vector2f(x+10, y+20),
					     colorf(1,0,0,0.5)).render();
			sys().unprepare_2d_drawing();
		}
	}
}

void user_interface::draw_terrain(const vector3& viewpos, angle dir,
				  double max_view_dist, bool mirrored, int above_water) const
{
#if 0
	glPushMatrix();
	glTranslated(0, 0, -viewpos.z);
	// still needed to render the props.
	mycoastmap.render(viewpos.xy(), max_view_dist, mirrored);
	glPopMatrix();
#endif

	// frustum is mirrored inside geoclipmap
	frustum viewfrustum = frustum::from_opengl();
	glPushMatrix();
	if (mirrored)
		glScalef(1.0f, 1.0f, -1.0f);
	viewfrustum.translate(viewpos);
	mygeoclipmap->set_viewerpos(viewpos);
	mygeoclipmap->display(viewfrustum, -viewpos, mirrored, above_water);
	glPopMatrix();
}



void user_interface::draw_weather_effects() const
{
#if defined(RAIN) || defined(SNOW)
	// draw layers of snow flakes or rain drops (test)
	// get projection from frustum to view
	matrix4 c2w = (matrix4::get_gl(GL_PROJECTION_MATRIX) * matrix4::get_gl(GL_MODELVIEW_MATRIX)).inverse();
	// draw planes between z-near and z-far with ascending distance and 2d texture with flakes/strains
	texture* tex = 0;
#ifdef RAIN
	unsigned sf = unsigned(mygame->get_time() * NR_OF_RAIN_FRAMES) % NR_OF_RAIN_FRAMES;
	tex = raintex[sf];
#endif
#ifdef SNOW
	unsigned sf = unsigned(mygame->get_time() * NR_OF_SNOW_FRAMES) % NR_OF_SNOW_FRAMES;
	tex = snowtex[sf];
#endif
	//pd.near_z,pd.far_z
	double zd[3] = { 0.3, 0.9, 0.7 };
	//fixme: planes should be orthogonal to z=0 plane (xy billboarding)
	for (unsigned i = 0; i < 1; ++i) {
		vector3 p0 = c2w * vector3(-1,  1, zd[i]);
		vector3 p1 = c2w * vector3(-1, -1, zd[i]);
		vector3 p2 = c2w * vector3( 1, -1, zd[i]);
		vector3 p3 = c2w * vector3( 1,  1, zd[i]);
		primitives::textured_quad(vector3f(p1),
					  vector3f(p2),
					  vector3f(p3),
					  vector3f(p0),
					  *tex,
					  vector2f(0, 3),
					  vector2f(3, 0));
		//fixme: uv size changes with depth
	}
#endif
}



void user_interface::toggle_pause()
{
	pause = !pause;
	if (pause) {
		add_message(texts::get(52));
		pause_all_sound();
	} else {
		add_message(texts::get(53));
		resume_all_sound();
	}
}



bool user_interface::time_scale_up()
{
	if (time_scale < 4096) {
		time_scale *= 2;
		return true;
	}
	return false;
}

bool user_interface::time_scale_down()
{
	if (time_scale > 1) {
		time_scale /= 2;
		return true;
	}
	return false;
}

void user_interface::draw_infopanel(bool onlytexts) const
{
	if (!onlytexts && panel_visible) {
		ostringstream os0;
		os0 << setw(3) << left << mygame->get_player()->get_heading().ui_value();
		panel_valuetexts[0]->set_text(os0.str());
		ostringstream os1;
		os1 << setw(3) << left << unsigned(fabs(round(sea_object::ms2kts(mygame->get_player()->get_speed()))));
		panel_valuetexts[1]->set_text(os1.str());
		ostringstream os2;
		os2 << setw(3) << left << unsigned(round(std::max(0.0, -mygame->get_player()->get_pos().z)));
		panel_valuetexts[2]->set_text(os2.str());
		ostringstream os3;
		os3 << setw(3) << left << get_absolute_bearing().ui_value();
		panel_valuetexts[3]->set_text(os3.str());
		ostringstream os4;
		os4 << setw(3) << left << time_scale;
		panel_valuetexts[4]->set_text(os4.str());
		// compute time string
		panel_valuetexts[5]->set_text(get_time_string(mygame->get_time()));

		panel->draw();
	}

	// draw messages: fixme later move to separate function ?
	double vanish_time = mygame->get_time() - message_vanish_time;
	int y = (onlytexts ? sys().get_res_y_2d() : panel->get_pos().y)
		- font_vtremington12->get_height();
	for (auto it = messages.rbegin();
	     it != messages.rend(); ++it) {
		if (it->first < vanish_time)
			break;
		double alpha = std::min(1.0, (it->first - vanish_time)/message_fadeout_time);
		font_vtremington12->print(0, y, it->second, color(255,255,255,uint8_t(255*alpha)), true);
		y -= font_vtremington12->get_height();
	}
}



void user_interface::add_message(const string& s)
{
	// add message
	messages.emplace_back(mygame->get_time(), s);

	// remove old messages
	while (messages.size() > 6)
		messages.pop_front();
	double vanish_time = mygame->get_time() - message_vanish_time;
	for (auto it = messages.begin(); it != messages.end(); ) {
		if (it->first < vanish_time) {
			it = messages.erase(it);
		} else {
			++it;
		}
	}
}



void user_interface::play_sound_effect(const string &se,
				       const vector3& noise_source /*, bool loop*/) const
{
	music::instance().play_sfx(se, mygame->get_player()->get_pos(),
			       mygame->get_player()->get_heading(),
			       noise_source);
}



void user_interface::set_allowed_popup()
{
	// 0 is always valid (no popup)
	if (current_popup == 0) return;

	unsigned mask = displays[current_display].get_popup_allow_mask();
	mask >>= (current_popup-1);
	while (mask != 0) {
		// is popup number valid?
		if (mask & 1)
			return;
		++current_popup;
		mask >>= 1;
	}
	current_popup = 0;
}



void user_interface::set_current_display(unsigned curdis)
{
	if (current_display == curdis) {
		// if we are already on the screen, toggle between popups instead.
		toggle_popup();
		return;
	}
	if (mygame)
		mygame->freeze_time();
	displays[current_display].leave();
	current_display = curdis;

	// clear both screen buffers
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	sys().finish_frame();
	glClear(GL_COLOR_BUFFER_BIT);
	sys().finish_frame();

	displays[current_display].enter(daymode);
	if (mygame)
		mygame->unfreeze_time();

	// check if current popup is still allowed. if not, clear popup
	if (current_popup > 0) {
		unsigned mask = displays[current_display].get_popup_allow_mask();
		mask >>= (current_popup-1);
		if ((mask & 1) == 0)
			current_popup = 0;
	}
}

void user_interface::playlist_mode_changed()
{
	if (playlist_repeat_checkbox->is_checked()) {
		music::instance().set_playback_mode(music::playback_mode::loop_track);
	} else if (playlist_shuffle_checkbox->is_checked()) {
		music::instance().set_playback_mode(music::playback_mode::shuffle_track);
	} else {
		music::instance().set_playback_mode(music::playback_mode::loop_list);
	}
}

void user_interface::playlist_mute()
{
	if (playlist_mute_checkbox->is_checked())
		music::instance().stop();
	else
		music::instance().play();
}

void user_interface::show_screen_selector()
{
	screen_selector_visible = true;
	playlist_visible = false;
	main_menu_visible = false;
}

void user_interface::toggle_popup()
{
	// determine which pop is shown and which is allowed and switch to it
	++current_popup;
	set_allowed_popup();
}

void user_interface::show_playlist()
{
	screen_selector_visible = false;
	playlist_visible = true;
	main_menu_visible = false;
}

void user_interface::pause_all_sound() const
{
	music::instance().pause_sfx(true);
}

void user_interface::resume_all_sound() const
{
	music::instance().pause_sfx(false);
}
