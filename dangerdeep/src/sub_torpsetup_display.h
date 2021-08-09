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

#pragma once

#include "user_display.h"

/// Display for the torpedo setup display for submarines
class sub_torpsetup_display : public user_display
{
  public:
    sub_torpsetup_display(class user_interface& ui_);
    bool handle_mouse_button_event(const mouse_click_data&) override;
    bool handle_mouse_motion_event(const mouse_motion_data&) override;
    void display() const override;

  protected:
    int which_element_is_turned{-1};
};
