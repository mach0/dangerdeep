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

// Object to create and display the number and tonnage of sunk ships.
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#ifndef SUB_SOLDBUCH_DISPLAY_H
#define SUB_SOLDBUCH_DISPLAY_H

#include "user_display.h"

class sub_soldbuch_display : public user_display
{
protected:
	std::unique_ptr<image> background;
	std::unique_ptr<image> player_photo;
	std::unique_ptr<image> primary_overlay;
	std::unique_ptr<image> stamps;

public:
	sub_soldbuch_display(class user_interface& ui_);

	void display() const override;

	void enter(bool is_day) override;
	void leave() override;
};

#endif
