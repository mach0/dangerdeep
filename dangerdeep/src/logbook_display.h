/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

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

// Object to create and display logbook entries.
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#ifndef LOGBOOK_DISPLAY_H
#define LOGBOOK_DISPLAY_H

#include "user_display.h"
class image;

class logbook_display : public user_display
{
protected:
	const vector2i page_left_offset;
	const vector2i page_right_offset;
	const vector2i page_size;
	std::unique_ptr<image> background;
	unsigned current_page;
	mutable unsigned nr_of_pages;

	void next_page();
	void previous_page();

public:
	logbook_display(class user_interface& ui_);
	void display(class game& gm) const override;
	void process_input(class game& gm, const SDL_Event& event) override;

	void enter(bool is_day) override;
	void leave() override;
};

#endif
