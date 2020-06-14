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

// user display: submarine's tdc screen 2
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_TDC2_DISPLAY_H
#define SUB_TDC2_DISPLAY_H

#include "user_display.h"
#include "image.h"
#include <vector>

class sub_tdc2_display : public user_display
{
	class scheme_screen2 {
	public:
		std::unique_ptr<image> background;
		// fixme: replace texture::ptr by fix_tex in all display classes where it makes sense
		fix_tex tubelight[6];
		fix_tex firebutton;
		fix_tex automode[2];	// on/off
		rotat_tex gyro_360;
		rotat_tex gyro_10;
		rotat_tex brightness;	// fixme: do we need that?
		rotat_tex target_course_360;
		rotat_tex target_course_10;
		// fixme add marker for course here (2x)
		rotat_tex target_range_ptr;
		rotat_tex target_range_mkr;
		scheme_screen2(bool is_day);
	protected:
		scheme_screen2();
		scheme_screen2(const scheme_screen2& );
		scheme_screen2& operator= (const scheme_screen2& );
	};

	std::unique_ptr<scheme_screen2> myscheme2;

	// automatic: means user puts in values or crew - fixme to be defined...
	unsigned selected_mode;	// 0-1 (automatic on / off)

	///> last time of game that a tube was selected, used for display
	double tubeselected_time;

public:
	sub_tdc2_display(class user_interface& ui_);

	//overload for zoom key handling ('y') and TDC input
	bool handle_mouse_button_event(const mouse_click_data& ) override;
	void display() const override;

	void enter(bool is_day) override;
	void leave() override;
};

#endif
