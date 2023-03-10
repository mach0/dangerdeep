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

// OpenGL primitives container
// (C)+(W) by Thorsten Jordan. See LICENSE

#pragma once

#include "color.h"
#include "oglext/OglExt.h"
#include "shader.h"
#include "vector3.h"

#include <vector>

/*
give color and/or texture ref to c'tor
that way glColor is obsolete
*/

class texture;

/// this class models OpenGL primitives with fix vertex count
template<unsigned size>
class primitive
{
    primitive() = delete;

  public:
    primitive(int type_, const colorf& col_) : type(type_), col(col_) { }
    void render_plain()
    {
        glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &vertices[0]);
        glDrawArrays(type, 0, size);
    }
    void render()
    {
        glsl_shader_setup::default_opaque->use();
        glsl_shader_setup::default_opaque->set_uniform(
            glsl_shader_setup::loc_o_color, col);
        render_plain();
    }

    int type;
    colorf col;
    vector3f vertices[size];
};

/// this class models OpenGL primitives with fix vertex count and colors
template<unsigned size>
class primitive_col
{
    primitive_col() = delete;

  public:
    primitive_col(int type_) : type(type_) { }
    void render_plain()
    {
        glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &vertices[0]);
        glVertexAttribPointer(
            glsl_shader_setup::idx_c_color,
            4,
            GL_UNSIGNED_BYTE,
            GL_TRUE,
            0,
            &colors[0]);
        glEnableVertexAttribArray(glsl_shader_setup::idx_c_color);
        glDrawArrays(type, 0, size);
        glDisableVertexAttribArray(glsl_shader_setup::idx_c_color);
    }
    void render()
    {
        glsl_shader_setup::default_col->use();
        render_plain();
    }

    int type;
    vector3f vertices[size];
    color colors[size];
};

/// this class models OpenGL primitives with fix vertex count and texcoords
template<unsigned size>
class primitive_tex
{
    primitive_tex() = delete;

  public:
    primitive_tex(int type_, const colorf& col_, const texture& tex_) :
        type(type_), col(col_), tex(tex_)
    {
    }
    void render_plain()
    {
        glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &vertices[0]);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &texcoords[0]);
        glDrawArrays(type, 0, size);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    void render()
    {
        glsl_shader_setup::default_tex->use();
        glsl_shader_setup::default_tex->set_uniform(
            glsl_shader_setup::loc_t_color, col);
        glsl_shader_setup::default_tex->set_gl_texture(
            tex, glsl_shader_setup::loc_t_tex, 0);
        render_plain();
    }

    int type;
    colorf col;
    const texture& tex;
    vector3f vertices[size];
    vector2f texcoords[size];
};

/// this class models OpenGL primitives with fix vertex count and colors +
/// texcoords
template<unsigned size>
class primitive_coltex
{
    primitive_coltex() = delete;

  public:
    primitive_coltex(int type_, const texture& tex_) :
        type(type_), tex(tex_) { }
    void render_plain()
    {
        glVertexPointer(3, GL_FLOAT, sizeof(vector3f), &vertices[0]);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, sizeof(vector2f), &texcoords[0]);
        glVertexAttribPointer(
            glsl_shader_setup::idx_ct_color,
            4,
            GL_UNSIGNED_BYTE,
            GL_TRUE,
            0,
            &colors[0]);
        glEnableVertexAttribArray(glsl_shader_setup::idx_ct_color);
        glDrawArrays(type, 0, size);
        glDisableVertexAttribArray(glsl_shader_setup::idx_ct_color);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    void render()
    {
        glsl_shader_setup::default_coltex->use();
        glsl_shader_setup::default_coltex->set_gl_texture(
            tex, glsl_shader_setup::loc_ct_tex, 0);
        render_plain();
    }

    int type;
    const texture& tex;
    vector3f vertices[size];
    color colors[size];
    vector2f texcoords[size];
};

/// this class models OpenGL primitives with variable vertex count
class primitives_plain
{
    primitives_plain() = delete;

  public:
    primitives_plain(
        int type,
        unsigned size,
        bool with_colors = false,
        bool with_tex    = false);
    void render();

    int type;
    std::vector<vector3f> vertices;
    std::vector<color> colors;
    std::vector<vector2f> texcoords;
};

/// this class models OpenGL primitives with variable vertex count and default
/// shaders
class primitives : public primitives_plain
{
    primitives() = delete;

  public:
    primitives(int type, unsigned size, const colorf& col); // uni-color
    primitives(int type, unsigned size);                    // per-vertex colors
    primitives(
        int type,
        unsigned size,
        const colorf& col,
        const texture& tex); // uni-color + tex
    primitives(
        int type,
        unsigned size,
        const texture& tex); // per-vertex colors, texcoords
    void render();
    void render_plain() { primitives_plain::render(); }

    /// render a 2d textured quad, face is back-sided
    static primitive_tex<4> textured_quad(
        const vector2f& xy0,
        const vector2f& xy1,
        const texture& tex,
        const vector2f& texc0 = vector2f(0, 0),
        const vector2f& texc1 = vector2f(1, 1),
        const colorf& col     = colorf(1, 1, 1, 1));
    /// render a 2d quad
    static primitive<4>
    quad(const vector2f& xy0, const vector2f& xy1, const colorf& col);
    /// render a 2d triangle
    static primitive<3> triangle(
        const vector2f& xy0,
        const vector2f& xy1,
        const vector2f& xy2,
        const colorf& col);
    /// render a 2d rectangle
    static primitive<4>
    rectangle(const vector2f& xy0, const vector2f& xy1, const colorf& col);
    /// render a 2d diamond
    static primitive<4> diamond(const vector2f& xy, float r, const colorf& col);
    /// render a 2d line
    static primitive<2>
    line(const vector2f& xy0, const vector2f& xy1, const colorf& col);
    /// render a 2d circle
    static primitives
    circle(const vector2f& xy, float radius, const colorf& col);
    /// render a 3d line
    static primitive<2>
    line(const vector3f& xyz0, const vector3f& xyz1, const colorf& col);
    /// render a 3d textured quad
    static primitive_tex<4> textured_quad(
        const vector3f& xyz0,
        const vector3f& xyz1,
        const vector3f& xyz2,
        const vector3f& xyz3,
        const texture& tex,
        const vector2f& texc0 = vector2f(0, 0),
        const vector2f& texc1 = vector2f(1, 1),
        const colorf& col     = colorf(1, 1, 1, 1));
    /// render a 3d z-axis aligned cylinder
    static primitives cylinder_z(
        double radius_bottom,
        double radius_top,
        double z_bottom,
        double z_top,
        double alpha,
        const texture& tex,
        double u_scal,
        unsigned nr_segs,
        bool inside = false);

    colorf col;
    const texture* tex;
};
