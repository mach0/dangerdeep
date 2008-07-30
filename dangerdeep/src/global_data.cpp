/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

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

#include "model.h"
#include "texture.h"
#include "image.h"
#include "font.h"
#include "global_data.h"
#include <sstream>
#include <list>
#include <iomanip>
#include "oglext/OglExt.h"
#include "system.h"
#include "log.h"
#include "datadirs.h"
#include "global_constants.h"
#include <SDL_image.h>
#include <stdexcept>

using namespace std;



// same with version string
string get_program_version()
{
	return string(VERSION);
}



global_data::global_data()
	: modelcache(get_data_dir()),
	  imagecache(get_image_dir()),
	  texturecache(get_texture_dir()),
	  //soundcache(get_sound_dir()),
	  fontcache(get_font_dir())
{
}



global_data::~global_data()
{
}


font *font_arial, *font_jphsl, *font_vtremington10, *font_vtremington12, 
     *font_typenr16;

bool loading_screen_usable = false;

void init_global_data()
{
	font_arial = fontcache().ref("font_arial");
	loading_screen_usable = true;
	font_jphsl = fontcache().ref("font_jphsl");
	font_vtremington10 = fontcache().ref("font_vtremington10");
	font_vtremington12 = fontcache().ref("font_vtremington12");
	font_typenr16 = fontcache().ref("font_typenr16");
	add_loading_screen("fonts loaded");
}

void deinit_global_data()
{
}

// display loading progress
list<string> loading_screen_messages;
unsigned starttime;
void display_loading_screen()
{
	if (!loading_screen_usable) return;

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	sys().prepare_2d_drawing();
	glColor4f(1, 1, 1, 1);

	// display a nice loading image in the background
	image* background;
	background = imagecache().ref( "entryscreen.png" );
	background->draw( 0, 0 );

	unsigned fh = font_arial->get_height();
	unsigned y = 0;
	for (list<string>::const_iterator it = loading_screen_messages.begin();
	     it != loading_screen_messages.end(); ++it) {
		font_arial->print(0, y, *it);
		y += fh;
	}
	sys().unprepare_2d_drawing();
	sys().swap_buffers();
}

void reset_loading_screen()
{
	loading_screen_messages.clear();
	loading_screen_messages.push_back("Loading...");
	log_info("Loading...");
	display_loading_screen();
	starttime = sys().millisec();
}

void add_loading_screen(const string& msg)
{
	unsigned tm = sys().millisec();
	unsigned deltatime = tm - starttime;
	starttime = tm;
	ostringstream oss;
	oss << msg << " (" << deltatime << "ms)";
	loading_screen_messages.push_back(oss.str());
	log_info(oss.str());
	display_loading_screen();
}

string get_time_string(double tm)
{
	unsigned seconds = unsigned(floor(myfmod(tm, 86400)));
	unsigned hours = seconds / 3600;
	unsigned minutes = (seconds % 3600) / 60;
	seconds = seconds % 60;
	ostringstream oss;
	oss << setw(2) << setfill('0') << hours << ":" 
	    << setw(2) << setfill('0') << minutes << ":"
	    << setw(2) << setfill('0') << seconds;
	return oss.str();
}


static double transform_nautic_coord_to_real(const string& s, char minus, char plus, int degmax)
{
	if (s.length() < 2)
		throw error(string("nautic coordinate invalid ") + s);
	char sign = s[s.length() - 1];
	if (sign != minus && sign != plus)
		throw error(string("nautic coordinate (direction sign) invalid ") + s);
	// find separator
	string::size_type st = s.find("/");
	if (st == string::npos)
		throw error(string("no separator in position string ") + s);
	string degrees = s.substr(0, st);
	string minutes = s.substr(st + 1, s.length() - st - 2);
	int deg = atoi(degrees.c_str());
	if (deg < 0 || deg > degmax)
		throw error(string("degrees are not in range [0...180/360] in position string ") + s);
	int mts = atoi(minutes.c_str());
	if (mts < 0 || mts > 59)
		throw error(string("minutes are not in [0...59] in position string ") + s);
	return (sign == minus ? -1 : 1) * ((DEGREE_IN_METERS * deg) + (MINUTE_IN_METERS * mts));
}

double transform_nautic_posx_to_real(const string& s)
{
	return transform_nautic_coord_to_real(s, 'W', 'E', 180);
}

double transform_nautic_posy_to_real(const string& s)
{
	return transform_nautic_coord_to_real(s, 'S', 'N', 90);
}

std::list<std::string> string_split(const std::string& src, char splitter)
{
	std::list<std::string> result;
	std::string::size_type where = 0, st = 0;
	do {
		st = src.find(splitter, where);
		if (st == std::string::npos) {
			// rest of string
			result.push_back(src.substr(where));
		} else {
			result.push_back(src.substr(where, st - where));
			where = st + 1;
		}
	} while (st != std::string::npos);
	return result;
}


void save_pgm(const char* fn, unsigned w, unsigned h, const Uint8* d, unsigned stride)
{
	std::ofstream osg(fn);
	if (!osg.good()) throw error(std::string("Can't open output file ") + fn);
	osg << "P5\n";
	osg << w << " " << h << "\n255\n";
	if (!stride) stride = w;
	for (unsigned y = 0; y < h; ++y) {
		osg.write((const char*)d, w);
		d += stride;
	}
}
