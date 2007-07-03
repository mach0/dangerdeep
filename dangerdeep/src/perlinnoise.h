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

// perlin 2d noise generator
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#ifndef M_PI
#define M_PI 3.1415927
#endif

#include "fixed.h"
#include <vector>


///\brief Generates perlin noise images.
class perlinnoise
{
public:
	// quadratic noise
	struct noise_func
	{
		std::vector<Uint8> data;
		unsigned size;		// in powers of two
		unsigned frequency;	// 1-x
		fixed32 phasex;
		fixed32 phasey;
		// create random noise function
		noise_func(unsigned s, unsigned f, float px = 0.0f, float py = 0.0f);

		// interpolate noise function value
		mutable unsigned offsetline1, offsetline2;
		mutable fixed32 linefac1;
		mutable fixed32 linefac2;
		void set_line_for_interpolation(const std::vector<fixed32>& interpolation_func, fixed32 y) const;
		Uint8 interpolate(const std::vector<fixed32>& interpolation_func, fixed32 x) const;
		Uint8 interpolate_sqr(fixed32 x, fixed32 y) const;
	};

protected:
	std::vector<noise_func> noise_functions;
	unsigned resultsize;

	std::vector<fixed32> interpolation_func;

public:
	// give size of result (power of two), size of noise function with minimal frequency and maximum frequency
	// sizeminfreq is usually very small, 2 or 4 at least, at most the same as size, at least 2
	// sizemaxfreq is usually very high, at most the same as size, at least as sizeminfreq
	perlinnoise(unsigned size, unsigned sizeminfreq, unsigned sizemaxfreq);

	// get number of functions/levels
	unsigned get_number_of_levels() const { return noise_functions.size(); }

	// set phase of a level
	void set_phase(unsigned level, float px, float py);

	// generate a composition of the noise functions
	std::vector<Uint8> generate() const;

	// generate a composition of the noise functions with x^2 interpolation
	std::vector<Uint8> generate_sqr() const;

	//fixme: we need 2 enhancements:
	//1. constructor to create levels of even sizes, so result can be very big
	//2. a function to compute a certain sample (for big noise)
	//2^16*2^16 noises are possible...
	//set linefac code must be copied to that function -> costly
	perlinnoise(unsigned size, unsigned levels);

	Uint8 value(unsigned x, unsigned y, unsigned depth = 0xffffffff) const;
};

#endif
