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
#include "image.h"
#include "submarine_interface.h"
#include "system_interface.h"
#include "texts.h"
#include "texture.h"
#include "torpedo.h"
#include <fstream>
#include <memory>

#include <sstream>
#include <utility>

using namespace std;



void sub_captainscabin_display::goto_successes()
{
	(dynamic_cast<submarine_interface&>(ui)).goto_successes();
}

void sub_captainscabin_display::goto_logbook()
{
	(dynamic_cast<submarine_interface&>(ui)).goto_logbook();
}

void sub_captainscabin_display::goto_torpedoes()
{
	(dynamic_cast<submarine_interface&>(ui)).goto_torpedomanagement();
}

void sub_captainscabin_display::goto_recogmanual()
{
	(dynamic_cast<submarine_interface&>(ui)).goto_recogmanual();
}

sub_captainscabin_display::clickable_area::clickable_area(const vector2i& tl, const vector2i& br,
							  int descr,
							  void (sub_captainscabin_display::*func)(),
							  color dc)
	: topleft(tl), bottomright(br), description(descr), action(func), desc_color(dc)
{
}

bool sub_captainscabin_display::clickable_area::is_mouse_over(vector2i pos) const
{
	return (pos.x >= topleft.x && pos.x <= bottomright.x &&
		pos.y >= topleft.y && pos.y <= bottomright.y);
}

void sub_captainscabin_display::clickable_area::do_action(sub_captainscabin_display& obj)
{
	(obj.*action)();
}

sub_captainscabin_display::sub_captainscabin_display(user_interface& ui_) :
	user_display(ui_)
{
	clickable_areas.emplace_back(vector2i(0, 540), vector2i(292,705), 272, &sub_captainscabin_display::goto_successes, color(255, 224, 224));
	clickable_areas.emplace_back(vector2i(415, 495), vector2i(486,520), 255, &sub_captainscabin_display::goto_logbook, color(224, 224, 255));
	clickable_areas.emplace_back(vector2i(713, 176), vector2i(862, 575), 253, &sub_captainscabin_display::goto_torpedoes, color(224, 255, 224));
	clickable_areas.emplace_back(vector2i(405, 430), vector2i(462,498), 273, &sub_captainscabin_display::goto_recogmanual, color(255, 224, 224));
}

void sub_captainscabin_display::display() const
{
//	submarine* sub = dynamic_cast<submarine*>(gm.get_player());

	// draw background
	sys().prepare_2d_drawing();

	background->draw(0, 0);

	for (const auto & it : clickable_areas) {
		if (it.is_mouse_over(mouse_position)) {
			font_vtremington12->print_hc(mouse_position.x, mouse_position.y - font_arial->get_height(),
					     texts::get(it.get_description()),
					     it.get_description_color(), true);
			break;
		}
	}

	ui.draw_infopanel();
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
			for (auto& it : clickable_areas) {
				if (it.is_mouse_over(mouse_position)) {
					it.do_action(*this);
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



void sub_captainscabin_display::enter(bool is_day)
{
	background = std::make_unique<image>(get_image_dir() + "captainscabin_main_"
				   + (is_day ? "daylight" : "redlight") + ".jpg");
}

void sub_captainscabin_display::leave()
{
	background.reset();
}
