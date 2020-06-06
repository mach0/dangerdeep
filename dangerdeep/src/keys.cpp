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

// key name definitions
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "keys.h"

key_name key_names[std::size_t(key_command::number)] = {
	{ key_command::ZOOM_MAP, "key_command::ZOOM_MAP" },
	{ key_command::UNZOOM_MAP, "key_command::UNZOOM_MAP" },
	{ key_command::SHOW_GAUGES_SCREEN, "key_command::SHOW_GAUGES_SCREEN" },
	{ key_command::SHOW_PERISCOPE_SCREEN, "key_command::SHOW_PERISCOPE_SCREEN" },
	{ key_command::SHOW_UZO_SCREEN, "key_command::SHOW_UZO_SCREEN" },
	{ key_command::SHOW_BRIDGE_SCREEN, "key_command::SHOW_BRIDGE_SCREEN" },
	{ key_command::SHOW_MAP_SCREEN, "key_command::SHOW_MAP_SCREEN" },
	{ key_command::SHOW_TORPEDO_SCREEN, "key_command::SHOW_TORPEDO_SCREEN" },
	{ key_command::SHOW_DAMAGE_CONTROL_SCREEN, "key_command::SHOW_DAMAGE_CONTROL_SCREEN" },
	{ key_command::SHOW_LOGBOOK_SCREEN, "key_command::SHOW_LOGBOOK_SCREEN" },
	{ key_command::SHOW_SUCCESS_RECORDS_SCREEN, "key_command::SHOW_SUCCESS_RECORDS_SCREEN" },
	{ key_command::SHOW_FREEVIEW_SCREEN, "key_command::SHOW_FREEVIEW_SCREEN" },
	{ key_command::RUDDER_LEFT, "key_command::RUDDER_LEFT" },
	{ key_command::RUDDER_HARD_LEFT, "key_command::RUDDER_HARD_LEFT" },
	{ key_command::RUDDER_RIGHT, "key_command::RUDDER_RIGHT" },
	{ key_command::RUDDER_HARD_RIGHT, "key_command::RUDDER_HARD_RIGHT" },
	{ key_command::RUDDER_UP, "key_command::RUDDER_UP" },
	{ key_command::RUDDER_HARD_UP, "key_command::RUDDER_HARD_UP" },
	{ key_command::RUDDER_DOWN, "key_command::RUDDER_DOWN" },
	{ key_command::RUDDER_HARD_DOWN, "key_command::RUDDER_HARD_DOWN" },
	{ key_command::CENTER_RUDDERS, "key_command::CENTER_RUDDERS" },
	{ key_command::THROTTLE_LISTEN, "key_command::THROTTLE_LISTEN" },
	{ key_command::THROTTLE_SLOW, "key_command::THROTTLE_SLOW" },
	{ key_command::THROTTLE_HALF, "key_command::THROTTLE_HALF" },
	{ key_command::THROTTLE_FULL, "key_command::THROTTLE_FULL" },
	{ key_command::THROTTLE_FLANK, "key_command::THROTTLE_FLANK" },
	{ key_command::THROTTLE_STOP, "key_command::THROTTLE_STOP" },
	{ key_command::THROTTLE_REVERSE, "key_command::THROTTLE_REVERSE" },	// means reverse slow...
	{ key_command::THROTTLE_REVERSEHALF, "key_command::THROTTLE_REVERSEHALF" },
	{ key_command::THROTTLE_REVERSEFULL, "key_command::THROTTLE_REVERSEFULL" },
	{ key_command::FIRE_TUBE_1, "key_command::FIRE_TUBE_1" },
	{ key_command::FIRE_TUBE_2, "key_command::FIRE_TUBE_2" },
	{ key_command::FIRE_TUBE_3, "key_command::FIRE_TUBE_3" },
	{ key_command::FIRE_TUBE_4, "key_command::FIRE_TUBE_4" },
	{ key_command::FIRE_TUBE_5, "key_command::FIRE_TUBE_5" },
	{ key_command::FIRE_TUBE_6, "key_command::FIRE_TUBE_6" },
	{ key_command::SELECT_TARGET, "key_command::SELECT_TARGET" },
	{ key_command::SCOPE_UP_DOWN, "key_command::SCOPE_UP_DOWN" },
	{ key_command::CRASH_DIVE, "key_command::CRASH_DIVE" },
	{ key_command::GO_TO_SNORKEL_DEPTH, "key_command::GO_TO_SNORKEL_DEPTH" },
	{ key_command::TOGGLE_SNORKEL, "key_command::TOGGLE_SNORKEL" },
	{ key_command::SET_HEADING_TO_VIEW, "key_command::SET_HEADING_TO_VIEW" },
	{ key_command::IDENTIFY_TARGET, "key_command::IDENTIFY_TARGET" },
	{ key_command::GO_TO_PERISCOPE_DEPTH, "key_command::GO_TO_PERISCOPE_DEPTH" },
	{ key_command::GO_TO_SURFACE, "key_command::GO_TO_SURFACE" },
	{ key_command::FIRE_TORPEDO, "key_command::FIRE_TORPEDO" },
	{ key_command::SET_VIEW_TO_HEADING, "key_command::SET_VIEW_TO_HEADING" },
	{ key_command::TOGGLE_ZOOM_OF_VIEW, "key_command::TOGGLE_ZOOM_OF_VIEW" },
	{ key_command::TURN_VIEW_LEFT, "key_command::TURN_VIEW_LEFT" },
	{ key_command::TURN_VIEW_LEFT_FAST, "key_command::TURN_VIEW_LEFT_FAST" },
	{ key_command::TURN_VIEW_RIGHT, "key_command::TURN_VIEW_RIGHT" },
	{ key_command::TURN_VIEW_RIGHT_FAST, "key_command::TURN_VIEW_RIGHT_FAST" },
	{ key_command::TIME_SCALE_UP, "key_command::TIME_SCALE_UP" },
	{ key_command::TIME_SCALE_DOWN, "key_command::TIME_SCALE_DOWN" },
	{ key_command::FIRE_DECK_GUN, "key_command::FIRE_DECK_GUN" },
	{ key_command::TOGGLE_RELATIVE_BEARING, "key_command::TOGGLE_RELATIVE_BEARING" },
	{ key_command::TOGGLE_MAN_DECK_GUN, "key_command::TOGGLE_MAN_DECK_GUN" },
	{ key_command::SHOW_TDC_SCREEN, "key_command::SHOW_TDC_SCREEN" },
	{ key_command::TOGGLE_POPUP, "key_command::TOGGLE_POPUP" },
	{ key_command::SHOW_TORPSETUP_SCREEN, "key_command::SHOW_TORPSETUP_SCREEN" },
	{ key_command::SHOW_TORPEDO_CAMERA, "key_command::SHOW_TORPEDO_CAMERA" },
	{ key_command::TAKE_SCREENSHOT, "key_command::TAKE_SCREENSHOT" }
};
