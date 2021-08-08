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

// user display: submarine's bridge
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sub_bridge_display.h"

#include "cfg.h"
#include "game.h"
#include "submarine.h"
#include "system_interface.h"
#include "user_interface.h"

namespace
{
enum element_type
{
    et_glasses = 0
};
}

void sub_bridge_display::pre_display() const { glClear(GL_DEPTH_BUFFER_BIT); }

freeview_display::projection_data
sub_bridge_display::get_projection_data(game& gm) const
{
    projection_data pd = freeview_display::get_projection_data(gm);
    if (element_for_id(et_glasses).is_visible())
    {
        pd.x     = 0;
        pd.y     = 0;
        pd.w     = SYS().get_res_x();
        pd.h     = SYS().get_res_x();
        pd.fov_x = 20.0;
    }
    return pd;
}

void sub_bridge_display::post_display() const { draw_elements(); }

sub_bridge_display::sub_bridge_display(user_interface& ui_) :
    freeview_display(ui_, "sub_bridge")
{
    auto* sub = dynamic_cast<submarine*>(ui_.get_game().get_player());
    add_pos   = sub->get_camera_position();

    aboard                = true;
    withunderwaterweapons = false;
    drawbridge            = true;
}

bool sub_bridge_display::handle_key_event(const key_data& k)
{
    if (k.down())
    {
        if (is_configured_key(key_command::TOGGLE_ZOOM_OF_VIEW, k))
        {
            auto& elem = element_for_id(et_glasses);
            elem.set_visible(!elem.is_visible());
            return true;
        }
        else if (k.is_keypad_number())
        {
            // filter away keys NP_1...NP_9 to avoid moving viewer like in
            // freeview mode
            return true;
        }
    }
    return freeview_display::handle_key_event(k);
}

bool sub_bridge_display::handle_mouse_wheel_event(const mouse_wheel_data& m)
{
    if (m.up())
    {
        // filter away
        return true;
    }
    else if (m.down())
    {
        // filter away
        return true;
    }
    return freeview_display::handle_mouse_wheel_event(m);
}
