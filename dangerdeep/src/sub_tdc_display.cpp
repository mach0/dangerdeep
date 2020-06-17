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

// user display: submarine's tdc
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sub_tdc_display.h"
#include "cfg.h"
#include "game.h"
#include "global_data.h"
#include "image.h"
#include "keys.h"
#include "log.h"
#include "submarine.h"
#include "submarine_interface.h"
#include "system_interface.h"
#include "texture.h"
#include <memory>

#include <sstream>
#include <utility>

namespace
{
	enum element_type {
		et_torp_speed = 0,
		et_aob_inner = 1,
		et_aob_ptr = 2,
		et_spread_ang_ptr = 3,
		et_spread_ang_mkr = 4,
		et_firesolution = 5,
		et_parallax_ptr = 6,
		et_parallax_mkr = 7,
		et_torptime_sec = 8,
		et_torptime_min = 9,
		et_target_pos = 10,
		et_target_speed = 11
	};
}



sub_tdc_display::sub_tdc_display(user_interface& ui_)
	: user_display(ui_, "sub_tdc")
{
}



bool sub_tdc_display::handle_mouse_button_event(const mouse_click_data& m)
{
	auto& gm = ui.get_game();
	auto* sub = dynamic_cast<submarine*>(gm.get_player());
	tdc& TDC = sub->get_tdc();

	if (m.down() && m.left()) {
		// check if mouse is over parallax display
		if (element_for_id(et_parallax_ptr).is_mouse_over(m.position_2d)) {
			const auto dir = element_for_id(et_parallax_ptr).set_value(m.position_2d);
			if (dir.has_value()) {
				TDC.set_additional_parallaxangle(dir.value());
			}
		}
		return true;
	}
	return false;
}



bool sub_tdc_display::handle_mouse_motion_event(const mouse_motion_data& m)
{
	auto& gm = ui.get_game();
	auto* sub = dynamic_cast<submarine*>(gm.get_player());
	tdc& TDC = sub->get_tdc();

	if (m.left()) {
		// check if mouse is over parallax display, fixme: same code as above, group it!
		if (element_for_id(et_parallax_ptr).is_mouse_over(m.position_2d)) {
			const auto dir = element_for_id(et_parallax_ptr).set_value(m.position_2d);
			if (dir.has_value()) {
				TDC.set_additional_parallaxangle(dir.value());
			}
		}
		return true;
	}
	return false;
}



void sub_tdc_display::display() const
{
	auto& gm = ui.get_game();
	auto* player = dynamic_cast<submarine*>(gm.get_player());
	const tdc& TDC = player->get_tdc();

	element_for_id(et_torp_speed).set_value(sea_object::ms2kts(TDC.get_torpedo_speed()));
	element_for_id(et_aob_inner).set_value(TDC.get_angle_on_the_bow().value_pm180());
	element_for_id(et_aob_ptr).set_value(TDC.get_angle_on_the_bow().value_pm180());
	// spread angle, fixme: add. lead angle is not right...
	// this means angle of spread when firing multiple torpedoes... this has to be (re)defined
	// the captain could fake additional lead angle by manipulating bearing etc.
	// this should be done to compensate ship turning or zig-zagging
	element_for_id(et_spread_ang_ptr).set_value(0.0);
	element_for_id(et_spread_ang_mkr).set_value(15.0);
	element_for_id(et_firesolution).set_value(0.333); // fire solution quality, factor, fixme, request from sub! depends on crew
	// parallax angle (fixme: why should the user set an angle? extra-correction here? is like
	// additional lead angle...)
	// 6 pointer degrees for 1 real degree, marker - 90
	// fixme the marker is changed?
	element_for_id(et_parallax_ptr).set_value(TDC.get_parallax_angle().value_pm180());
	element_for_id(et_parallax_mkr).set_value(TDC.get_parallax_angle().value_pm180());
	const auto t = TDC.get_torpedo_runtime();
	element_for_id(et_torptime_sec).set_value(myfmod(t, 60));
	element_for_id(et_torptime_min).set_value(myfmod(t, 3600));
	element_for_id(et_target_pos).set_value((TDC.get_bearing() - player->get_heading()).value());
	element_for_id(et_target_speed).set_value(sea_object::ms2kts(TDC.get_target_speed()));
	// fixme all click radii, min/max values etc are missing!
	draw_elements();
}
