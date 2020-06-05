/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2016  Thorsten Jordan, Luis Barrancos and others.

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

// SDL2/OpenGL based system input/output
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef SYSTEM_INTERFACE_H
#define SYSTEM_INTERFACE_H

#include <memory>
#include <string>
#include <vector>
#include "input_event_handler.h"
#include "singleton.h"
#include "vector2.h"

///\brief This class groups system related functions like graphic output or user input.
class system_interface : public singleton<system_interface>
{
public:
	/// parameter structure for class system
	struct parameters
	{
		vector2i resolution;		///< X/Y-Resolution of screen/window
		std::string window_caption;	///< Window caption (UTF-8)
		bool fullscreen;		///< Fullscreen mode instead of window?
		bool vertical_sync;		///< Use vertical sync?

		parameters();
	};

	/// Constructor
	system_interface(const parameters& params = parameters());

	/// Destructor
	~system_interface();

	/// Set new parameters, returns true on success
	bool set_parameters(const parameters& params);

	/// Finish frame (rarely needed, call finish_frame() instead!)
	void swap_buffers();

	/// Get the name of a key
	static std::string get_key_name(key_code key, key_mod mod);

	/// Finish drawing of frame in window, will trigger fetching of input events, returns true if program should quit!
	bool finish_frame();

	/// Return global time stamp in milliseconds (inactive process time not counted!)
	uint32_t millisec();

	/// Set a directory for screenshots
	void set_screenshot_directory(const std::string& s) { screenshot_dir = s; }

	/// Take a screenshot
	void screenshot(const std::string& filename = std::string());

	/// set maximum fps rate (0 for unlimited)
	void set_max_fps(unsigned fps) { maxfps = fps; }

	/// Request current resolution
	vector2i get_resolution() const { return params.resolution; }

	/// Request 2D offset on screen
	vector2i get_2D_screen_offset() const { return offset_2D; }

	/// Request 2D size on screen
	vector2i get_2D_screen_size() const { return size_2D; }

	/// Translate pseudo 2D coordinates (1024x768 range) to real coordinates. Note that y-coord is negated!
	vector2f translate_2D_coordinates(const vector2i& c) const { return vector2f(scale_pseudo_2D.x * c.x + offset_pseudo_2D.x, scale_pseudo_2D.y * c.y + offset_pseudo_2D.y); }

	/// Translate pseudo 2D size (1024x768 range) to real size. Note that y-coord is NOT negated!
	vector2f translate_2D_size(const vector2i& c) const { return vector2f(scale_pseudo_2D.x * c.x, scale_pseudo_2D.y * c.y); }

	/// Translate pseudo 2D size (1024x768 range) to real size. Note that y-coord is NOT negated!
	vector2f translate_2D_size(const vector2u& c) const { return vector2f(scale_pseudo_2D.x * c.x, scale_pseudo_2D.y * c.y); }

	/// Request available screen resolutions
	const std::vector<vector2i>& get_available_resolutions() const { return available_resolutions; }

	/// Request if fullscreen mode is set up
	bool is_fullscreen_mode() const { return params.fullscreen; }

	/// Add new event handler on top of stack
	void add_event_handler(std::weak_ptr<input_event_handler> new_handler);

	/// Remove topmost event handler
	void remove_event_handler();

private:
	void prepare_new_resolution();

	system_interface(const system_interface& ) = delete;
	system_interface& operator= (const system_interface& ) = delete;
	system_interface(system_interface&& ) = delete;
	system_interface& operator= (system_interface&& ) = delete;

	void* sdl_main_window;				///< SDL window pointer (made opaque)
	void* sdl_gl_context;				///< SDL GL context pointer (made opaque)
	parameters params;				///< The current parameters.
	vector2i offset_2D;				///< The offset in 2D for drawing
	vector2i size_2D;				///< The size in 2D for drawing
	vector2f scale_pseudo_2D;			///< Scaling for pseudo 2D coordinates
	vector2f offset_pseudo_2D;			///< Offset for pseudo 2D coordinates
	uint32_t time_passed_while_sleeping;		///< Total time in milliseconds that passed while application slept
	uint32_t sleep_time;				///< Time point when application went to sleep
	bool is_sleeping;				///< Flag if application is sleeping
	unsigned maxfps;				///< Maximum frames per second (0: unlimited)
	uint32_t last_swap_time;			///< time of last window swap
	unsigned screenshot_nr;				///< The number of taken screenshots
	std::string screenshot_dir;			///< Directory for screenshots
	std::vector<vector2i> available_resolutions;	///< List of available screen resolutions
	std::vector<std::weak_ptr<input_event_handler>> input_event_handlers;	///< stack of input event handlers
};

/// handy helper function
inline class system_interface& SYS() { return system_interface::instance(); }

#endif
