// user interface for controlling a sea_object
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <list>
#include <vector>
#include <map>
using namespace std;
#include "sea_object.h"
#include "global_data.h"
#include "color.h"

#define WAVERND 64        	// must be 2^x, length/width of ramdom array
#define WAVESIZE 10		// meters from wave to wave
#define WAVEDEPTH 20		// nr of waves drawn in z direction
#define WAVETIDE 3.0      	// amplitude of waves in meters
#define WAVETIDECYCLETIME 2.0	// time for full cycle of wave

#define MAPGRIDSIZE 1000	// meters

class user_display;
class logbook_display;
class ships_sunk_display;

class user_interface
{
public:
	enum color_mode { day_color_mode, night_color_mode };
	enum sound_effect { se_submarine_torpedo_launch, se_torpedo_detonation };

protected:
	bool quit;		// whishes user to quit?
	bool pause;
	unsigned time_scale;
	sea_object* player_object;
	list<string> panel_texts;
	unsigned panel_height;
	bool panel_visible;

	// used in various screens
	angle bearing;
	unsigned viewmode;
	sea_object* target;

	logbook_display* captains_logbook;
	ships_sunk_display* ships_sunk_disp;

	// periscope
	bool zoom_scope;	// use 6x instead 1.5 fixme implement

	// map
	float mapzoom;	// factor pixel/meter
	vector2 mapclick;
	double mapclickdist;

	// free view mode
	float viewsideang, viewupang;	// global spectators viewing angles
	vector3 viewpos;

	user_interface();
	user_interface& operator= (const user_interface& other);
	user_interface(const user_interface& other);
	user_interface(sea_object* player);

	unsigned wavedisplaylists;		// # of first display list
static vector<unsigned char> waveheights;	// random square array
static vector<float> sinvec;			// sin table (256 entries)
	void init(void);
	void deinit(void);
static void init_water_data(void);
	static unsigned char get_waterheight(int x, int y) { return waveheights[(x&(WAVERND-1))+(y&(WAVERND-1))*WAVERND]; }
	static float get_waterheight(float x_, float y_, int wavephase); // bilinear sampling

	inline virtual sea_object* get_player(void) const { return player_object; }
	virtual bool keyboard_common(int keycode, class system& sys, class game& gm) = 0;

	static texture* torptex(unsigned type);

	// color funtions.
	virtual void set_display_color ( color_mode mode ) const;
	virtual void set_display_color ( const class game& gm ) const;
	
	// 2d drawing must be turned on for them
	void draw_infopanel(class system& sys, class game& gm) const;
	void draw_gauge(class system& sys, class game& gm, unsigned nr, int x, int y, unsigned wh, angle a,
		const string& text, angle a2) const;
	void draw_gauge(class system& sys, class game& gm, unsigned nr, int x, int y, unsigned wh, angle a,
		const string& text) const {
			draw_gauge(sys, gm, nr, x, y, wh, a, text, a);
	}
	// draws turnable switch. parameters: pos, first index and number of descriptions,
	// selected description, extra description text number and title text nr.
	void draw_turnswitch(class system& sys, class game& gm, int x, int y,
		unsigned firstdescr, unsigned nrdescr, unsigned selected, unsigned extradescr, unsigned title) const;
	// Matching input function, give pos 0-255,0-255.
	unsigned turnswitch_input(int x, int y, unsigned nrdescr) const;
	
	void draw_manometer_gauge ( class system& sys, class game& gm, unsigned nr,
		int x, int y, unsigned wh, float value, const string& text ) const;
	void draw_clock(class system& sys, class game& gm, int x, int y, unsigned wh, double t,
	        const string& text) const;
	void draw_vessel_symbol(class system& sys, const vector2& offset, 
                                sea_object* so, color c);
	void draw_trail(sea_object* so, const vector2& offset);
	virtual void draw_pings(class game& gm, const vector2& offset);
	virtual void draw_sound_contact(class game& gm, const sea_object* player,
		double max_view_dist);
	virtual void draw_visual_contacts(class system& sys, class game& gm,
		const sea_object* player, const vector2& offset);
	virtual void draw_square_mark ( class system& sys, class game& gm,
		const vector2& mark_pos, const vector2& offset, const color& c );

	// Display functions for screens.
	virtual void display_gauges(class system& sys, class game& gm);
	virtual void display_bridge(class system& sys, class game& gm);
	virtual void display_map(class system& sys, class game& gm);
	virtual void display_logbook(class system& sys, class game& gm);
	virtual void display_successes(class system& sys, class game& gm);
	virtual void display_freeview(class system& sys, class game& gm);
	virtual void display_glasses(class system& sys, class game& gm);
	virtual void display_damagestatus(class system& sys, class game& gm) = 0;

	virtual sound* get_sound_effect ( sound_effect se ) const;

public:	
	virtual ~user_interface();
	virtual void display(class system& sys, class game& gm) = 0;

	// 3d drawing functions
	virtual void draw_water(const vector3& viewpos, angle dir, unsigned wavephase, double max_view_dist) const;
	virtual void draw_view(class system& sys, class game& gm, const vector3& viewpos,
		angle dir, unsigned withplayer, bool withunderwaterweapons); // give 0-2 for "withplayer": draw 0 - nothing, 1 - sub/ship, 2 - bridge
	virtual bool user_quits(void) const { return quit; }
	virtual bool paused(void) const { return pause; }
	virtual unsigned time_scaling(void) const { return time_scale; }
	virtual void add_message(const string& s);
	virtual void add_captains_log_entry ( class game& gm, const string& s);
	virtual bool time_scale_up(void);	// returns true on success
	virtual bool time_scale_down(void);
	virtual void record_sunk_ship ( const ship* so );
	/** This method creates a message about the rudder state. */
	virtual void add_rudder_message();
	virtual void play_sound_effect ( sound_effect se, double volume = 1.0f ) const;
	virtual void play_sound_effect_distance ( sound_effect se, double distance ) const;
};

#endif
