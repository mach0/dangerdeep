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

// Submarine tdc popup.
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "datadirs.h"
#include "game.h"
#include "sub_tdc_popup.h"
#include "user_interface.h"


sub_tdc_popup::sub_tdc_popup(user_interface& ui_)
 :	user_popup(ui_)
 ,	background(elem2D({9, 151}, get_image_dir() + "popup_control_daylight.jpg|png", get_image_dir() + "popup_control_redlight.jpg|png"))
{
}





void sub_tdc_popup::display() const
{
	bool is_day = ui.get_game().is_day_mode();
	background.draw(is_day);
}
