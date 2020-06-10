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

// user display: submarine's bg hearing device
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sub_bg_display.h"
#include "cfg.h"
#include "game.h"
#include "global_data.h"
#include "image.h"
#include "keys.h"
#include "submarine.h"
#include "submarine_interface.h"
#include "system_interface.h"
#include "texture.h"
#include <memory>

#include <sstream>
#include <utility>

using namespace std;

static const double TK_ANGFAC = 360.0/512.0;
static const unsigned TK_PHASES = 6;

sub_bg_display::scheme::scheme(bool day)
{
	const string x = day ? "BG_daylight" : "BG_redlight";
	background = std::make_unique<image>(get_image_dir() + x + "_background.jpg");
	direction_ptr.set(x + "_pointer.png", 341, 153, 373, 346);
	for (unsigned i = 0; i < TK_PHASES; ++i) {
		ostringstream osn;
		osn << (i+1) ;
		turn_wheel[i].set(x + "_knob_pos" + osn.str() + ".png", 110, 641);
	}
}

sub_bg_display::sub_bg_display(user_interface& ui_)
	: user_display(ui_), turnknobdrag(TK_NONE), turnknobang(TK_NR)
{
}



bool sub_bg_display::handle_mouse_button_event(const mouse_click_data& m)
{
	if (m.down()) {
		// check if mouse is over turn knobs
		if (!myscheme.get()) THROW(error, "sub_bg_display without scheme!");
		const scheme& s = *myscheme;
		turnknobdrag = TK_NONE;
		if (s.turn_wheel[0].is_mouse_over(m.position_2d, 128)) {
			turnknobdrag = TK_DIRECTION;
			return true;
		}
	} else if (m.up()) {
		turnknobdrag = TK_NONE;
		return true;
	}
	return false;
}



bool sub_bg_display::handle_mouse_motion_event(const mouse_motion_data& m)
{
	if (m.left()) {
		if (turnknobdrag != TK_NONE) {
			float& ang = turnknobang[unsigned(turnknobdrag)];
			ang += m.position_2d.x * TK_ANGFAC;
			switch (turnknobdrag) {
			case TK_DIRECTION:
				// bring to 0...360 degree value
				ang = myfmod(ang, 720.0f);
				//sub->set_bg_direction(ang); // fixme: set angle of player
				break;
			default:	// can never happen
				break;
			}
			return true;
		}
	}
	return false;
}



void sub_bg_display::display() const
{
	sys().prepare_2d_drawing();

	// get hearing device angle from submarine, if it has one

	if (!myscheme.get()) THROW(error, "sub_bg_display::display without scheme!");
	const scheme& s = *myscheme;

	s.background->draw(0, 0);
	s.turn_wheel[unsigned(myfmod(-turnknobang[TK_DIRECTION] * 2.0f, 90.0f)) * TK_PHASES / 90].draw();
	s.direction_ptr.draw(turnknobang[TK_DIRECTION] * 0.5f /* fixme: get angle from player*/);

	ui.draw_infopanel();

	sys().unprepare_2d_drawing();
}



void sub_bg_display::enter(bool is_day)
{
	myscheme = std::make_unique<scheme>(is_day);
}



void sub_bg_display::leave()
{
	myscheme.reset();
}
