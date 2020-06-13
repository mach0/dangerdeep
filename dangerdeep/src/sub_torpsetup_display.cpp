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

// user display: submarine's torpedo setup
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sub_torpsetup_display.h"
#include "cfg.h"
#include "game.h"
#include "global_data.h"
#include "image.h"
#include "keys.h"
#include "log.h"
#include "submarine.h"
#include "submarine_interface.h"
#include "system_interface.h"
#include "torpedo.h"
#include "vector2.h"
#include <memory>

#include <sstream>
#include <utility>

using namespace std;

/*
static int tubelightdx[6] = { 31,150,271,392,518,641};
static int tubelightdy[6] = {617,610,612,612,617,611};
static int tubelightnx[6] = { 34,154,276,396,521,647};
static int tubelightny[6] = {618,618,618,618,618,618};
static int tubeswitchx = 760, tubeswitchy = 492;
*/

static const double TK_ANGFAC = 360.0/512.0;
static const unsigned TK_PHASES = 6;

static const vector2i firstturn_pos(64, 574);
static const vector2i secrange_pos(803, 552);
static const vector2i preheat_pos(730, 377);
static const vector2i torpspeed_pos(834, 251);



sub_torpsetup_display::scheme::scheme(bool day)
{
	const string x = day ? "torpsetup_daylight" : "torpsetup_redlight";
	background = std::make_unique<image>(get_image_dir() + x + "_background.jpg|png");
	rundepthptr.set(x + "_rundepthptr.png", 609, 66, 638, 169);
	secondaryrangeptr.set(x + "_secondaryrangeptr.png", 228, 157, 257, 262);
	primaryrangeptr.set(x + "_primaryrangeptr.png", 241, 90, 260, 263);
	torpspeeddial.set(x + "_torpspeed.png", 541, 77, 636, 172);
	turnangledial.set(x + "_turnangle.png", 469, 508, 619, 658);
	primaryrangedial.set(x + "_primaryrunlength.png", 231, 508, 381, 658);
	torpspeed[0] = std::make_unique<texture>(get_image_dir() + x + "_speedslow.png");
	torpspeed[1] = std::make_unique<texture>(get_image_dir() + x + "_speedmedium.png");
	torpspeed[2] = std::make_unique<texture>(get_image_dir() + x + "_speedhigh.png");
	firstturn[0] = std::make_unique<texture>(get_image_dir() + x + "_turnleft.png");
	firstturn[1] = std::make_unique<texture>(get_image_dir() + x + "_turnright.png");
	secondaryrange[0] = std::make_unique<texture>(get_image_dir() + x + "_secondaryrange_short.png");
	secondaryrange[1] = std::make_unique<texture>(get_image_dir() + x + "_secondaryrange_long.png");
	preheating[0] = std::make_unique<texture>(get_image_dir() + x + "_preheatoff.png");
	preheating[1] = std::make_unique<texture>(get_image_dir() + x + "_preheaton.png");
	temperaturescale = std::make_unique<texture>(get_image_dir() + x + "_tempscale.png");

	// read knobs images and cut to separate images
	sdl_image primaryrangeknobs_day(get_image_dir() + x + "_primaryrangeknobs.png");
	sdl_image turnangleknobs_day(get_image_dir() + x + "_turnangleknobs.png");
	sdl_image rundepthknobs_day(get_image_dir() + x + "_rundepthknobs.png");
	for (unsigned i = 0; i < TK_PHASES; ++i) {
		primaryrangeknob[i].set(new texture(primaryrangeknobs_day, 0, i*192, 192, 192), 277, 571, 373, 667);
		turnangleknob[i].set(new texture(turnangleknobs_day, 0, i*192, 192, 192), 528, 571, 624, 667);
		rundepthknob[i].set(new texture(rundepthknobs_day, 0, i*192, 192, 192), 819, 17, 915, 113);
	}
}



sub_torpsetup_display::sub_torpsetup_display(user_interface& ui_)
	: user_display(ui_), turnknobdrag(TK_NONE), turnknobang(TK_NR)
{
	/*
	selected_tube = 0;
	selected_mode = 0;
	*/
}



