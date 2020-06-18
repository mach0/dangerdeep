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

// test program for displays
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "mymain.cpp"
#include "user_display.h"
#include "user_interface.h"
#include "system_interface.h"

class test_display : public user_display
{
public:
	test_display(user_interface& ui_, const char* name) : user_display(ui_, name) {}
	void set_time(double t)
	{
		// show value range every N seconds
		const auto secs = 5.0;
		const auto f = helper::mod(t, secs) / secs;
		for (auto& elem : elements) {
			// switch also phases, visibility etc.
		}
	}
	void draw_elements() const override
	{
		user_display::draw_elements();
	}
	void check_mouse(const vector2i& mpos)
	{
		for (auto& elem : elements) {
			if (elem.is_mouse_over(mpos)) {
				elem.set_value(mpos);
			}
		}
	}
};



int mymain(std::vector<string>& args)
{
	if (args.size() != 1) {
		return -1;
	}

	system_interface::parameters params;
	params.near_z = 1.0;
	params.far_z = 1000.0;
	params.resolution = {1024, 768};
	params.resolution2d = {1024,768};
	params.fullscreen = false;
	system_interface::create_instance(new class system_interface(params));

	user_interface& ui = *(user_interface*)nullptr;	//fixme we need a stub here that delivers valid data! at least for all calls
	test_display td(ui, args[0].c_str());

	// allow change of every element by mouse click, variate all values by time, until ESC pressed

	bool doquit = false;
	auto ic = std::make_shared<input_event_handler_custom>();
	ic->set_handler([&](const input_event_handler::key_data& k) {
		if (k.down()) {
			switch (k.keycode) {
			case key_code::ESCAPE:
				doquit = true; break;
			default:
				return false;
				break;
			}
		}
		return true;
	});
	ic->set_handler([&](const input_event_handler::mouse_click_data& m) {
		if (m.down() && m.left()) {
			// set angle/value by click
			td.check_mouse(m.position_2d);
		}
		return true;
	});
	ic->set_handler([&](const input_event_handler::mouse_motion_data& m) {
		if (m.left()) {
			// set angle/value by dragging
			td.check_mouse(m.position_2d);
		}
		return true;
	});
	sys().add_input_event_handler(ic);

	while (!doquit) {
		td.set_time(sys().millisec() / 1000.0);
		td.draw_elements();
		sys().finish_frame();
	}

	system_interface::destroy_instance();

	return 0;
}
