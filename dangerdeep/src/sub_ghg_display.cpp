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

// user display: submarine's ghg (Gruppenhorchgeraet) hearing device
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sub_ghg_display.h"
#include "global_data.h"

namespace
{
	const double TK_ANGFAC = 360.0/512.0;
	enum element_type {
		et_direction_ptr = 0,
		et_direction_knob = 1,
		et_volume_dial = 2,
		et_volume_knob = 3,
	};
}



sub_ghg_display::sub_ghg_display(user_interface& ui_)
 :	user_display(ui_, "sub_ghg")
 ,	turnknobdrag(TK_NONE)
 ,	turnknobang(TK_NR)
{
}



bool sub_ghg_display::handle_mouse_button_event(const mouse_click_data& m)
{
	if (m.down()) {
		// check if mouse is over turn knobs
		turnknobdrag = TK_NONE;
		if (element_for_id(et_direction_knob).is_mouse_over(m.position_2d, 64)) {
			turnknobdrag = TK_DIRECTION;
			return true;
		} else if (element_for_id(et_volume_knob).is_mouse_over(m.position_2d)) {
			turnknobdrag = TK_VOLUME;
			return true;
		}
	} else if (m.up()) {
		turnknobdrag = TK_NONE;
		return true;
	}
	return false;
}



bool sub_ghg_display::handle_mouse_motion_event(const mouse_motion_data& m)
{
	if (m.left()) {
		if (turnknobdrag != TK_NONE) {
			float& ang = turnknobang[unsigned(turnknobdrag)];
			ang += m.position_2d.x * TK_ANGFAC;
			switch (turnknobdrag) {
			case TK_DIRECTION:
				// 0-360*4 degrees match to 0-360
				ang = myclamp(ang, -320.0f, 320.0f);
				//sub->set_ghg_direction(ang*0.5); // fixme: set angle of player
				break;
			case TK_VOLUME:
				// 0-288 degrees match to 5-85 degrees angle
				ang = myclamp(ang, 0.0f, 252.0f);
				break;
			default:	// can never happen
				break;
			}
			return true;
		}
	}
	return false;
}



void sub_ghg_display::display() const
{
	element_for_id(et_volume_dial).set_value(-turnknobang[TK_VOLUME]-18.0f);//fixme scaling!
	element_for_id(et_direction_ptr).set_value(turnknobang[TK_DIRECTION]*0.5 /* fixme: get angle from player*/);
	element_for_id(et_direction_knob).set_value(turnknobang[TK_DIRECTION]);
	draw_elements();

	// get hearing device angle from submarine, if it has one

	// test hack: test signal strengths
// 	angle sonar_ang = angle(turnknobang[TK_DIRECTION]*0.5) + player->get_heading();
// 	vector<double> noise_strengths = gm.sonar_listen_ships(player, sonar_ang);
// 	printf("noise strengths, global ang=%f, L=%f M=%f H=%f U=%f\n",
// 	       sonar_ang.value(), noise_strengths[0], noise_strengths[1], noise_strengths[2], noise_strengths[3]);
}
