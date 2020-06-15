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

#include "global_data.h"	// for myfmod
#include "sub_bg_display.h"

namespace
{
	const double TK_ANGFAC = 360.0/512.0;
	const unsigned TK_PHASES = 6;
	enum element_type {
		et_pointer = 0,
		et_turnknob = 1
	};
}

sub_bg_display::sub_bg_display(user_interface& ui_)
 :	user_display(ui_, "sub_bg"),
	turnknobdrag(TK_NONE),
	turnknobang(TK_NR)
{
}




bool sub_bg_display::handle_mouse_button_event(const mouse_click_data& m)
{
	if (m.down()) {
		// check if mouse is over turn knobs
		turnknobdrag = TK_NONE;
		if (element_for_id(et_turnknob).is_mouse_over(m.position_2d, 128)) {
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
		if (turnknobdrag) {
			//fixme not set angle in display? used twice...
		}
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
	// set angle in elements and draw them
	element_for_id(et_turnknob).set_phase(myfmod(-turnknobang[TK_DIRECTION] * 2.0f, 90.0f) / 90.f);
	element_for_id(et_pointer).set_phase(turnknobang[TK_DIRECTION] * 0.5f / 360.f /* fixme: get angle from player*/);
	draw_elements();
}
