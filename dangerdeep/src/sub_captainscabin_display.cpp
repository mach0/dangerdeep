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

// user display: submarine's torpedo room
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sub_captainscabin_display.h"
#include "font.h"
#include "game.h"
#include "global_data.h"
#include "submarine_interface.h"
#include "system_interface.h"
#include "texts.h"

namespace
{
	enum element_type {
		et_begin = 0,
		et_successes = 0,
		et_logbook = 1,
		et_torpedoes = 2,
		et_recogmanual = 3,
		et_end = 4
	};

	void do_action(submarine_interface& s, element_type t)
	{
		switch (t) {
		case et_successes:	s.goto_successes(); break;
		case et_logbook:	s.goto_logbook(); break;
		case et_torpedoes:	s.goto_torpedomanagement(); break;
		case et_recogmanual:	s.goto_recogmanual(); break;
		default:	THROW(error, "invalid element type");
		}
	}

	int get_description(element_type t)
	{
		switch (t) {
		case et_successes:	return 272;
		case et_logbook:	return 255;
		case et_torpedoes:	return 253;
		case et_recogmanual:	return 273;
		default:	THROW(error, "invalid element type");
		}
	}

	color get_color(element_type t)
	{
		switch (t) {
		case et_successes:	return color(255, 224, 224);
		case et_logbook:	return color(224, 224, 255);
		case et_torpedoes:	return color(224, 255, 224);
		case et_recogmanual:	return color(255, 224, 224);
		default:	THROW(error, "invalid element type");
		}
	}
}



sub_captainscabin_display::sub_captainscabin_display(user_interface& ui_) :
	user_display(ui_, "sub_captainscabin")
{
}



void sub_captainscabin_display::display() const
{
	draw_elements();

	sys().prepare_2d_drawing();
	for (int i = et_begin; i != et_end; ++i) {
		if (element_for_id(i).is_mouse_over(mouse_position)) {
			font_vtremington12->print_hc(mouse_position.x, mouse_position.y - font_arial->get_height(),
					     texts::get(get_description(element_type(i))),
					     get_color(element_type(i)), true);
			break;
		}
	}
	sys().unprepare_2d_drawing();
}



bool sub_captainscabin_display::handle_mouse_button_event(const mouse_click_data& m)
{
	if (m.down()) {
		// just memorize
		mouse_position = m.position_2d;
	} else if (m.up()) {
		mouse_position = m.position_2d;
		if (m.left()) {
			for (int i = et_begin; i != et_end; ++i) {
				if (element_for_id(i).is_mouse_over(mouse_position)) {
					do_action(static_cast<submarine_interface&>(ui), element_type(i));
					return true;
				}
			}
		}
	}
	return false;
}



bool sub_captainscabin_display::handle_mouse_motion_event(const mouse_motion_data& m)
{
	mouse_position = m.position_2d;
	return false;
}
