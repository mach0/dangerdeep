// OpenGL based widgets
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifndef WIDGET_H
#define WIDGET_H

#include <list>
#include <string>
using namespace std;
#include "system.h"
#include "color.h"
#include "font.h"
#include "vector2.h"

// fixme: make yes/no, ok, yes/no/cancel dialogue
// when a dialogue opens another one, both should be drawn, and the parent
// should pass draw()/process_input() calls to its child
// make a special flag: widget* wait_for?
// process_input(){if (wait_for)wait_for->process_input();else ...old...;} ?
// ein widget.close fehlt. close:= parent.remove(this), wenn parent==0 dann globale liste nach this
// durchsuchen. run() l�uft dann bis globale liste leer ist.

// theme files:
// two images, one for elements, one for icons.
// each image is one row of square elements.
// elements:
// 2 for background/sunken background
// 2*8 for borders (clockwise, starting top left), normal and inverse border
// square length is determined by height of image, so width has to be (2+2*8)*h = 18*h
// icons:
// arrow up, arrow down, unchecked box, checked box

class widget
{
protected:
	enum return_values { NO_RETURN=-1, OK=0 };
	vector2i pos, size;
	string text;
	widget* parent;
	bool enabled;
	list<widget*> children;
	int retval;	// run() is stopped whenever this becomes != NO_RETURN
	
	widget();
	widget(const widget& );
	widget& operator= (const widget& );
	static void draw_frame(int x, int y, int w, int h, bool out);
	static void draw_rect(int x, int y, int w, int h, bool out);
	static void draw_area(int x, int y, int w, int h, bool out);
public:
	class theme {
		theme();
		theme(const theme& );
		theme& operator= (const theme& );
	public:
		class texture *backg, *skbackg, *frame[8], *frameinv[8], *icons[4];
		const font* myfont;
		color textcol, textselectcol;
		int frame_size(void) const;
		int icon_size(void) const;
		theme(const char* elements_filename, const char* icons_filename, const font* fnt,
			color tc, color tsc);
		~theme();
	};

protected:
	static class widget::theme* globaltheme;
	static widget* focussed;	// which widget has the focus
	
	static int oldmx, oldmy, oldmb;	// used for input calculation

public:	
	static void set_theme(class theme* t);
	widget(int x, int y, int w, int h, const string& text_, widget* parent_ = 0);
	virtual ~widget();
	virtual void add_child(widget* w);
	virtual void remove_child(widget* w);
	virtual bool is_mouse_over(void) const;
	virtual void draw(void) const;
	virtual bool compute_focus(void);
	virtual vector2i get_pos(void) const { return pos; }
	virtual void set_pos(const vector2i& p) { move_pos(p - pos); }
	virtual void move_pos(const vector2i& p);
	virtual vector2i get_size(void) const { return size; }
	virtual void set_size(const vector2i& s) { size = s; }
	virtual widget* get_parent(void) const { return parent; }
	virtual void set_parent(widget* w) { parent = w; }
	virtual string get_text(void) const { return text; }
	virtual void set_text(const string& s) { text = s; }
	virtual bool is_enabled(void) const;
	virtual void enable(void);
	virtual void disable(void);
	virtual void on_char(void);	// called for every key in queue
	virtual void on_click(void) {};	// called on mouse button down
	virtual void on_release(void) {};	// called on mouse button up
	virtual void on_drag(void) {};	// called on mouse move while button is down
	virtual void prepare_input(void);	// call once before process_input
	virtual void process_input(void);	// determine type of input, fetch it to on_* functions

	static widget* create_dialogue_ok(const string& text);		// run() always returns 1
	static widget* create_dialogue_ok_cancel(const string& text);	// run() returns 1 for ok, 0 for cancel

	virtual int run(void);	// show & exec. widget, automatically disable widgets below
	virtual void close(int val) { retval = val; }	// close this widget (stops run() on next turn, returns val)
	
	static list<widget*> widgets;	// stack of dialogues, topmost is back
};

class widget_text : public widget
{
protected:

	widget_text();
	widget_text(const widget_text& );
	widget_text& operator= (const widget_text& );
public:
	widget_text(int x, int y, int w, int h, const string& text_, widget* parent_ = 0)
		: widget(x, y, w, h, text_, parent_) {}
	~widget_text() {}
	void draw(void) const;
};

class widget_checkbox : public widget
{
protected:
	bool checked;

	widget_checkbox();
	widget_checkbox(const widget_checkbox& );
	widget_checkbox& operator= (const widget_checkbox& );
public:
	widget_checkbox(int x, int y, int w, int h, const string& text_, widget* parent_ = 0)
		: widget(x, y, w, h, text_, parent_) {}
	~widget_checkbox() {}
	void draw(void) const;
	void on_click(void);
	bool is_checked(void) const { return checked; }
	virtual void on_change(void) {}
};

class widget_button : public widget
{
protected:
	bool pressed;

	widget_button();
	widget_button(const widget_button& );
	widget_button& operator= (const widget_button& );
public:
	widget_button(int x, int y, int w, int h, const string& text_,
		widget* parent_ = 0) : widget(x, y, w, h, text_, parent_), pressed(false) {}
	~widget_button() {}
	void draw(void) const;
	void on_click(void);
	void on_release(void);
	bool is_pressed(void) const { return pressed; }
	virtual void on_change(void) {}
};

