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

#include "sub_tdc2_display.h"
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


using std::vector;
using std::list;
using std::string;
using std::ostringstream;

static const int tubelightcoordx[6] = {
	 81,
	203,
	324,
	446,
	569,
	693,
};



sub_tdc2_display::scheme_screen2::scheme_screen2(bool day)
{
	const string x = day ? "TDCScreen2_Daylight" : "TDCScreen2_Redlight";
	background = std::make_unique<image>(get_image_dir() + x + "_base_image.jpg");
	for (unsigned i = 0; i < 6; ++i) {
		tubelight[i].set(x + "_tube" + str(i+1) + "_on.png", tubelightcoordx[i], 605);
	}
	firebutton.set(x + "_FireButton_ON.png", 68, 92);
	automode[0].set(x + "_AutoManualKnob_automode.png", 900, 285);
	automode[1].set(x + "_AutoManualKnob_manualmode.png", 900, 285);
	gyro_360.set(x + "_HGyro_pointer_main_rotating.png", 383, 406, 431, 455);
	gyro_10.set(x + "_HGyro_pointer_refinement_rotating.png", 188, 378, 212, 455);
	brightness.set(x + "_Brightness_dial_pointer_rotating.png", 897, 478, 911, 526);
	target_course_360.set(x + "_VGyro_pointer_main_rotating.png", 695, 373, 721, 453);
	target_course_10.set(x + "_VGyro_pointer_refinement_rotating.png", 696, 152, 721, 233);
	target_range_ptr.set(x + "_Zielentfernung_pointer_rotating.png", 317, 98, 341, 194);
	target_range_mkr.set(x + "_Zielentfernung_user_marker_rotating.png", 325, 295, 341, 194);
}



sub_tdc2_display::sub_tdc2_display(user_interface& ui_)
	: user_display(ui_), tubeselected_time(0.0)
{
}



bool sub_tdc2_display::handle_mouse_button_event(const mouse_click_data& m)
{
	auto& gm = ui.get_game();
	auto* sub = dynamic_cast<submarine*>(gm.get_player());
	auto& si = dynamic_cast<submarine_interface&>(ui);
	tdc& TDC = sub->get_tdc();

	if (m.down() && m.left()) {
		if (!myscheme2.get()) THROW(error, "sub_tdc2_display without scheme!");
		const scheme_screen2& s = *myscheme2;

		// check if mouse is over tube indicators
		unsigned nrtubes = sub->get_nr_of_bow_tubes() + sub->get_nr_of_stern_tubes();
		for (unsigned i = 0; i < nrtubes; ++i) {
			if (s.tubelight[i].is_mouse_over(m.position_2d)) {
				si.select_tube(i);
				log_debug("Torpedo tube selected: #" << i+1);
				tubeselected_time = gm.get_time();
			}
		}

		// fire button
		if (s.firebutton.is_mouse_over(m.position_2d)) {
			si.fire_tube(sub, si.get_selected_tube());
		}

		// auto mode
		else if (s.automode[0].is_mouse_over(m.position_2d)) {
			TDC.set_auto_mode(!TDC.auto_mode_enabled());
		}
	}
	return true;
}



void sub_tdc2_display::display() const
{
	auto& gm = ui.get_game();
	auto* player = dynamic_cast<submarine*>(gm.get_player());

	sys().prepare_2d_drawing();

	const tdc& TDC = player->get_tdc();

	if (!myscheme2.get()) THROW(error, "sub_tdc2_display::display without scheme!");
	const scheme_screen2& s = *myscheme2;

	// background
	s.background->draw(0, 0);

	unsigned selected_tube = dynamic_cast<const submarine_interface&>(ui).get_selected_tube();

	// draw tubes if ready
	const double blink_duration = 3.0;
	const double blink_period_duration = 0.25;
	for (unsigned i = 0; i < 6; ++i) {
		if (player->is_tube_ready(i)) {
			if (selected_tube != i || gm.get_time() > tubeselected_time + blink_duration ||
			    (unsigned(floor((gm.get_time() - tubeselected_time) / blink_period_duration)) & 1)) {
				s.tubelight[i].draw();
			}
		}
	}

	// fire button
	if (player->is_tube_ready(selected_tube) && TDC.solution_valid()) {
		s.firebutton.draw();
	}

	// automatic fire solution on / off switch
	s.automode[TDC.auto_mode_enabled() ? 0 : 1].draw();

	// draw gyro pointers
	angle leadangle = TDC.get_lead_angle();
	s.gyro_360.draw(leadangle.value());
	s.gyro_10.draw(myfmod(leadangle.value(), 10.0) * 36.0);

	// target values (influenced by quality!)
	double tgtcourse = TDC.get_target_course().value();
	s.target_course_360.draw(tgtcourse);
	s.target_course_10.draw(myfmod(tgtcourse, 10.0) * 36.0);

	// target range
	double tgtrange = TDC.get_target_distance();
	// clamp displayed value
	if (tgtrange < 300) tgtrange = 300;
	if (tgtrange > 11000) tgtrange = 11000;
	// compute non-linear dial value
	tgtrange = sqrt(12.61855670103 * tgtrange - 3685.567010309);
	s.target_range_ptr.draw(tgtrange);
	//fixme: get tgt range marker also... or store it in this screen class?
	//hmm no the TDC needs to now user input, so store it there...

	// fixme: show some sensible value
	s.brightness.draw(45);

	ui.draw_infopanel(true);
	sys().unprepare_2d_drawing();
}



void sub_tdc2_display::enter(bool is_day)
{
	myscheme2 = std::make_unique<scheme_screen2>(is_day);
}



void sub_tdc2_display::leave()
{
	myscheme2.reset();
}
