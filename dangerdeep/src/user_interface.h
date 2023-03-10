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

// user interface for controlling a sea_object
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#pragma once

#include "caustics.h"
#include "coastmap.h"
#include "color.h"
#include "geoclipmap.h"
#include "input_event_handler.h"
#include "sea_object.h"
#include "user_display.h"
#include "user_popup.h"

#include <list>
#include <map>
#include <vector>

class game;
class water;

///\defgroup interfaces In-game user interfaces
///\brief Base class for a user interface for playing the game.
///\ingroup interfaces
class user_interface : public input_event_handler
{
    user_interface();

  public:
    enum color_mode
    {
        day_color_mode,
        night_color_mode
    };

  protected:
    game* mygame; // pointer to game object that is displayed

    bool pause;
    bool abort_request; // by user (back to game menu)
    unsigned time_scale;

    // command panel, to submarine_interface!
    // display texts above panel, fading out, no widget! fixme
    bool panel_visible;
    std::unique_ptr<class widget> panel;
    class widget_text* panel_valuetexts[6];

    // screen selector menu
    std::unique_ptr<class widget> screen_selector;
    bool screen_selector_visible;

    // music playlist
    std::unique_ptr<class widget> music_playlist;
    bool playlist_visible;
    class widget_checkbox* playlist_repeat_checkbox;
    class widget_checkbox* playlist_shuffle_checkbox;
    class widget_checkbox* playlist_mute_checkbox;

    // main menu
    std::unique_ptr<class widget> main_menu;
    bool main_menu_visible;

    /// holds the last n messages. They're displayed above the panel and fading
    /// out over time.
    std::list<std::pair<double, std::string>> messages;

    // used in various screens
    angle bearing;
    angle elevation;          // -90...90 deg (look down ... up)
    bool bearing_is_relative; // bearing means angle relative to course or
                              // absolute? (default = true)

    // which display is active
    unsigned current_display;

    // fixme replace the above with: THE ONE AND ONLY DATA UI SHOULD HAVE
    std::vector<std::unique_ptr<user_display>> displays;
    // fixme must be std::vector<std::shared_ptr<user_display>> displays to
    // register them as input event handlers!

    // which popup is shown (0 = none)
    unsigned current_popup;

    // possible popups
    std::vector<std::unique_ptr<user_popup>> popups;

    // environmental data
    std::unique_ptr<class sky> mysky; // the one and only sky
    caustics mycaustics;              //	caustic map
    coastmap mycoastmap; // this may get moved to game.h, yet it is used for
                         // display only, that's why it is here
    std::unique_ptr<geoclipmap> mygeoclipmap; // terrain rendering instance

    // is display in day mode (or night/redlight mode)?
    bool daymode;

    // weather graphics
    std::vector<std::unique_ptr<class texture>>
        raintex; // images (animation) of rain drops
    std::vector<std::unique_ptr<class texture>>
        snowtex; // images (animation) of snow flakes

    // free view mode
    //	float freeviewsideang, freeviewupang;	// global spectators viewing
    // angles 	vector3 freeviewpos;

    user_interface& operator=(const user_interface& other);
    user_interface(const user_interface& other);
    user_interface(game& gm);

    // MUST be called after constructing an user_interface object (or one of its
    // heirs). this function waits for completion of threads used to construct
    // the ui object.
    void finish_construction();

    //	inline virtual sea_object* get_player() const { return player_object; }

    //	void draw_clock(game& gm, int x, int y, unsigned wh, double t,
    //	        const string& text) const;

    // adjusts "current_popup" if not set to allowed popup
    void set_allowed_popup();

    // set "current_display" only via this function, so that checks can be
    // performed autom.
    void set_current_display(unsigned curdis);

    virtual void playlist_mode_changed();
    virtual void playlist_mute();

    virtual void show_screen_selector();
    virtual void toggle_popup();
    virtual void show_playlist();

  public:
    ~user_interface() override;

    // display (const) and input handling
    virtual void display() const;

    // set global time for display (needed for water/sky animation)
    virtual void set_time(double tm);

    // process common events (common keys, mouse input to panel)
    bool handle_key_event(const key_data&) override;
    bool handle_mouse_button_event(const mouse_click_data&) override;
    bool handle_mouse_motion_event(const mouse_motion_data&) override;
    bool handle_mouse_wheel_event(const mouse_wheel_data&) override;

    // create ui matching to player type (requested from game)
    static std::shared_ptr<user_interface> create(game& gm);

    [[nodiscard]] const sky& get_sky() const { return *(mysky.get()); }
    [[nodiscard]] const caustics& get_caustics() const { return mycaustics; }
    [[nodiscard]] const water& get_water() const;
    [[nodiscard]] const coastmap& get_coastmap() const { return mycoastmap; }

    // helper functions

    [[nodiscard]] virtual angle get_relative_bearing() const;
    [[nodiscard]] virtual angle get_absolute_bearing() const;
    [[nodiscard]] virtual angle get_elevation() const;
    // add angles to change bearing/elevation
    virtual void add_bearing(angle a);
    virtual void add_elevation(angle a);

    // 2d drawing must be on for this
    void draw_infopanel(bool onlytexts = false) const;

    // render red triangle for target in view. give viewport coordinates.
    virtual void
    show_target(double x, double y, double w, double h, const vector3& viewpos);

    // 3d drawing functions
    virtual void draw_terrain(
        const vector3& viewpos,
        angle dir,
        double max_view_dist,
        bool mirrored,
        int above_water) const;

    virtual void draw_weather_effects() const;

    virtual void toggle_pause();
    [[nodiscard]] virtual bool paused() const { return pause; }
    [[nodiscard]] virtual unsigned time_scaling() const { return time_scale; }
    virtual void add_message(const std::string& s);
    virtual bool time_scale_up(); // returns true on success
    virtual bool time_scale_down();
    //	virtual void record_sunk_ship ( const class ship* so );
    /** This method creates a message about the rudder state. */
    virtual void play_sound_effect(
        const std::string& se,
        const vector3& noise_source /*, bool loop = false*/) const;
    virtual void pause_all_sound() const;
    virtual void resume_all_sound() const;

    // get current game of user_interface
    virtual game& get_game() { return *mygame; }
    [[nodiscard]] virtual const game& get_game() const { return *mygame; }

    [[nodiscard]] bool abort_requested() const { return abort_request; }
    void request_abort(bool abrt = true) { abort_request = abrt; }

    void switch_geo_wire()
    {
        mygeoclipmap->wireframe = !mygeoclipmap->wireframe;
    }
};
