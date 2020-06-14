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



sub_tdc_display::scheme_screen1::scheme_screen1(bool day)
{
	const string x = day ? "TDCScreen1_Daylight" : "TDCScreen1_Redlight";
	background = std::make_unique<image>(get_image_dir() + x + "_Base_image.jpg|png");
	aob_ptr.set(x + "_Lagenwinkel_pointer_rotating.png", 484, 426, 512, 551);
	aob_inner.set(x + "_AngleOnBow_innerdial_rotating.png", 417, 456, 512, 550);
	spread_ang_ptr.set(x + "_Facherwinkel_pointer_rotating.png", 799, 502, 846, 527);
	spread_ang_mkr.set(x + "_Facherwinkel_usermarker_rotating.png", 950, 512, 846, 527);
	firesolution = std::make_unique<texture>(get_image_dir() + x + "_firesolution_slidingscale_pointer.png");
	parallax_ptr.set(x + "_parallaxwinkel_pointer_rotating.png", 820, 104, 846, 201);
	parallax_mkr.set(x + "_parallaxwinkel_usermarker_rotating.png", 952, 186, 846, 201);
	torptime_min.set(x + "_TorpedoLaufzeit_pointer_minutes_rotating.png", 175, 484, 195, 563);
	torptime_sec.set(x + "_TorpedoLaufzeit_pointer_seconds_rotating.png", 170, 465, 195, 563);
	torp_speed.set(x + "_TorpGeschwindigkeit_innerdial_rotating.png", 406, 83, 512, 187);
	target_pos.set(x + "_Zielposition_pointer_rotating.png", 158, 86, 183, 183);
	target_speed.set(x + "_ZielTorpGeschwindigkeit_pointer_rotating.png", 484, 62, 511, 187);
}



sub_tdc_display::sub_tdc_display(user_interface& ui_)
	: user_display(ui_), tubeselected_time(0.0)
{
}



bool sub_tdc_display::handle_mouse_button_event(const mouse_click_data& m)
{
	auto& gm = ui.get_game();
	auto* sub = dynamic_cast<submarine*>(gm.get_player());
	auto& si = dynamic_cast<submarine_interface&>(ui);
	tdc& TDC = sub->get_tdc();

	if (m.down() && m.left()) {
		if (!myscheme1.get()) THROW(error, "sub_tdc_display without scheme!");
		const scheme_screen1& s = *myscheme1;
		// check if mouse is over parallax display
		int parasz = s.parallax_ptr.center.y - s.parallax_ptr.left_top.y + 20;
		if (m.position_2d.x >= s.parallax_ptr.center.x - parasz
		    && m.position_2d.x <= s.parallax_ptr.center.x + parasz
		    && m.position_2d.y >= s.parallax_ptr.center.y - parasz
		    && m.position_2d.y <= s.parallax_ptr.center.y + parasz) {
			auto v = vector2(m.position_2d - s.parallax_ptr.center);
			v.y = -v.y;
			angle userang(v);
			double usera = userang.value_pm180() / 6;
			if (usera < -25) usera = -25;
			if (usera > 25) usera = 25;
			TDC.set_additional_parallaxangle(usera);
		}
	}
	return true;
}



bool sub_tdc_display::handle_mouse_motion_event(const mouse_motion_data& m)
{
	auto& gm = ui.get_game();
	auto* sub = dynamic_cast<submarine*>(gm.get_player());
	tdc& TDC = sub->get_tdc();

	if (m.left()) {
		if (!myscheme1.get()) THROW(error, "sub_tdc_display without scheme!");
		const scheme_screen1& s = *myscheme1;
		// check if mouse is over parallax display, fixme: same code as above, group it!
		int parasz = s.parallax_ptr.center.y - s.parallax_ptr.left_top.y + 20;
		if (m.position_2d.x >= s.parallax_ptr.center.x - parasz
		    && m.position_2d.x <= s.parallax_ptr.center.x + parasz
		    && m.position_2d.y >= s.parallax_ptr.center.y - parasz
		    && m.position_2d.y <= s.parallax_ptr.center.y + parasz) {
			auto v = vector2(m.position_2d - s.parallax_ptr.center);
			v.y = -v.y;
			angle userang(v);
			double usera = userang.value_pm180() / 6;
			if (usera < -25) usera = -25;
			if (usera > 25) usera = 25;
			TDC.set_additional_parallaxangle(usera);
		}
		return true;
	}
	return false;
}



void sub_tdc_display::display() const
{
	auto& gm = ui.get_game();
	auto* player = dynamic_cast<submarine*>(gm.get_player());

	sys().prepare_2d_drawing();

	const tdc& TDC = player->get_tdc();

	if (!myscheme1.get()) THROW(error, "sub_tdc_display::display without scheme!");
	const scheme_screen1& s = *myscheme1;

	// draw torpedo speed dial (15deg = 0, 5knots = 30deg)
	// torpedo speed (depends on selected tube!), but TDC is already set accordingly
	s.torp_speed.draw(sea_object::ms2kts(TDC.get_torpedo_speed()) * 330.0/55 + 15);

	// angle on the bow finer value, note use real fmod here...
	s.aob_inner.draw(fmod(TDC.get_angle_on_the_bow().value_pm180(), 10.0) * -36.0);

	// background
	s.background->draw(0, 0);

	// angle on the bow coarse value
	s.aob_ptr.draw(TDC.get_angle_on_the_bow().value_pm180());

	// spread angle, fixme: add. lead angle is not right...
	// this means angle of spread when firing multiple torpedoes... this has to be (re)defined
	// the captain could fake additional lead angle by manipulating bearing etc.
	// this should be done to compensate ship turning or zig-zagging
	s.spread_ang_ptr.draw(0.0 /*TDC.get_spread_angle().value()*/ /20 * 180.0 - 90);
	s.spread_ang_mkr.draw(15.0/*TDC.get_user_spread_angle().value()*/ /20 * 180.0 - 90);	//fixme

	// fire solution quality
	double quality = 0.333; // per cent, fixme, request from sub! depends on crew
	s.firesolution->draw(268 - int(187*quality + 0.5), 418);

	// parallax angle (fixme: why should the user set an angle? extra-correction here? is like
	// additional lead angle...)
	// 6 pointer degrees for 1 real degree, marker - 90
	double parang = TDC.get_parallax_angle().value_pm180();
	// clamp value (maybe tweak value so that pointer shakes when reaching the limit?)
	if (parang < -26) parang = -26;
	if (parang > 26) parang = 26;
	s.parallax_ptr.draw(parang * 6);
	s.parallax_mkr.draw(TDC.get_additional_parallaxangle().value_pm180() * 6 - 90);

	// torpedo run time
	double t = TDC.get_torpedo_runtime();
	s.torptime_sec.draw(myfmod(t, 60) * 6);
	s.torptime_min.draw(myfmod(t, 3600) * 0.1);

	// target bearing (influenced by quality!)
	s.target_pos.draw((TDC.get_bearing() - player->get_heading()).value());

	// target speed
	s.target_speed.draw(15 + sea_object::ms2kts(TDC.get_target_speed()) * 330.0/55);

	ui.draw_infopanel(true);
	sys().unprepare_2d_drawing();
}



void sub_tdc_display::enter(bool is_day)
{
	myscheme1 = std::make_unique<scheme_screen1>(is_day);
}



void sub_tdc_display::leave()
{
	myscheme1.reset();
}
