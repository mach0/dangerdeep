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

#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

// fixme: do we need that on win32? probably not, because we have SDL...
#ifdef WIN32
// 2006-12-01 doc1972 added check to prevent double definition. 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */
#include <windows.h>
#endif

#include <SDL.h>
#include <string>
#include <list>
#include <cmath>
#include "objcache.h"
#include <sstream>
#include <iomanip>

std::string get_program_version();


///> all global data grouped in one class
class global_data
{
 private:
	// no copy
	global_data(const global_data& );
	global_data& operator= (const global_data& );

 protected:
	static global_data* inst;

 public:
	objcachet<class model> modelcache;
	objcachet<class image> imagecache;
	objcachet<class texture> texturecache;
	//objcachet<class sound> soundcache;
	objcachet<class font> fontcache;

	global_data();
	~global_data();

	static global_data& instance() { return *inst; }
};

inline objcachet<class model>& modelcache() { return global_data::instance().modelcache; }
inline objcachet<class image>& imagecache() { return global_data::instance().imagecache; }
inline objcachet<class texture>& texturecache() { return global_data::instance().texturecache; }
//inline objcachet<class sound>& soundcache() { return global_data::instance().soundcache; }
inline objcachet<class font>& fontcache() { return global_data::instance().fontcache; }


// define here
#define CHECK_TORP_SIGSEGV_BUG

// special functions/classes for this check
#ifdef CHECK_TORP_SIGSEGV_BUG
#include <map>
class trashcan
{
	std::map<void*, std::pair<double, const char*> > garbageobjs;
	static trashcan* instance;
 public:
	trashcan() : time(0) {}
	void add(void* obj, double gametime, const char* name) {
		garbageobjs[obj] = std::make_pair(gametime, name);
	}
	const char* has_obj(void* obj, double& gametime) const {
		std::map<void*, std::pair<double, const char*> >::const_iterator it
			= garbageobjs.find(obj);
		if (it != garbageobjs.end()) {
			gametime = it->second.first;
			return it->second.second;
		} else {
			return 0;
		}
	}
	double time; // can be read globally, set by game::simulate
	static trashcan& inst() { if (!instance) instance = new trashcan(); return *instance; }
};
#endif
	


// store in objects where it is needed/ref'd! fixme
extern class texture *notepadsheet,
	*terraintex, *panelbackground;

// only used as shortcut, fonts are managed by fontcache.
extern class font *font_arial, *font_arialbd, *font_times, *font_timesbd, *font_verdana, *font_verdanabd, *font_olympiaworn, *font_damagedtypewriter, *font_king, *font_typenr, *font_jphsl, *font_janeaust, *font_vtportable, *font_vtremington8, *font_vtremington10, *font_vtremington12, *font_vtremington14;

void init_global_data();
void deinit_global_data();

// display loading progress
void reset_loading_screen();
void add_loading_screen(const std::string& msg);

// transform time in seconds to 24h time of clock string (takes remainder of 86400 seconds first = 1 day)
std::string get_time_string(double tm);

// handle modulo calculation for negative values the way I need it
inline float myfmod(float a, float b) { return a-floorf(a/b)*b; }//fmod is different for negative a/b
inline float myfrac(float a) { return a-floorf(a); }
inline double myfmod(double a, double b) { return a-floor(a/b)*b; }//fmod is different for negative a/b
inline double myfrac(double a) { return a-floor(a); }
inline float mysgn(float a) { return (a < 0) ? -1.0f : ((a > 0) ? 1.0f : 0.0f); }
inline double mysgn(double a) { return (a < 0) ? -1.0 : ((a > 0) ? 1.0 : 0.0); }
template<class T> inline T myclamp(const T& v, const T& minv, const T& maxv) { return (v < minv) ? minv : ((v > maxv) ? maxv : v); }
template<class C> inline void add_saturated(C& sum, const C& add, const C& max) { sum += add; if (sum > max) sum = max; }
// return a random value in [0, 1(
inline double rnd() { return double(rand())/RAND_MAX; }
inline unsigned rnd(unsigned b) { return unsigned(b*rnd()); }

// fast clamping
inline Sint32 clamp_zero(Sint32 x) { return x & ~(x >> 31); }
inline Sint32 clamp_value(Sint32 x, Sint32 val) { return val - clamp_zero(val - x); }

inline unsigned ulog2(unsigned x) { unsigned i = 0; for ( ; x > 0; ++i, x >>= 1); return i - 1; }
inline unsigned nextgteqpow2(unsigned x) { unsigned i = 1; for ( ; i < x && i != 0; i <<= 1); return i; }
inline bool ispow2(unsigned x) { return (x & (x-1)) == 0; }

// give degrees,minutes like this 123/45x with x = N,W,E,S 
double transform_nautic_posx_to_real(const std::string& s);
double transform_nautic_posy_to_real(const std::string& s);

std::list<std::string> string_split(const std::string& src, char splitter = ',');

///> transform any data type to a string for easier error reporting etc.
template<typename T> std::string str(const T& x) { std::ostringstream oss; oss << x; return oss.str(); }
///> do the same with width and filler
template<typename T> std::string str_wf(const T& x, unsigned width, char filler = '0') { std::ostringstream oss; oss << std::setw(width) << std::setfill(filler) << x; return oss.str(); }

#endif
