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

// OpenGL GLSL shaders - do NOT use class system in here!
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "shader.h"

#include "error.h"
#include "log.h"
#include "oglext/OglExt.h"
#include "texture.h"

#include <fstream>
#include <memory>
#include <stdexcept>
using namespace std;

/*
Note!
Linux/Nvidia, use:

export __GL_WriteProgramObjectAssembly=1
export __GL_WriteProgramObjectSource=1

to get ASM source.
*/

bool glsl_shader::enable_hqsfx   = false;
bool glsl_shader::is_nvidia_card = false;

const glsl_program* glsl_program::used_program = nullptr;

std::unique_ptr<glsl_shader_setup> glsl_shader_setup::default_opaque;
std::unique_ptr<glsl_shader_setup> glsl_shader_setup::default_col;
std::unique_ptr<glsl_shader_setup> glsl_shader_setup::default_tex;
std::unique_ptr<glsl_shader_setup> glsl_shader_setup::default_coltex;
unsigned glsl_shader_setup::loc_o_color  = 0;
unsigned glsl_shader_setup::idx_c_color  = 0;
unsigned glsl_shader_setup::loc_t_tex    = 0;
unsigned glsl_shader_setup::loc_t_color  = 0;
unsigned glsl_shader_setup::loc_ct_tex   = 0;
unsigned glsl_shader_setup::idx_ct_color = 0;

void glsl_shader_setup::default_init()
{
    // not as external files since this would add a file/compile dependency
    // hmm we always handle the vertex color here, its either constant or
    // per-vertex...
    static const char* vs = "#ifdef USE_TEX\n"
                            "varying vec2 texcoord;\n"
                            "#endif\n"
                            "#ifdef USE_COL\n"
                            "attribute vec4 vcolor;\n"
                            "varying vec4 color;\n"
                            "#endif\n"
                            "void main(){\n"
                            "#ifdef USE_TEX\n"
                            "texcoord = gl_MultiTexCoord0.xy;\n"
                            "#endif\n"
                            "#ifdef USE_COL\n"
                            "color = vcolor;\n"
                            "#endif\n"
                            "gl_Position = ftransform();\n"
                            "}\n";
    static const char* fs = "#ifdef USE_TEX\n"
                            "uniform sampler2D tex;\n"
                            "varying vec2 texcoord;\n"
                            "#endif\n"
                            "#ifdef USE_COL\n"
                            "varying vec4 color;\n"
                            "#else\n"
                            "uniform vec4 color;\n"
                            "#endif\n"
                            "void main(){\n"
                            "vec4 c = color;\n"
                            "#ifdef USE_TEX\n"
                            "c *= texture2D(tex, texcoord.xy);\n"
                            "#endif\n"
                            "gl_FragColor = c;\n"
                            "}\n";

    std::string vss(vs);
    std::string fss(fs);
    glsl_shader::defines_list dl;

    // fixme: which index is returned for gl_Position?
    // if we don't use gl_Position in shaders, but use our own vertex
    // attrib for positions, does the gl2 driver handles this efficiently,
    // i.e. without interpolating the gl_Position additionally?

    default_opaque = std::make_unique<glsl_shader_setup>(vss, fss, dl, true);
    default_opaque->use();
    loc_o_color = default_opaque->get_uniform_location("color");

    dl.push_back("USE_COL");
    default_col = std::make_unique<glsl_shader_setup>(vss, fss, dl, true);
    default_col->use();
    idx_c_color = default_col->get_vertex_attrib_index("vcolor");

    dl.push_back("USE_TEX");
    default_coltex = std::make_unique<glsl_shader_setup>(vss, fss, dl, true);
    default_coltex->use();
    loc_ct_tex   = default_coltex->get_uniform_location("tex");
    idx_ct_color = default_coltex->get_vertex_attrib_index("vcolor");

    dl.pop_front(); // remove "USE_COL"
    default_tex = std::make_unique<glsl_shader_setup>(vss, fss, dl, true);
    default_tex->use();
    loc_t_color = default_tex->get_uniform_location("color");
    loc_t_tex   = default_tex->get_uniform_location("tex");

    default_opaque->use(); // use opaque shader as default
    default_opaque->set_uniform(loc_o_color, colorf(1, 1, 1, 1));
}

void glsl_shader_setup::default_deinit()
{
    default_opaque.reset();
    default_col.reset();
    default_tex.reset();
    default_coltex.reset();
    loc_o_color = 0;
    loc_t_tex   = 0;
    loc_t_color = 0;
    loc_ct_tex  = 0;
}

