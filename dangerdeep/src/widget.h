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

// OpenGL based widgets
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef WIDGET_H
#define WIDGET_H

#include <string>
#include <vector>
#include "system_interface.h"
#include "color.h"
#include "image.h"
#include "font.h"
#include "model.h"
#include "vector2.h"
#include "objcache.h"
#include "xml.h"
#include "texts.h"
#include "input_event_handler.h"

// fixme: add image-widget

// fixme: when editing in a widget_edit you have to click twice on a button to lose focus AND click the button .. bad

// fixme: make yes/no, ok, yes/no/cancel dialogue
// when a dialogue opens another one, both should be drawn, and the parent
// should pass draw()/process_input() calls to its child
// make a special flag: widget* wait_for?
// process_input(){if (wait_for)wait_for->process_input();else ...old...;} ?
// ein widget.close fehlt. close:= parent.remove(this), wenn parent==0 dann globale liste nach this
// durchsuchen. run() läuft dann bis globale liste leer ist.

// more widgets: progress bar

// theme files:
// two images, one for elements, one for icons.
// each image is one row of square elements.
// elements:
// 2 for background/sunken background
// 2*8 for borders (clockwise, starting top left), normal and inverse border
// 2 for scrollbar background / scrollbar foreground
// square length is determined by height of image, so width has to be (2+2*8+2)*h = 20*h
// icons:
// arrow up, arrow down, unchecked box, checked box

class widget
{
protected:
	std::string name;
	vector2i pos, size;
	std::string text;
	widget* parent;
	std::string background_image_name;
	image* background; // if this is != 0, the image is rendered in the background, centered
	const texture* background_tex; // drawn as tiles if != 0 and no image defined
	bool enabled;
	std::vector<std::unique_ptr<widget>> children;
	int retval;
	bool closeme;	// is set to true by close(), stops run() the next turn
	mutable bool redrawme;	// is set to true by redraw(), cleared each time draw() is called

	widget();
	widget(const widget& );
	widget& operator= (const widget& );
	static void draw_frame(int x, int y, int w, int h, bool out);
	static void draw_rect(int x, int y, int w, int h, bool out);
	static void draw_line(int x1, int y1, int x2, int y2);
	virtual void draw_area(int x, int y, int w, int h, bool out) const;
	virtual void draw_area_col(int x, int y, int w, int h, bool out, color c) const;

	static objcachet<class image>* myimagecache;
	static objcachet<class image>& imagecache();	// checks for existance
public:
	class theme {
		theme() = delete;
		theme(const theme& ) = delete;
		theme& operator= (const theme& ) = delete;
	public:
		std::unique_ptr<texture> backg;
		std::unique_ptr<texture> skbackg;
		std::unique_ptr<texture> frame[8];
		std::unique_ptr<texture> frameinv[8];
		std::unique_ptr<texture> icons[4];
		std::unique_ptr<texture> sbarbackg;
		std::unique_ptr<texture> sbarsurf;
		const font* myfont;
		color textcol, textselectcol, textdisabledcol;
		int frame_size() const;
		int icon_size() const;
		theme(const char* elements_filename, const char* icons_filename, const font* fnt,
			color tc, color tsc, color tdc);
	};

	/* fixme wtf?! delete that!
	struct key_event {
		const widget* source;
		const SDL_keysym ks;
		key_event(const widget* _source, const SDL_keysym& _ks)
			: source(_source), ks(_ks) {}
	};
	*/
	struct mouse_click_event {
		const widget* source;
		const int mx, my, mb;
		mouse_click_event(const widget* _source, int _mx, int _my, int _mb)
			: source(_source), mx(_mx), my(_my), mb(_mb) {}
	};
	struct mouse_release_event {
		const widget* source;
		mouse_release_event(const widget* _source)
			: source(_source) {}
	};
	struct mouse_drag_event {
		const widget* source;
		const int mx, my, rx, ry, mb;
		mouse_drag_event(const widget* _source, int _mx, int _my, int _rx, int _ry, int _mb)
			: source(_source), mx(_mx), my(_my), rx(_rx), ry(_ry), mb(_mb) {}
	};
	struct mouse_scroll_event {
		const widget* source;
		const int wd;
		mouse_scroll_event(const widget* _source, int _wd)
			: source(_source), wd(_wd) {}
	};

protected:
	static std::unique_ptr<widget::theme> globaltheme;
	static widget* focussed;	// which widget has the focus
	static widget* mouseover;	// which widget the mouse is over

