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

// game editor
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef GAME_EDITOR_H
#define GAME_EDITOR_H

#include "game.h"

///\brief Central object of the game world with physics simulation etc. Editor specialization
class game_editor : public game
{
protected:
	//game_editor();
	game_editor& operator= (const game_editor& other);
	game_editor(const game_editor& other);

	/// is time simulation enabled in the editor? can be disabled for editing
	bool time_sim_enabled;

	/// this value controls if time simulation is paused or not (used when menues are open)
	bool time_sim_paused;

public:
	/// create new editor instance. subtype can be changed later.
	game_editor(/*const std::string& subtype*/);

	// create from mission file or savegame (xml file)
	game_editor(const std::string& filename);

	void simulate(double delta_t);

	// is editor?
	virtual bool is_editor() const { return true; }

	// main game loop
	run_state exec();
};

#endif