glsl_shader::glsl_shader(
    const string& filename,
    type stype,
    const glsl_shader::defines_list& dl) :
    id(0)
{
    switch (stype)
    {
        case VERTEX:
        case VERTEX_IMMEDIATE:
            id = glCreateShader(GL_VERTEX_SHADER);
            break;
        case FRAGMENT:
        case FRAGMENT_IMMEDIATE:
            id = glCreateShader(GL_FRAGMENT_SHADER);
            break;
        default:
            throw invalid_argument("invalid shader type");
    }
    if (id == 0)
    {
        THROW(error, "can't create glsl shader");
    }
    try
    {
        // read shader source
        std::unique_ptr<ifstream> ifprg;
        if (stype == VERTEX || stype == FRAGMENT)
        {
            ifprg = std::make_unique<ifstream>(filename.c_str(), ios::in);
            if (ifprg->fail())
            {
                THROW(file_read_error, filename);
            }
        }

        // the program as string
        string prg;

        // always add this mandatory string, some ATI cards don't like it. We
        // don't care.
        prg += "#version 120\n";

        // add special optimizations for Nvidia cards
        if (is_nvidia_card)
        {
            // add some more performance boost stuff if requested
            if (true)
            { // fixme: later add cfg-switch for it
                prg += "#pragma optionNV(fastmath on)\n"
                       "#pragma optionNV(fastprecision on)\n"
                       /*"#pragma optionNV(ifcvt all)\n"*/
                       "#pragma optionNV(inline all)\n"
                    //"#pragma optionNV(unroll all)\n"  not faster on 7x00
                    // hardware
                    ;
            }
        }

        // add global hqsfx flag, but define first, so user can override it
        if (enable_hqsfx)
        {
            prg += "#define HQSFX\n";
        }

        // add defines to top of list for preprocessor
        for (const auto& it : dl)
        {
            prg += string("#define ") + it + "\n";
        }

        // read lines.
        if (ifprg.get())
        {
            while (!ifprg->eof())
            {
                string s;
                getline(*ifprg, s);
                prg += s + "\n";
            }
        }
        else
        {
            prg += filename;
        }

        const char* prg_cstr = prg.c_str();
        glShaderSource(id, 1, &prg_cstr, nullptr);

        glCompileShader(id);

        GLint compiled = GL_FALSE;
        glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);

        // get compile log
        GLint maxlength = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxlength);
        string compilelog(maxlength + 1, ' ');
        GLsizei length = 0;
        glGetShaderInfoLog(id, maxlength, &length, &compilelog[0]);

        if (!compiled)
        {
            log_warning("compiling failed, log:");
            log_warning(compilelog);
            THROW(error, string("compiling of shader failed : ") + filename);
        }

        log_info("shader compiled successfully, log:");
        log_info(compilelog);
    }
    catch (exception& e)
    {
        glDeleteShader(id);
        throw;
    }
}

glsl_shader::~glsl_shader()
{
    glDeleteShader(id);
}

glsl_program::glsl_program()

{
    id = glCreateProgram();
    if (id == 0)
    {
        THROW(error, "can't create glsl program");
    }
}

glsl_program::~glsl_program()
{
    if (used_program == this)
    {
        // can happen now that we use shaders for everything - but it is no
        // problem
        // log_warning("deleting bound glsl program!");
        use_fixed();
    }
    // if shaders are still attached, it is rather a bug...
    for (auto& attached_shader : attached_shaders)
    {
        glDetachShader(id, attached_shader->id);
    }
    glDeleteProgram(id);
}

void glsl_program::attach(glsl_shader& s)
{
    glAttachShader(id, s.id);
    attached_shaders.push_front(&s);
    linked = false;
}

void glsl_program::detach(glsl_shader& s)
{
    glDetachShader(id, s.id);
    for (auto it = attached_shaders.begin(); it != attached_shaders.end();)
    {
        if (*it == &s)
        {
            glDetachShader(id, (*it)->id);
            it = attached_shaders.erase(it);
        }
        else
        {
            ++it;
        }
    }
    linked = false;
}

void glsl_program::link()
{
    glLinkProgram(id);
    GLint waslinked = GL_FALSE;
    glGetProgramiv(id, GL_LINK_STATUS, &waslinked);

    if (!waslinked)
    {
        log_warning("linking failed, log:");
        GLint maxlength = 0;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxlength);
        string compilelog(maxlength + 1, ' ');
        GLsizei length = 0;
        glGetProgramInfoLog(id, maxlength, &length, &compilelog[0]);
        log_warning(compilelog);
        THROW(error, "linking of program failed");
    }

    linked = true;
}

void glsl_program::use() const
{
    if (used_program == this)
    {
        return;
    }
    if (!linked)
    {
        THROW(error, "glsl_program::use() : program not linked");
    }
    glUseProgram(id);
    used_program = this;
}