	static int oldmx, oldmy, oldmb;	// used for input calculation

public:
	// Note! call this once before using images!
	static void set_image_cache(objcachet<class image>* imagecache);

	static std::string text_ok, text_cancel;

	static void set_theme(std::unique_ptr<theme> t) { globaltheme = std::move(t); }
	static const theme* get_theme() { return globaltheme.get(); }
	static std::unique_ptr<theme> replace_theme(std::unique_ptr<theme> t);
	widget(int x, int y, int w, int h, std::string  text_, widget* parent_ = nullptr, const std::string& backgrimg = std::string());
	widget(xml_elem&, widget* parent = nullptr);
	virtual ~widget();
	template<typename T> T& add_child(std::unique_ptr<T>&& w) {
		auto& ref = *w;
		w->set_parent(this);
		w->move_pos(pos);
		children.push_back(std::move(w));
		return ref;
	}
	/** same as add_child, but place new child near last child.
	    Give distance to last child and direction 0-3 (above, right, below, left)
	    A distance < 0 means use theme border width * -distance (default children distance) */
	virtual void add_child_near_last_child_generic(std::unique_ptr<widget>&& , int distance = -2, unsigned direction = 2);
	template<typename T> T& add_child_near_last_child(std::unique_ptr<T>&& w, int distance = -2, unsigned direction = 2) {
		auto& ref = *w;
		add_child_near_last_child_generic(std::unique_ptr<widget>(std::move(w)), distance, direction);
		return ref;
	}
	widget* get_child(const std::string&, bool recursive = true);
	const std::string& get_name() const { return name; }
	///> recompute size so that window embraces all children exactly.
	virtual void clip_to_children_area();
	virtual void remove_child(widget* w);
	virtual void remove_children();
	virtual bool is_mouse_over(int mx, int my) const;
	virtual bool is_mouse_over(const vector2i& m) const { return is_mouse_over(m.x, m.y); }
	virtual void draw() const;
	virtual bool compute_focus(int mx, int my);
	virtual bool compute_focus(const vector2i& m) { return compute_focus(m.x, m.y); }
	virtual bool compute_mouseover(int mx, int my);
	virtual bool compute_mouseover(const vector2i& m) { return compute_mouseover(m.x, m.y); }
	virtual vector2i get_pos() const { return pos; }
	virtual void set_pos(const vector2i& p) { move_pos(p - pos); }
	virtual void move_pos(const vector2i& p);
	virtual void align(int h, int v);	// give <0,0,>0 for left,center,right
	virtual vector2i get_size() const { return size; }
	virtual void set_size(const vector2i& s) { size = s; }
	virtual widget* get_parent() const { return parent; }
	virtual void set_parent(widget* w) { parent = w; }
	virtual std::string get_text() const { return text; }
	virtual void set_text(const std::string& s) { text = s; }
	virtual const image* get_background() const { return background; }
	virtual const texture* get_background_tex() const { return background_tex; }
	//Note! such a function is a bad idea, as image is not unref'd then at the cache!
	//virtual void set_background(const image* b) { background = b; background_tex = 0; }
	virtual void set_background(const texture* t) { background_tex = t; background = nullptr; }
	virtual void set_return_value(int rv) { retval = rv; }
	virtual int get_return_value() const { return retval; }
	virtual bool was_closed() const { return closeme; }
	virtual bool is_enabled() const;
	virtual void enable();
	virtual void disable();
	virtual void redraw();

	// called for every key in queue
	virtual void on_key(key_code , key_mod );
	virtual void on_text(const std::string& );
	// called on mouse button down
	virtual void on_click(vector2i position, mouse_button btn) {}
	// called on mouse wheel action, mb is 1 for up, 2 for down
	virtual void on_wheel(input_action wd);
	// called on mouse button up
	virtual void on_release() {}
	// called on mouse move while button is down
	virtual void on_drag(vector2i position, vector2i motion, mouse_button_state btnstate) {}

