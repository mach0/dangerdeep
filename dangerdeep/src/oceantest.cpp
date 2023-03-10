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

// (ocean) water simulation test
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "ocean_wave_generator.h"

#include <fstream>
using namespace std;

#ifndef fmin
#define fmin(x, y) (x < y) ? x : y
#endif
#ifndef fmax
#define fmax(x, y) (x > y) ? x : y
#endif

int main(int argc, char** argv)
{
    const unsigned resbig = 1024;
    const unsigned ressml = 128;
    srand(1234);
    /*
      Wave tile length of 256 gives only few detail at small waves.
      Rather use length of 512 or more. That way the tiles will be less visible.
      Resolutions that are too high (4096 or so) don't bring any more detail.
      For a 256m wide tile there is are only few high-frequency details.
      (256m vs. 4096 res => 6.25cm per pixel).
      Values below 25cm or so aren't noticeable. So use 25cm details, with 128
      res this gives a subdetail tile of 32m. This means a 256m tile with 128
      res, so 2m per pixel. And a (bump map) hf-tile with 128 res, 32m length,
      so 0.25m per pixel. The fft translates frequencies to heights, so we have
      128x128 pixel or 128x128 amplitude values for frequencies. We want only
      the higher frequencies, this means anything below 2m. With 32m to 256m the
      hf-tile is 1/8 size of the real tile. So the frequencies for 1/1, 1/2, 1/4
      (and maybe 1/8) must be cleared. This means only 4 of 128 lines/columns,
      or clearing 1/4 of all lines/columns? Which frequencies are stored?
      1/1,1/2,1/3,...1/128 ???
    */

    ocean_wave_generator<float> owg1(
        resbig,         // wave resolution
        vector2f(1, 1), // wind dir
        12,             // wind speed m/s
        1e-8,           // scale factor "A"
        256,            // wave tile length
        10);            // tide cycle time in seconds
    srand(1234);
    // if we delete the higher 256 frequencies, it doesn't seem to make
    // any difference (with a 1024 original resolution)
    // if we delete the higher 384 frequencies only very tiny differences
    // occur, that can be seen only when comparing the two images in gimp
    // with max. contrast.
    // 512 is way too much removed, 480 too less, 500 too much. 490 too less.
    // 496-498 seems to match the upscaled 128->1024-version, but shows some
    // rectangular artefacts. A bit ugly...
    // with 498 there is 498/512 frequencies removed in each direction,
    // so only upper 14 frequencies remain. No wonder the picture shows
    // rectangular structures. The question is if we compute a 1024 with top
    // removed 16 and low removed rest and combine that, would it look like
    // the original?
    ocean_wave_generator<float> owg3(owg1, resbig, -498);

    srand(1234);

    ocean_wave_generator<float> owg4(owg1, resbig, 512 - 498);

    srand(1234);

    /*
        ocean_wave_generator<float> owg2(ressml,	// wave resolution
                         vector2f(1,1),	// wind dir
                         12,	// wind speed m/s
                         1e-8,	// scale factor "A"
                         1024,	// wave tile length
                         100);	// tide cycle time in seconds
    */
    ocean_wave_generator<float> owg2(owg1, ressml);

    srand(1234);
    owg1.set_time(0);
    srand(1234);
    owg2.set_time(0);
    srand(1234);
    owg3.set_time(0);
    srand(1234);
    owg4.set_time(0);
    vector<float> heights1;
    vector<float> heights2;
    vector<float> heights5;
    vector<float> heights6;
    cout << "gen 1...\n";
    owg1.compute_heights(heights1);
    cout << "gen 2...\n";
    owg2.compute_heights(heights2);
    cout << "gen 3...\n";
    owg3.compute_heights(heights5);
    cout << "gen 4...\n";
    owg4.compute_heights(heights6);

    // compute an interpolated version of the lower one
    vector<float> heights3 = heights1;
    int fac                = resbig / ressml;
    for (unsigned int y = 0; y < resbig; ++y)
    {
        int yy   = y / fac;
        int y2   = (yy + 1) % ressml;
        float yr = float(y - yy * fac) / fac;
        for (unsigned int x = 0; x < resbig; ++x)
        {
            int xx                   = x / fac;
            int x2                   = (xx + 1) % ressml;
            float xr                 = float(x - xx * fac) / fac;
            float h0                 = heights2[yy * ressml + xx];
            float h1                 = heights2[yy * ressml + x2];
            float h2                 = heights2[y2 * ressml + xx];
            float h3                 = heights2[y2 * ressml + x2];
            float h4                 = (1 - xr) * h0 + xr * h1;
            float h5                 = (1 - xr) * h2 + xr * h3;
            heights3[y * resbig + x] = (1 - yr) * h4 + yr * h5;
        }
    }

    vector<float> heights4 = heights3;
    for (unsigned i = 0; i < heights3.size(); ++i)
    {
        heights4[i] = heights1[i] - heights3[i];
    }

    // add 5+6 -> 7
    vector<float> heights7 = heights5;
    for (unsigned i = 0; i < heights7.size(); ++i)
        // we seem to need some scaling here, but in theory we shouldn't.
        // so the way we create the maps and add them doesnt seem to be the same
        // as creating it in one step with one FFT.
        heights7[i] += heights6[i] * 0.5;

    float minh = 1e10;
    float maxh = -1e10;
    for (float it : heights1)
    {
        minh = fmin(minh, it);
        maxh = fmax(maxh, it);
    }
    cout << "1: minh " << minh << " maxh " << maxh << "\n";
    ofstream osg1("waveh1.pgm");
    osg1 << "P5\n";
    osg1 << resbig << " " << resbig << "\n255\n";
    for (float it : heights1)
    {
        auto h = uint8_t((it - minh) * 255 / (maxh - minh));
        osg1.write((const char*) &h, 1);
    }

    minh = 1e10;
    maxh = -1e10;
    for (float it : heights2)
    {
        minh = fmin(minh, it);
        maxh = fmax(maxh, it);
    }
    cout << "2: minh " << minh << " maxh " << maxh << "\n";
    ofstream osg2("waveh2.pgm");
    osg2 << "P5\n";
    osg2 << ressml << " " << ressml << "\n255\n";
    for (float it : heights2)
    {
        auto h = uint8_t((it - minh) * 255.9 / (maxh - minh));
        osg2.write((const char*) &h, 1);
    }

    minh = 1e10;
    maxh = -1e10;
    for (float it : heights3)
    {
        minh = fmin(minh, it);
        maxh = fmax(maxh, it);
    }
    cout << "3: minh " << minh << " maxh " << maxh << "\n";
    ofstream osg3("waveh3.pgm");
    osg3 << "P5\n";
    osg3 << resbig << " " << resbig << "\n255\n";
    for (float it : heights3)
    {
        auto h = uint8_t((it - minh) * 255.9 / (maxh - minh));
        osg3.write((const char*) &h, 1);
    }

    minh = 1e10;
    maxh = -1e10;
    for (float it : heights4)
    {
        minh = fmin(minh, it);
        maxh = fmax(maxh, it);
    }
    cout << "4: minh " << minh << " maxh " << maxh << "\n";
    ofstream osg4("waveh4.pgm");
    osg4 << "P5\n";
    osg4 << resbig << " " << resbig << "\n255\n";
    for (float it : heights4)
    {
        auto h = uint8_t((it - minh) * 255.9 / (maxh - minh));
        osg4.write((const char*) &h, 1);
    }

    minh = 1e10;
    maxh = -1e10;
    for (float it : heights5)
    {
        minh = fmin(minh, it);
        maxh = fmax(maxh, it);
    }
    cout << "5: minh " << minh << " maxh " << maxh << "\n";
    ofstream osg5("waveh5.pgm");
    osg5 << "P5\n";
    osg5 << resbig << " " << resbig << "\n255\n";
    for (float it : heights5)
    {
        auto h = uint8_t((it - minh) * 255.9 / (maxh - minh));
        osg5.write((const char*) &h, 1);
    }

    minh = 1e10;
    maxh = -1e10;
    for (float it : heights6)
    {
        minh = fmin(minh, it);
        maxh = fmax(maxh, it);
    }
    cout << "6: minh " << minh << " maxh " << maxh << "\n";
    ofstream osg6("waveh6.pgm");
    osg6 << "P5\n";
    osg6 << resbig << " " << resbig << "\n255\n";
    for (float it : heights6)
    {
        auto h = uint8_t((it - minh) * 255.9 / (maxh - minh));
        osg6.write((const char*) &h, 1);
    }

    minh = 1e10;
    maxh = -1e10;
    for (float it : heights7)
    {
        minh = fmin(minh, it);
        maxh = fmax(maxh, it);
    }
    cout << "7: minh " << minh << " maxh " << maxh << "\n";
    ofstream osg7("waveh7.pgm");
    osg7 << "P5\n";
    osg7 << resbig << " " << resbig << "\n255\n";
    for (float it : heights7)
    {
        auto h = uint8_t((it - minh) * 255.9 / (maxh - minh));
        osg7.write((const char*) &h, 1);
    }

    return 0;
}