bool sub_torpsetup_display::handle_mouse_button_event(const mouse_click_data& m)
{
	auto& gm = ui.get_game();
	auto* sub = dynamic_cast<submarine*>(gm.get_player());
	torpedo::setup& tbsetup = sub->get_torp_in_tube(dynamic_cast<submarine_interface&>(ui).get_selected_tube()).setup;
	if (!myscheme.get()) THROW(error, "sub_torpsetup_display without scheme!");
	const scheme& s = *myscheme;
	if (m.down() && m.left())
	{
		// check if mouse is over turn knobs
		turnknobdrag = TK_NONE;
		if (s.primaryrangeknob[0].is_mouse_over(m.position_2d)) {
			turnknobdrag = TK_PRIMARYRANGE;
		} else if (s.turnangleknob[0].is_mouse_over(m.position_2d)) {
			turnknobdrag = TK_TURNANGLE;
		} else if (s.rundepthknob[0].is_mouse_over(m.position_2d)) {
			turnknobdrag = TK_RUNDEPTH;
		} else if (s.is_over(s.firstturn[0], firstturn_pos, m.position_2d)) {
			tbsetup.initialturn_left = (m.position_2d.x < firstturn_pos.x + int(s.firstturn[0]->get_width()/2));
			log_debug("left?"<<tbsetup.initialturn_left);
		} else if (s.is_over(s.secondaryrange[0], secrange_pos, m.position_2d)) {
			tbsetup.short_secondary_run = (m.position_2d.x < secrange_pos.x + int(s.secondaryrange[0]->get_width()/2));
			log_debug("short run?"<<tbsetup.short_secondary_run);
		} else if (s.is_over(s.preheating[0], preheat_pos, m.position_2d)) {
			tbsetup.preheating = (m.position_2d.y < preheat_pos.y + int(s.preheating[0]->get_height()/2)) ? true : false;
		} else if (s.is_over(s.torpspeed[0], torpspeed_pos, m.position_2d)) {
			int i = (m.position_2d.y - torpspeed_pos.y) * 3 / s.torpspeed[0]->get_height();
			unsigned idx = 2 - unsigned(myclamp(i, int(0), int(2)));
			log_debug("torpspeed="<<idx);
			tbsetup.torpspeed = idx;
		}
		return true;
	} else if (m.up() && m.left()) {
		turnknobdrag = TK_NONE;
	}
	return false;
}



bool sub_torpsetup_display::handle_mouse_motion_event(const mouse_motion_data& m)
{
	auto& gm = ui.get_game();
	auto* sub = dynamic_cast<submarine*>(gm.get_player());
	torpedo::setup& tbsetup = sub->get_torp_in_tube(dynamic_cast<submarine_interface&>(ui).get_selected_tube()).setup;
	if (m.left()) {
		if (turnknobdrag != TK_NONE) {
			float& ang = turnknobang[unsigned(turnknobdrag)];
			ang += m.relative_motion_2d.x * TK_ANGFAC;
			switch (turnknobdrag) {
			case TK_PRIMARYRANGE:
				// 0-360 degrees match to 0-16
				ang = myclamp(ang, 0.0f, 359.0f);
				tbsetup.primaryrange = unsigned(ang*17/360)*100+1600;
				break;
			case TK_TURNANGLE:
				// 0-360 degrees match to 0-180 degrees angle
				// fixme: currently only 0/1 used!
				//ang = myclamp(ang, 0.0f, 360.0f);
				//tbsetup.turnangle = ang*180/360;
				ang = myclamp(ang, 0.0f, 179.0f);
				// fixme: allow only 90/180 for FAT, any angle for LUT, nothing for other types
				tbsetup.turnangle = unsigned(ang*2/180)*90+90;
				break;
			case TK_RUNDEPTH:
				// 0-360 degrees match to 0-25m
				ang = myclamp(ang, 0.0f, 360.0f);
				tbsetup.rundepth = ang*25/360;
				break;
			default:	// can never happen
				break;
			}
		}
	}
	return false;
}