	// run() always returns 1    - fixme: make own widget classes for them?
	static std::unique_ptr<widget> create_dialogue_ok(widget* parent_, const std::string& title, const std::string& text = "", int w = 0, int h = 0);
	std::unique_ptr<widget> create_dialogue_ok(const std::string& title, const std::string& text = "", int w = 0, int h = 0) { return create_dialogue_ok(this, title, text, w, h); }
	// run() returns 1 for ok, 0 for cancel
	static std::unique_ptr<widget> create_dialogue_ok_cancel(widget* parent_, const std::string& title, const std::string& text = "", int w = 0, int h = 0);
	std::unique_ptr<widget> create_dialogue_ok_cancel(const std::string& title, const std::string& text = "") { return create_dialogue_ok_cancel(this, title, text); }

	// show & exec. widget, automatically disable widgets below
	// run() runs for "time" milliseconds (or forever if time == 0), then returns
	// if do_stacking is false only this widget is drawn, but none of its parents
	static int run(widget& w, unsigned timeout = 0, bool do_stacking = true, widget* focussed_at_begin = nullptr);
	static bool handle_key_event(widget& w, const input_event_handler::key_data& k);
	static bool handle_mouse_button_event(widget& w, const input_event_handler::mouse_click_data& m);
	static bool handle_mouse_motion_event(widget& w, const input_event_handler::mouse_motion_data& m);
	static bool handle_mouse_wheel_event(widget& w, const input_event_handler::mouse_wheel_data& m);
	static bool handle_text_input_event(widget& w, const std::string& t);

	virtual void close(int val);	// close this widget (stops run() on next turn, returns val)
	virtual void open();	// "open" this widget (reverts what close() did)

	static std::vector<widget*> widgets;	// stack of dialogues, topmost is back
	static void ref_all_backgrounds();	// for all stacked widgets, ref backgrounds
	static void unref_all_backgrounds();	// for all stacked widgets, unref backgrounds
};

class widget_text : public widget
{
protected:
	bool sunken;

	widget_text();
	widget_text(const widget_text& );
	widget_text& operator= (const widget_text& );
public:
	widget_text(int x, int y, int w, int h, const std::string& text_, widget* parent_ = nullptr, bool sunken_ = false)
		: widget(x, y, w, h, text_, parent_), sunken(sunken_) {}
	widget_text(xml_elem& elem, widget* _parent = nullptr);
	void draw() const override;
	virtual void set_text_and_resize(const std::string& s);
};

class widget_checkbox : public widget
{
protected:
	bool checked;

	widget_checkbox();
	widget_checkbox(const widget_checkbox& );
	widget_checkbox& operator= (const widget_checkbox& );
public:
	widget_checkbox(int x, int y, int w, int h, bool checked_, const std::string& text_, widget* parent_ = nullptr)
		: widget(x, y, w, h, text_, parent_), checked(checked_) {}
	widget_checkbox(xml_elem& elem, widget* _parent = nullptr);
	void draw() const override;
	void on_click(vector2i position, mouse_button btn) override;
	bool is_checked() const { return checked; }
	virtual void on_change() {}
};

class widget_button : public widget
{
protected:
	bool pressed;

	widget_button();
	widget_button(const widget_button& );
	widget_button& operator= (const widget_button& );
public:
	widget_button(int x, int y, int w, int h, const std::string& text_,
		      widget* parent_ = nullptr, const std::string& backgrimg = std::string()) : widget(x, y, w, h, text_, parent_, backgrimg), pressed(false) {}
	widget_button(xml_elem& elem, widget* _parent = nullptr);
	void draw() const override;
	void on_click(vector2i position, mouse_button btn) override;
	void on_release() override;
	bool is_pressed() const { return pressed; }
	virtual void on_change() {}
};

