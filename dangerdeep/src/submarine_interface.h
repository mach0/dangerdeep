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

// user interface for controlling a submarine
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#pragma once

#include "submarine.h"

#include <list>
#include <vector>
//#include "datadirs.h"
#include "color.h"
#include "user_interface.h"

// pop-ups:
// can be used in various screens, so they should be stored in
// submarine_interface each screen states which popups are allowed when they're
// displayed and where to place the popus. input: every mouse event inside the
// popup rectangle goes to the popup. only clicks outside go to the screen.
// should popups receive keyboard messages?
// if yes then let the popup handle them, and pass unhandled keys to the main
// screen. popup-concept could also be declared in user_interface

///\brief User interface implementation for control of submarines.
/** This class handles all the input and output to and from the player and the
   game if the user plays a commander of a submarine. */
///\ingroup interfaces
class submarine_interface : public user_interface
{
  public:
    // the indices for the displays
    enum
    {
        display_mode_gauges,
        display_mode_valves,
        display_mode_periscope,
        display_mode_uzo,
        display_mode_bridge,
        display_mode_map,
        display_mode_torpedoroom,
        display_mode_damagestatus,
        display_mode_captainscabin,
        display_mode_logbook,
        display_mode_soldbuch,
        display_mode_successes,
        display_mode_sonar,
        display_mode_freeview,
        display_mode_tdc,
        display_mode_tdc2,
        display_mode_torpsetup,
        display_mode_recogmanual,
        nr_of_displays
    };

    enum
    {
        popup_mode_control,
        popup_mode_tdc,
        popup_mode_ecard,
        popup_mode_recogmanual,
        nr_of_popups
    };

  private:
    submarine_interface()        = delete;
    submarine_interface& operator=(const submarine_interface& other) = delete;
    submarine_interface(const submarine_interface& other)            = delete;

  protected:
    unsigned selected_tube;
    std::unique_ptr<class torpedo_camera_display> torpedo_cam_view;
    mutable unsigned torpedo_cam_track_nr;

    /// overloaded from user_interface, for forced screen switching
    void set_time(double tm) override;

  public:
    // public, because the functions could be called by heirs of user_display,
    // and should be called only from there.
    void goto_gauges();
    void goto_periscope();
    void goto_UZO();
    void goto_bridge();
    void goto_map();
    void goto_torpedomanagement();
    void goto_damagecontrol();
    void goto_captainscabin();
    void goto_logbook();
    void goto_soldbuch();
    void goto_successes();
    void goto_sonar();
    void goto_freeview();
    void goto_TDC();
    void goto_TDC2();
    void goto_torpedosettings();
    void goto_recogmanual();
    void goto_valves();

    submarine_interface(class game& gm);
    ~submarine_interface() override;

    void fire_tube(submarine* player, int nr);

    void display() const override;
    bool handle_key_event(const key_data&) override;
    bool handle_mouse_button_event(const mouse_click_data&) override;
    virtual unsigned get_selected_tube() const { return selected_tube; }
    virtual void select_tube(unsigned nr) { selected_tube = nr; }
};
