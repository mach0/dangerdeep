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

#ifndef SUB_GHG_DISPLAY_H
#define SUB_GHG_DISPLAY_H

#include "user_display.h"
#include "image.h"
#include <vector>

/// Display for the Gruppenhorchgeraet hearing device
class sub_ghg_display : public user_display
{
public:
	sub_ghg_display(class user_interface& ui_);
	bool handle_mouse_button_event(const mouse_click_data& ) override;
	bool handle_mouse_motion_event(const mouse_motion_data& ) override;
	void display() const override;

protected:
	enum turnknobtype {
		TK_NONE = -1,
		TK_DIRECTION = 0,
		TK_VOLUME = 1,
		TK_NR = 2
	};

	turnknobtype turnknobdrag;
	std::vector<float> turnknobang;
};

#endif