/// A class that can be used to call any function with any parameters, i.e. also a member function via lambda that calls the member, when the object
/// is the first parameter. Can also be used to set values.
template<typename ...Types>
class widget_caller_button : public widget_button
{
	std::function<void(Types...)> func;
	std::tuple<Types...> args;
public:
	widget_caller_button(int x, int y, int w, int h, const std::string& text, widget* parent,
			      std::function<void(Types...)> func_, Types ... args_) : widget_button(x, y, w, h, text, parent), func(func_), args(args_ ...) {}
	widget_caller_button(std::function<void(Types...)> func_, Types ... args_) : widget_button(0, 0, 0, 0, std::string{}, nullptr), func(func_), args(args_ ...) {}
	void on_release() override { widget_button::on_release(); std::apply(func, args); }
};

template<typename ...Types>
class widget_caller_checkbox : public widget_checkbox
{
	std::function<void(Types...)> func;
	std::tuple<Types...> args;
public:
	widget_caller_checkbox(int x, int y, int w, int h, const std::string& text, widget* parent, bool checked,
			       std::function<void(Types...)> func_, Types ... args_) : widget_checkbox(x, y, w, h, checked, text, parent), func(func_), args(args_ ...) {}
	void on_change() override { widget_checkbox::on_change(); std::apply(func, args); }
};

class widget_menu : public widget
{
protected:
	bool horizontal;
	int entryw, entryh;
	int entryspacing;

	widget_menu() = default;
	widget_menu(const widget_menu& );
	widget_menu& operator= (const widget_menu& );

	// do not use add_child here!

public:
	widget_menu(int x, int y, int w, int h, const std::string& text_, bool horizontal_ = false,
		    widget* parent_ = nullptr);
	widget_menu(xml_elem& elem, widget* _parent = nullptr);
	void set_entry_spacing(int spc) { entryspacing = spc; }
	void adjust_buttons(unsigned totalsize);	// width or height
	widget_button* add_entry(const std::string& s, std::unique_ptr<widget_button> wb); // wb's text is always set to s
	int get_selected() const;
	void draw() const override;
};

class widget_scrollbar : public widget
{
protected:
	unsigned scrollbarpixelpos;	// current pixel position
	unsigned scrollbarpos;		// current position
	unsigned scrollbarmaxpos;	// maximum number of positions

	unsigned get_max_scrollbarsize() const;	// total height of bar in pixels
	unsigned get_scrollbarsize() const;	// height of slider bar in pixels
	void compute_scrollbarpixelpos();	// recompute value from pos values

	void draw_area(int x, int y, int w, int h, bool out) const override;

	widget_scrollbar();
	widget_scrollbar(const widget_scrollbar& );
	widget_scrollbar& operator= (const widget_scrollbar& );
public:
	widget_scrollbar(int x, int y, int w, int h, widget* parent_ = nullptr);
	widget_scrollbar(xml_elem& elem, widget* _parent = nullptr);
	void set_nr_of_positions(unsigned s);
	unsigned get_current_position() const;
	void set_current_position(unsigned p);
	void draw() const override;
	void on_click(vector2i position, mouse_button btn) override;
	void on_drag(vector2i position, vector2i motion, mouse_button_state btnstate) override;
	void on_wheel(input_action wd) override;
	virtual void on_scroll() {}
};

class widget_list : public widget
{
protected:
	std::vector<std::string> entries;
	unsigned listpos;
	int selected;
	widget_scrollbar* myscrollbar;	// stored also as child
	int columnwidth;	// in pixels, translates tabs to column switches, set -1 for no columns (default)

	widget_list();
	widget_list(const widget_list& );
	widget_list& operator= (const widget_list& );
public:
	widget_list(int x, int y, int w, int h, widget* parent_ = nullptr);
	widget_list(xml_elem& elem, widget* _parent = nullptr);
	void delete_entry(unsigned n);
	void insert_entry(unsigned n, const std::string& s);
	void append_entry(const std::string& s);
	void set_entry(unsigned n, const std::string& s);
	void sort_entries();
	void make_entries_unique();
	std::string get_entry(unsigned n) const;
	unsigned get_listsize() const;
	int get_selected() const;
	void set_selected(unsigned n);
	std::string get_selected_entry() const;
	unsigned get_nr_of_visible_entries() const;
	void clear();
	void draw() const override;
	void on_click(vector2i position, mouse_button btn) override;
	void on_drag(vector2i position, vector2i motion, mouse_button_state btnstate) override;
	void on_wheel(input_action wd) override;
	virtual void on_sel_change() {}
	void set_column_width(int cw);
};