auto glsl_program::get_uniform_location(const std::string& name) const
    -> unsigned
{
    if (used_program != this)
    {
        THROW(error, "glsl_program::get_uniform_location, program not bound!");
    }
    return unsigned(glGetUniformLocation(id, name.c_str()));
}

void glsl_program::set_gl_texture(
    const texture& tex,
    unsigned loc,
    unsigned texunit) const
{
    if (used_program != this)
    {
        THROW(error, "glsl_program::set_gl_texture, program not bound!");
    }
    glActiveTexture(GL_TEXTURE0 + texunit);
    tex.set_gl_texture();
    glUniform1i(loc, texunit);
}

void glsl_program::set_uniform(unsigned loc, const vector3f& value) const
{
    if (used_program != this)
    {
        THROW(error, "glsl_program::set_uniform, program not bound!");
    }
    glUniform3f(loc, value.x, value.y, value.z);
}

void glsl_program::set_uniform(unsigned loc, const vector2f& value) const
{
    if (used_program != this)
    {
        THROW(error, "glsl_program::set_uniform, program not bound!");
    }
    glUniform2f(loc, value.x, value.y);
}

void glsl_program::set_uniform(
    unsigned loc,
    const std::vector<vector2f>& values) const
{
    if (used_program != this)
    {
        THROW(error, "glsl_program::set_uniform, program not bound!");
    }

    glUniform2fv(loc, values.size() * 2, &values[0].x);
}

void glsl_program::set_uniform(unsigned loc, float value) const
{
    if (used_program != this)
    {
        THROW(error, "glsl_program::set_uniform, program not bound!");
    }
    glUniform1f(loc, value);
}

void glsl_program::set_uniform(unsigned loc, int value) const
{
    if (used_program != this)
    {
        THROW(error, "glsl_program::set_uniform, program not bound!");
    }
    glUniform1i(loc, value);
}

void glsl_program::set_uniform(unsigned loc, const vector3& value) const
{
    if (used_program != this)
    {
        THROW(error, "glsl_program::set_uniform, program not bound!");
    }
    glUniform3f(loc, value.x, value.y, value.z);
}

void glsl_program::set_uniform(unsigned loc, const matrix4& value) const
{
    if (used_program != this)
    {
        THROW(error, "glsl_program::set_uniform, program not bound!");
    }
    float tmp[16];
    const double* ea = value.elemarray();
    for (int i = 0; i < 16; ++i)
    {
        tmp[i] = float(ea[i]);
    }
    glUniformMatrix4fv(loc, 1, GL_TRUE, tmp);
}

void glsl_program::set_uniform(unsigned loc, const vector4f& value) const
{
    if (used_program != this)
    {
        THROW(error, "glsl_program::set_uniform, program not bound!");
    }
    glUniform4f(loc, value.x, value.y, value.z, value.w);
}

void glsl_program::set_uniform(
    unsigned loc,
    const std::vector<vector4f>& values) const
{
    if (used_program != this)
    {
        THROW(error, "glsl_program::set_uniform, program not bound!");
    }

    glUniform4fv(loc, values.size() * 4, &values[0].x);
}

void glsl_program::set_uniform(unsigned loc, const colorf& value) const
{
    if (used_program != this)
    {
        THROW(error, "glsl_program::set_uniform, program not bound!");
    }
    glUniform4f(loc, value.r, value.g, value.b, value.a);
}

auto glsl_program::get_vertex_attrib_index(const std::string& name) const
    -> unsigned
{
    if (used_program != this)
    {
        THROW(
            error, "glsl_program::get_vertex_attrib_index, program not bound!");
    }
    return glGetAttribLocation(id, name.c_str());
}

void glsl_program::use_fixed()
{
    glUseProgram(0);
    used_program = nullptr;
}

auto glsl_program::is_fixed_in_use() -> bool
{
    return used_program == nullptr;
}

glsl_shader_setup::glsl_shader_setup(
    const std::string& filename_vshader,
    const std::string& filename_fshader,
    const glsl_shader::defines_list& dl,
    bool immediate) :
    vs(filename_vshader,
       immediate ? glsl_shader::VERTEX_IMMEDIATE : glsl_shader::VERTEX,
       dl),
    fs(filename_fshader,
       immediate ? glsl_shader::FRAGMENT_IMMEDIATE : glsl_shader::FRAGMENT,
       dl)
{
    prog.attach(vs);
    prog.attach(fs);
    prog.link();
}

void glsl_shader_setup::use() const
{
    prog.use();
}

void glsl_shader_setup::use_fixed()
{
    glsl_program::use_fixed();
}
