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

// user display: submarine's torpedo setup display
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUB_TORPSETUP_DISPLAY_H
#define SUB_TORPSETUP_DISPLAY_H

#include "user_display.h"
#include "image.h"
#include <vector>

class sub_torpsetup_display : public user_display
{
	class scheme {
	public:
		std::unique_ptr<image> background;
		rotat_tex rundepthptr;
		rotat_tex secondaryrangeptr;
		rotat_tex primaryrangeptr;
		rotat_tex torpspeeddial;
		rotat_tex turnangledial;
		rotat_tex primaryrangedial;
		// everything that does not rotate could also be an "image"...
		// but only when this doesn't trash the image cache
		std::unique_ptr<texture> torpspeed[3];	// slow/medium/fast
		std::unique_ptr<texture> firstturn[2];	// left/right
		std::unique_ptr<texture> secondaryrange[2];	// short/long
		std::unique_ptr<texture> preheating[2];	// on/off
		std::unique_ptr<texture> temperaturescale;
		rotat_tex primaryrangeknob[6];
		rotat_tex turnangleknob[6];
		rotat_tex rundepthknob[6];
		bool is_over(const std::unique_ptr<texture>& tex, const vector2i& pos,
			     vector2i mouse_position, int border = 32) const {
			return (mouse_position.x >= pos.x - border)
				&& (mouse_position.y >= pos.y - border)
				// 2006-11-30 doc1972 only hight and width is unsigned, so we cast the result back to signed
				&& (mouse_position.x < (int)(pos.x + tex->get_width() + border))
				&& (mouse_position.y < (int)(pos.y + tex->get_height() + border));
		}
		scheme(bool day);
	protected:
		scheme();
		scheme(const scheme& );
		scheme& operator= (const scheme& );
	};

	enum turnknobtype {
		TK_NONE = -1,
		TK_PRIMARYRANGE = 0,
		TK_TURNANGLE = 1,
		TK_RUNDEPTH = 2,
		TK_NR = 3
	};

	std::unique_ptr<scheme> myscheme;

	turnknobtype turnknobdrag;
	std::vector<float> turnknobang;

	/*
	unsigned selected_tube;	// 0-5
	unsigned selected_mode;	// 0-1 (automatic on / off)
	*/

public:
	sub_torpsetup_display(class user_interface& ui_);

	bool handle_mouse_button_event(const mouse_click_data& ) override;
	bool handle_mouse_motion_event(const mouse_motion_data& ) override;
	void display() const override;

	void enter(bool is_day) override;
	void leave() override;
};

#endif