class widget_edit : public widget
{
protected:
	unsigned cursorpos;
	// move cursor. will handle multibyte strings correctly. Returns new position after movement.
	unsigned cursor_left() const;
	unsigned cursor_right() const;

	widget_edit();
	widget_edit(const widget_edit& );
	widget_edit& operator= (const widget_edit& );
public:
	widget_edit(int x, int y, int w, int h, const std::string& text_, widget* parent_ = nullptr)
		: widget(x, y, w, h, text_, parent_), cursorpos(text_.length()) {}
	widget_edit(xml_elem& elem, widget* _parent = nullptr);
	void set_text(const std::string& s) override { widget::set_text(s); cursorpos = s.length(); }
	void draw() const override;
	void on_key(key_code , key_mod ) override;
	void on_text(const std::string& ) override;
	virtual void on_enter() {}	// run on pressed ENTER-key
	virtual void on_change() {}
};

class widget_fileselector : public widget
{
protected:
	widget_list* current_dir;
	widget_edit* current_filename;
	widget_text* current_path;

	void read_current_dir();
	unsigned nr_dirs, nr_files;

	struct filelist : public widget_list
	{
		void on_click(vector2i position, mouse_button btn) override {
			widget_list::on_click(position, btn);
			dynamic_cast<widget_fileselector*>(parent)->listclick();
		}
		filelist(int x, int y, int w, int h) : widget_list(x, y, w, h) {}
		~filelist() override = default;
	};

	widget_fileselector();
	widget_fileselector(const widget_fileselector& );
	widget_fileselector& operator= (const widget_fileselector& );
public:
	widget_fileselector(int x, int y, int w, int h, const std::string& text_, widget* parent_ = nullptr);
	widget_fileselector(xml_elem& elem, widget* _parent = nullptr);
	std::string get_filename() const { return current_path->get_text() + current_filename->get_text(); }
	void listclick();
};

class widget_3dview : public widget
{
protected:
	std::unique_ptr<model> mdl;
	color backgrcol;
	double z_angle;
	double x_angle;
	vector3f translation;	// translation.z is neg. distance to viewer
	vector4f lightdir;
	color lightcol;

	void on_wheel(input_action wd) override;
	void on_drag(vector2i position, vector2i motion, mouse_button_state btnstate) override;

	widget_3dview();
	widget_3dview(const widget_3dview& );
	widget_3dview& operator= (const widget_3dview& );
public:
	widget_3dview(int x, int y, int w, int h, std::unique_ptr<model> mdl, color bgcol, widget* parent_ = nullptr);
	widget_3dview(xml_elem& elem, widget* _parent = nullptr);
	void draw() const override;
	void set_model(std::unique_ptr<model> mdl_);
	model* get_model() { return mdl.get(); }
	// widget will handle orientation itself. also user input for changing that...
	// void set_orientation() / set_translation() <- later.
	void set_light_dir(const vector4f& ld) { lightdir = ld; }
	void set_light_color(color lc) { lightcol = lc; }
};

class widget_slider : public widget
{
protected:
	int minvalue;
	int maxvalue;
	int currvalue;
	int descrstep;

	widget_slider();
	widget_slider(const widget_slider& );
	widget_slider& operator= (const widget_slider& );
public:
	/// Note: height is for full widget, so give enough space for descriptions + text + slider bar
	widget_slider(int x, int y, int w, int h, const std::string& text_,
		      int minv, int maxv, int currv, int descrstep,
		      widget* parent_ = nullptr);
	widget_slider(xml_elem& elem, widget* _parent = nullptr);
	void draw() const override;
	void on_key(key_code , key_mod ) override;
	void on_click(vector2i position, mouse_button btn) override;
	void on_drag(vector2i position, vector2i motion, mouse_button_state btnstate) override;
	virtual void set_values(int minv, int maxv, int currv, int descrstep);
	virtual void on_change() {}
	virtual int get_min_value() const { return minvalue; }
	virtual int get_curr_value() const { return currvalue; }
	virtual int get_max_value() const { return maxvalue; }
};

#endif
