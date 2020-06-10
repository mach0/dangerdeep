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

// user display: submarine's gauges
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_GAUGES_DISPLAY_H
#define SUB_GAUGES_DISPLAY_H

#include "user_display.h"

#include "texture.h"
#include "image.h"
#include "angle.h"
#include <vector>

///\brief Class for display and input of submarine's main gauges.
///\ingroup displays
class sub_gauges_display : public user_display
{
	struct indicator {
		texture mytex;
		unsigned x, y, w, h;
		indicator(const sdl_image& s, unsigned x_, unsigned y_, unsigned w_, unsigned h_);
		void display(double angle) const;
		bool is_over(vector2i pos) const;
		angle get_angle(vector2i pos) const;
	};

	std::unique_ptr<image> controlscreen;

	std::unique_ptr<indicator> indicator_compass;
	std::unique_ptr<indicator> indicator_bow_depth_rudder;
	std::unique_ptr<indicator> indicator_stern_depth_rudder;
	std::unique_ptr<indicator> indicator_depth;
	std::unique_ptr<indicator> indicator_knots;
	std::unique_ptr<indicator> indicator_main_rudder;
	std::unique_ptr<indicator> indicator_mt;

// old control screen, now for type II had these indicators:
//	enum { compass, battery, compressor, diesel, bow_depth_rudder, stern_depth_rudder,
//	       depth, knots, main_rudder, mt, nr_of_indicators };

	mutable int throttle_angle;        // mutable because it will be changed in display()

protected:
	int compute_throttle_angle(int throttle_pos) const;

public:
	sub_gauges_display(class user_interface& ui_);

	void display() const override;
	bool handle_mouse_button_event(const mouse_click_data& ) override;

	void enter(bool is_day) override;
	void leave() override;
};

#endif
