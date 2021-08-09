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

// user display: submarine's periscope
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#pragma once

#include "freeview_display.h"
#include "shader.h"
#include "texture.h"

/// A display for submarine's periscope
class sub_periscope_display : public freeview_display
{
  public:
    sub_periscope_display(class user_interface& ui_);

    // overload for zoom key handling ('y') and TDC input
    bool handle_key_event(const key_data&) override;
    bool handle_mouse_motion_event(const mouse_motion_data&) override;
    bool handle_mouse_wheel_event(const mouse_wheel_data&) override;
    void display() const override;

    unsigned get_popup_allow_mask() const override;

  protected:
    void pre_display() const override;
    projection_data get_projection_data(class game& gm) const override;
    void
    set_modelview_matrix(class game& gm, const vector3& viewpos) const override;
    void post_display() const override;

    bool zoomed{false}; // use 1,5x (false) or 6x zoom (true)

    bool use_hqsfx{false};
    texture::ptr viewtex;
    texture::ptr blurtex;
    std::unique_ptr<glsl_shader_setup> glsl_blurview;
    unsigned loc_blur_texc_offset;
    unsigned loc_tex_view;
    unsigned loc_tex_blur;

    vector3 get_viewpos(class game& gm) const override;
};
