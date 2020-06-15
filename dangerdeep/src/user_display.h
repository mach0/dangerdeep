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

// Base interface for user screens.
// subsim (C)+(W) Markus Petermann and Thorsten Jordan. SEE LICENSE

#ifndef USER_DISPLAY_H
#define USER_DISPLAY_H

#include <list>
#include "texture.h"	// fixme replace by "gpu_interface.h"
#include "datadirs.h"
#include "input_event_handler.h"
class image;

///\defgroup displays In-game user interface screens
///\brief Base class for a single screen of the ingame user interface.
///\ingroup displays
class user_display : public input_event_handler
{
private:
	// no empty construction, no copy
	user_display() = delete;
	user_display(user_display& ) = delete;
	user_display& operator= (const user_display& ) = delete;

protected:
	// common functions: draw_infopanel(class game& gm)

	// the display needs to know its parent (user_interface) to access common data
	class user_interface& ui;

	user_display(class user_interface& ui_, const char* display_name = nullptr/*fixme empty default until all displays are converted*/);

#if 1 // fixme remove!
	// commonly used helper classes
	class rotat_tex {
	public:
		rotat_tex()  = default;
		std::unique_ptr<texture> tex;
		vector2i left_top;
		vector2i center;
		void draw(double angle) const {
			// fixme: maybe rotate around pixel center (x/y + 0.5)
			tex->draw_rot(center.x, center.y, angle, center.x - left_top.x, center.y - left_top.y);
		}
		void set(texture* tex_, int left_, int top_, int centerx_, int centery_) {
			tex.reset(tex_);
			left_top = {left_, top_};
			center = {centerx_, centery_};
		}
		void set(const std::string& filename, int left_, int top_, int centerx_, int centery_) {
			set(new texture(get_image_dir() + filename), left_, top_, centerx_, centery_);
		}
		bool is_mouse_over(vector2i pos, int tolerance = 0) const {
			return (pos.x + tolerance >= left_top.x && pos.y + tolerance >= left_top.y
				&& pos.x - tolerance < left_top.x + int(tex->get_width())
				&& pos.y - tolerance < left_top.y + int(tex->get_height()));
		}
	protected:
		rotat_tex(const rotat_tex& );
		rotat_tex& operator= (const rotat_tex& );
	};

	class fix_tex {
	public:
		fix_tex()  = default;
		std::unique_ptr<texture> tex;
		vector2i left_top;
		void draw() const {
			tex->draw(left_top.x, left_top.y);
		}
		void set(texture* tex_, int left_, int top_) {
			tex.reset(tex_);
			left_top.x = left_;
			left_top.y = top_;
		}
		void set(const std::string& filename, int left_, int top_) {
			set(new texture(get_image_dir() + filename), left_, top_);
		}
		bool is_mouse_over(vector2i pos, int tolerance = 0) const {
			return (pos.x + tolerance >= left_top.x && pos.y + tolerance >= left_top.y
				&& pos.x - tolerance < left_top.x + int(tex->get_width())
				&& pos.y - tolerance < left_top.y + int(tex->get_height()));
		}
	protected:
		fix_tex(const fix_tex& );
		fix_tex& operator= (const fix_tex& );
	};
#endif

	/// A 2D image element for normal or rotated image elements
	class elem2D
	{
	public:
		/// Construct static element
		elem2D(const vector2i& pos, const std::string& filename_day, const std::string& filename_night = std::string());
		/// Construct rotateable element
		elem2D(const vector2i& pos, const vector2i& ctr, const std::string& filename_day, const std::string& filename_night = std::string());
		/// Construct static element with N angles/phases
		elem2D(const vector2i& pos, unsigned phases, unsigned offset, const std::string& filename_day, const std::string& filename_night = std::string());
		/// Set the angle to use for rotation in [0...360[
		void set_angle(float angle_) const;
		/// Set the phase to use for subimage in [0...1[
		void set_phase(float phase_) const;
		/// Draw element (rotated/phased if defined)
		void draw() const;
		/// Is Mouse over element? Does not check for rotation, just uses 2D area.
		bool is_mouse_over(const vector2i& mpos, int tolerance = 0) const;
		/// Define drawing dimensions
		void set_draw_size(const vector2i& sz);
		/// Initialize texture
		void init(bool is_day);
		/// Deinitialize texture
		void deinit();
		/// Request size
		//const vector2i& get_size() const { return size; }
		/// Get position
		const vector2i& get_position() const { return position; }
		/// For storage in vector
		elem2D() = default;

	protected:
		vector2i position;		///< Position (left/top) of the element on screen
		vector2i center;		///< Center of the element on screen (used for rotation)
		//vector2i size;		///< Drawing size (taken from image initially)
		bool rotateable{false};	///< Is this a rotateable element?
		bool has_night{false};	///< Does night image data exist?
		mutable float angle{0.f};			///< The angle to use for rotateable elements, mutable because called by const display()
		mutable unsigned phase{0};			///< The subimage (phase) to use, mutable because called by const display()
		std::vector<std::string> filenames_day;// obsolete with new gpu interface
		std::vector<std::string> filenames_night;	// obsolete with new gpu interface
		//std::vector<image> data_day;			///< Image data (always set)
		//std::vector<image> data_night;		///< Image data for night (optional)
		//std::vector<gpu::texture> tex;		///< Texture data (only used when display is active!)
		std::vector<std::unique_ptr<texture>> tex;	///< Texture data (only used when display is active!) // obsolete with new gpu interface
	};

	std::vector<elem2D> elements;		///< Elements to use for drawing
	std::unordered_map<unsigned, unsigned> id_to_element;	///< Mapping IDs from definition to elements
	const elem2D& element_for_id(unsigned id) const;	///< Deliver element for a given ID
	void draw_elements() const;

public:
	// needed for correct destruction of heirs.
	virtual ~user_display() = default;
	// very basic. Just draw display and handle input.
	virtual void display() const = 0;
	// mask contains one bit per popup (at most 31 popups)
	virtual unsigned get_popup_allow_mask() const { return 0; }

	/// overload with code that inits data for that display, like loading images
	virtual void enter(bool is_day);
	/// overload with code that deinits data for that display, like freeing images
	virtual void leave();
};

#endif /* USER_DISPLAY_H */
