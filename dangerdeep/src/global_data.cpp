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

// global data
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "global_data.h"

#include "constant.h"
#include "datadirs.h"
#include "font.h"
#include "helper.h"
#include "image.h"
#include "log.h"
#include "model.h"
#include "oglext/OglExt.h"
#include "system_interface.h"
#include "texture.h"

#include <SDL_image.h>
#include <iomanip>
#include <list>
#include <sstream>
#include <stdexcept>

using namespace std;

// same with version string
auto get_program_version() -> string
{
    return string(VERSION);
}

global_data::global_data() :
    modelcache(get_data_dir()), imagecache(get_image_dir()),
    texturecache(get_texture_dir())
// soundcache(get_sound_dir())
{
    font_arial = std::make_unique<font>(get_font_dir() + "font_arial");
    font_jphsl = std::make_unique<font>(get_font_dir() + "font_jphsl");

    font_vtremington10 =
        std::make_unique<font>(get_font_dir() + "font_vtremington10");

    font_vtremington12 =
        std::make_unique<font>(get_font_dir() + "font_vtremington12");

    font_typenr16 = std::make_unique<font>(get_font_dir() + "font_typenr16");
}

global_data::~global_data()
{
    font_arial         = nullptr;
    font_jphsl         = nullptr;
    font_vtremington10 = nullptr;
    font_vtremington12 = nullptr;
    font_typenr16      = nullptr;
}

std::unique_ptr<class font> font_arial, font_jphsl, font_vtremington10,
    font_vtremington12, font_typenr16;

// display loading progress
list<string> loading_screen_messages;
unsigned starttime;

void display_loading_screen()
{
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    SYS().prepare_2d_drawing();

    // display a nice loading image in the background
    image* background;
    background = imagecache().ref("entryscreen.png");
    background->draw(0, 0);

    unsigned fh = font_arial->get_height();
    unsigned y  = 0;

    for (auto& loading_screen_message : loading_screen_messages)
    {
        font_arial->print(0, y, loading_screen_message);
        y += fh;
    }
    SYS().unprepare_2d_drawing();
    SYS().finish_frame();
}

void reset_loading_screen()
{
    loading_screen_messages.clear();
    loading_screen_messages.emplace_back("Loading...");
    log_info("Loading...");
    display_loading_screen();
    starttime = SYS().millisec();
}

void add_loading_screen(const string& msg)
{
    unsigned tm        = SYS().millisec();
    unsigned deltatime = tm - starttime;
    starttime          = tm;
    ostringstream oss;
    oss << msg << " (" << deltatime << "ms)";
    loading_screen_messages.push_back(oss.str());
    log_info(oss.str());
    display_loading_screen();
}

auto get_time_string(double tm) -> string
{
    auto seconds     = unsigned(floor(helper::mod(tm, 86400.0)));
    unsigned hours   = seconds / 3600;
    unsigned minutes = (seconds % 3600) / 60;
    seconds          = seconds % 60;

    ostringstream oss;
    oss << setw(2) << setfill('0') << hours << ":" << setw(2) << setfill('0')
        << minutes << ":" << setw(2) << setfill('0') << seconds;

    return oss.str();
}

#define CA 0.0003
void jacobi_amp(double u, double k, double& sn, double& cn)
{
    double emc = 1.0 - k * k;

    double a, b, c, d = 0.0, dn;
    double em[14], en[14];
    int i, ii, l, bo;

    if (emc)
    {
        bo = (emc < 0.0);
        if (bo)
        {
            d = 1.0 - emc;
            emc /= -1.0 / d;
            u *= (d = sqrt(d));
        }
        a  = 1.0;
        dn = 1.0;

        for (i = 1; i <= 13; i++)
        {
            l     = i;
            em[i] = a;
            en[i] = (emc = sqrt(emc));
            c     = 0.5 * (a + emc);
            if (fabs(a - emc) <= CA * a)
            {
                break;
            }
            emc *= a;
            a = c;
        }

        u *= c;
        sn = sin(u);
        cn = cos(u);

        if (sn)
        {
            a = (cn) / (sn);
            c *= a;

            for (ii = l; ii >= 1; ii--)
            {
                b = em[ii];
                a *= c;
                c *= (dn);
                dn = (en[ii] + a) / (b + a);
                a  = c / b;
            }

            a  = 1.0 / sqrt(c * c + 1.0);
            sn = (sn >= 0.0 ? a : -a);
            cn = c * sn;
        }
        if (bo)
        {
            cn = dn;
            sn /= d;
        }
    }
    else
    {
        cn = 1.0 / cosh(u);
        sn = tanh(u);
    }
}
#undef CA

auto transform_real_to_geo(const vector2f& pos) -> vector2f
{
    double sn, cn, r;
    vector2f coord;

    jacobi_amp(pos.y / constant::WGS84_A, constant::WGS84_K, sn, cn);

    r = sqrt(
        (constant::WGS84_B * constant::WGS84_B)
        / (1.0 - constant::WGS84_K * constant::WGS84_K * cn * cn));

    coord.x = (180.0 * pos.x) / (M_PI * r);
    coord.y = (asin(sn) * 180.0) / M_PI;

    return coord;
}

static auto transform_nautic_coord_to_real(
    const string& s,
    char minus,
    char plus,
    int degmax) -> double
{
    if (s.length() < 2)
    {
        THROW(error, string("nautic coordinate invalid ") + s);
    }

    char sign = s[s.length() - 1];
    if (sign != minus && sign != plus)
    {
        THROW(error, string("nautic coordinate (direction sign) invalid ") + s);
    }

    // find separator
    string::size_type st = s.find('/');
    if (st == string::npos)
    {
        THROW(error, string("no separator in position string ") + s);
    }

    string degrees = s.substr(0, st);
    string minutes = s.substr(st + 1, s.length() - st - 2);
    int deg        = atoi(degrees.c_str());

    if (deg < 0 || deg > degmax)
    {
        THROW(
            error,
            string("degrees are not in range [0...180/360] in position string ")
                + s);
    }

    int mts = atoi(minutes.c_str());
    if (mts < 0 || mts > 59)
    {
        THROW(
            error,
            string("minutes are not in [0...59] in position string ") + s);
    }

    return (sign == minus ? -1 : 1)
           * ((constant::DEGREE_IN_METERS * deg)
              + (constant::MINUTE_IN_METERS * mts));
}

auto transform_nautic_posx_to_real(const string& s) -> double
{
    return transform_nautic_coord_to_real(s, 'W', 'E', 180);
}

auto transform_nautic_posy_to_real(const string& s) -> double
{
    return transform_nautic_coord_to_real(s, 'S', 'N', 90);
}

auto string_split(const std::string& src, char splitter)
    -> std::list<std::string>
{
    std::list<std::string> result;
    std::string::size_type where = 0, st = 0;
    do
    {
        st = src.find(splitter, where);
        if (st == std::string::npos)
        {
            // rest of string
            result.push_back(src.substr(where));
        }
        else
        {
            result.push_back(src.substr(where, st - where));
            where = st + 1;
        }
    } while (st != std::string::npos);
    return result;
}

void save_pgm(
    const char* fn,
    unsigned w,
    unsigned h,
    const uint8_t* d,
    unsigned stride)
{
    std::ofstream osg(fn);

    if (!osg.good())
    {
        THROW(error, std::string("Can't open output file ") + fn);
    }

    osg << "P5\n";
    osg << w << " " << h << "\n255\n";

    if (!stride)
    {
        stride = w;
    }

    for (unsigned y = 0; y < h; ++y)
    {
        osg.write(reinterpret_cast<const char*>(d), w);
        d += stride;
    }
}