template<class Obj, class Func>
class widget_caller_button : public widget_button
{
	Obj* obj;
	Func func;
public:
	widget_caller_button(Obj* obj_, Func func_, int x = 0, int y = 0, int w = 0, int h = 0,
		const string& text = "", widget* parent = 0)
		: widget_button(x, y, w, h, text, parent), obj(obj_), func(func_) {}
	~widget_caller_button() {}
	void on_release(void) { widget_button::on_release(); (obj->*func)(); }
};

template<class Obj, class Func, class Arg>
class widget_caller_arg_button : public widget_button
{
	Obj* obj;
	Func func;
	Arg arg;
public:
	widget_caller_arg_button(Obj* obj_, Func func_, Arg arg_, int x = 0, int y = 0, int w = 0, int h = 0,
		const string& text = "", widget* parent = 0)
		: widget_button(x, y, w, h, text, parent), obj(obj_), func(func_), arg(arg_) {}
	~widget_caller_arg_button() {}
	void on_release(void) { widget_button::on_release(); (obj->*func)(arg); }
};

template<class Func, class Arg>
class widget_func_arg_button : public widget_button
{
	Func func;
	Arg arg;
public:
	widget_func_arg_button(Func func_, Arg arg_, int x = 0, int y = 0, int w = 0, int h = 0,
		const string& text = "", widget* parent = 0)
		: widget_button(x, y, w, h, text, parent), func(func_), arg(arg_) {}
	~widget_func_arg_button() {}
	void on_release(void) { widget_button::on_release(); func(arg); }
};

template<class Obj>
class widget_set_button : public widget_button
{
	Obj& obj;
	Obj value;
public:
	widget_set_button(Obj& obj_, const Obj& val, int x = 0, int y = 0, int w = 0, int h = 0,
		const string& text = "", widget* parent = 0)
		: widget_button(x, y, w, h, text, parent), obj(obj_), value(val) {}
	~widget_set_button() {}
	void on_release(void) { widget_button::on_release(); obj = value; }
};

class widget_menu : public widget
{
protected:
	bool horizontal;
	int entryw, entryh;

	widget_menu() {}
	widget_menu(const widget_menu& );
	widget_menu& operator= (const widget_menu& );
public:
	widget_menu(int x, int y, int w, int h, bool horizontal_ = false, widget* parent_ = 0)
		: widget(x, y, 0, 0, "", parent_), horizontal(horizontal_), entryw(w), entryh(h) {}
	widget_button* add_entry(const string& s, widget_button* wb = 0);
	int get_selected(void) const;
	~widget_menu() {};
	void draw(void) const;
};

class widget_scrollbar : public widget
{
protected:
	unsigned scrollbarpixelpos;	// current pixel position
	unsigned scrollbarpos;		// current position
	unsigned scrollbarmaxpos;	// maximum number of positions

	unsigned get_max_scrollbarsize(void) const;	// total height of bar in pixels
	unsigned get_scrollbarsize(void) const;	// height of slider bar in pixels
	void compute_scrollbarpixelpos(void);	// recompute value from pos values

	widget_scrollbar();
	widget_scrollbar(const widget_scrollbar& );
	widget_scrollbar& operator= (const widget_scrollbar& );
public:
	widget_scrollbar(int x, int y, int w, int h, widget* parent_ = 0);
	~widget_scrollbar();
	void set_nr_of_positions(unsigned s);
	unsigned get_current_position(void) const;
	void set_current_position(unsigned p);
	void draw(void) const;
	void on_click(void);
	void on_drag(void);
	virtual void on_scroll(void) {}
};

class widget_list : public widget
{
protected:
	list<string> entries;
	unsigned listpos;
	int selected;
	widget_scrollbar* myscrollbar;	// stored also as child

	widget_list();
	widget_list(const widget_list& );
	widget_list& operator= (const widget_list& );
public:
	widget_list(int x, int y, int w, int h, widget* parent_ = 0);
	~widget_list() {};
	void append_entry(const string& s);
	string get_entry(unsigned n) const;
	unsigned get_listsize(void) const;
	int get_selected(void) const;
	void set_selected(unsigned n);
	string get_selected_entry(void) const;
	unsigned get_nr_of_visible_entries(void) const;
	void clear(void);
	void draw(void) const;
	void on_click(void);
	void on_drag(void);
	virtual void on_sel_change(void) {}
};

class widget_edit : public widget
{
protected:
	unsigned cursorpos;

	widget_edit();
	widget_edit(const widget_edit& );
	widget_edit& operator= (const widget_edit& );
public:
	widget_edit(int x, int y, int w, int h, const string& text_, widget* parent_ = 0)
		: widget(x, y, w, h, text_, parent_), cursorpos(0) {}
	~widget_edit() {}
	void draw(void) const;
	void on_char(void);
	virtual void on_enter(void) {}	// run on pressed ENTER-key
	virtual void on_change(void) {}
};

#endif