void sub_torpsetup_display::display() const
{
	auto& gm = ui.get_game();
	auto* sub = dynamic_cast<submarine*>(gm.get_player());

	sys().prepare_2d_drawing();

#if 0
	bool is_day = gm.is_day_mode();
	int* tubelightx = (is_day) ? tubelightdx : tubelightnx;
	int* tubelighty = (is_day) ? tubelightdy : tubelightny;
#endif

	if (!myscheme.get()) THROW(error, "sub_torpsetup_display::display without scheme!");
	const scheme& s = *myscheme;

	// testing:
	double ctr = gm.get_time();

	// as first draw lowest layer: sliding temperature scale
	double temperature = myfmod(ctr, 35);//22.5;	// degrees, fixme
	int tempscalex = 549 - int(36 + 435*temperature/35.0f);
	s.temperaturescale->draw(tempscalex, 364);

	// as next draw lower layer: dials
	double torpspeed = myfmod(ctr, 55);//44.0; // knots
	s.torpspeeddial.draw(-(torpspeed * 330/55.0)); // 55kts = 0deg+x*330deg

	// get tube settings
	const torpedo::setup& tbsetup = sub->get_torp_in_tube(dynamic_cast<submarine_interface&>(ui).get_selected_tube()).setup;

	unsigned primaryrangedial = tbsetup.primaryrange - 1600;
	s.primaryrangedial.draw(primaryrangedial / -5.0f);	// 1 degree = 5meters

	float firstturnangle = tbsetup.turnangle.value();
	s.turnangledial.draw(firstturnangle * -1.8f); // 18 degrees = 10 turn degrees

	// draw background
	s.background->draw(0, 0);

	// draw objects from upper layer: knobs/switches/pointers
	unsigned torpspeedidx = tbsetup.torpspeed;
	s.torpspeed[torpspeedidx]->draw(torpspeed_pos.x, torpspeed_pos.y);

	unsigned ftidx = tbsetup.initialturn_left ? 0 : 1;
	s.firstturn[ftidx]->draw(firstturn_pos.x, firstturn_pos.y);

	unsigned sridx = tbsetup.short_secondary_run ? 0 : 1;
	s.secondaryrange[sridx]->draw(secrange_pos.x, secrange_pos.y);

	unsigned preheatingidx = tbsetup.preheating ? 1 : 0;
	s.preheating[preheatingidx]->draw(preheat_pos.x, preheat_pos.y);

	s.primaryrangeknob[unsigned(myfmod(turnknobang[TK_PRIMARYRANGE], 360.0f)/(45.0/TK_PHASES)+0.5)%TK_PHASES].draw(0);

	s.turnangleknob[unsigned(myfmod(turnknobang[TK_TURNANGLE], 360.0f)/(45.0/TK_PHASES)+0.5)%TK_PHASES].draw(0);

	s.rundepthknob[unsigned(myfmod(turnknobang[TK_RUNDEPTH], 360.0f)/(45.0/TK_PHASES)+0.5)%TK_PHASES].draw(0);

	double rundepth = tbsetup.rundepth;	// meters
	s.rundepthptr.draw(rundepth * 300/25.0 + 30); // 25m = 30deg+x*300deg

	double secondaryrange = myfmod(ctr,32)*50;//800.0; // meters
	s.secondaryrangeptr.draw(secondaryrange * 320/1600.0 + 20.0); // 1600m = 20deg+x*320deg

	double primaryrange = myfmod(ctr,32)*500;//2300.0; // meters
	s.primaryrangeptr.draw(primaryrange * 320/16000.0 + 20.0); // 16000m = 20deg+x*320deg

	ui.draw_infopanel(true);
	sys().unprepare_2d_drawing();
}



void sub_torpsetup_display::enter(bool is_day)
{
	myscheme = std::make_unique<scheme>(is_day);
}



void sub_torpsetup_display::leave()
{
	myscheme.reset();
}
