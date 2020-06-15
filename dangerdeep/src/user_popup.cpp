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

// Base interface for user screen popups.
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "user_popup.h"
#include "system_interface.h"

user_popup::elem2D::elem2D(vector2i pos, const std::string& filename_day, const std::string& filename_night)
 :	position(pos)
{
	tex_day = std::make_unique<image>(filename_day);
	if (!filename_night.empty()) {
		tex_night = std::make_unique<image>(filename_night);
	}
}



void user_popup::elem2D::draw(bool is_day) const
{
	sys().prepare_2d_drawing();
	if (is_day || tex_night.get() == nullptr) {
		tex_day->draw(position.x, position.y);
	} else {
		tex_night->draw(position.x, position.y);
	}
	sys().unprepare_2d_drawing();
}
