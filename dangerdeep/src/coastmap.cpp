// a coastmap
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GL/gl.h>

#include "coastmap.h"
#include "binstream.h"
#include "global_data.h"
#ifndef MAPCOMPILER
#include "texture.h"
#endif
#include <cassert>
#include <fstream>


void coastmap::save(const string& filename) const
{
	ofstream out((get_map_dir() + filename).c_str(), ios::binary|ios::out);
	assert(out.good() && "can't write map file");
	write_u16(out, pixels_per_seg);
	write_u16(out, segsx);
	write_u16(out, segsy);
	write_double(out, pixelw_real);
	write_double(out, offsetx);
	write_double(out, offsety);
	for (vector<coastsegment>::const_iterator it = coastsegments.begin(); it != coastsegments.end(); ++it) {
		it->save(out);
	}
}

coastmap::coastmap(const string& filename)
{
	ifstream in((get_map_dir() + filename).c_str(), ios::in | ios::binary);
	assert(in.good() && "can't read map file");
	pixels_per_seg = read_u16(in);
	segsx = read_u16(in);
	segsy = read_u16(in);
	pixelw_real = read_double(in);
	offsetx = read_double(in);
	offsety = read_double(in);
	unsigned s = segsx * segsy;
	coastsegments.reserve(s);
	for ( ; s > 0; --s) {
		coastsegments.push_back(coastsegment(in));
	}
}

void coastmap::draw_as_map(unsigned detail) const
{
	glBindTexture(GL_TEXTURE_2D, 0);
	double rsegw = pixelw_real * pixels_per_seg;
	double tsx = 1.0/segsx;
	double tsy = 1.0/segsy;
#ifndef MAPCOMPILER
	atlanticmap->set_gl_texture();
#endif	
	double ry = offsety;
	double ty = 0;
	for (unsigned y = 0; y < segsy; ++y) {
		double rx = offsetx;
		double tx = 0;
		for (unsigned x = 0; x < segsx; ++x) {
			coastsegments[y*segsx+x].draw_as_map(rx, ry, rsegw, tx, ty, tsx, tsy, detail);
			rx += rsegw;
			tx += tsx;
		}
		ry += rsegw;
		ty += tsy;
	}
}

void coastmap::render(double px, double py, unsigned detail) const
{
	px -= offsetx;
	py -= offsety;

	// determine which coast segment can be seen (at most 4)
	// fixme
	double rsegw = pixelw_real * pixels_per_seg;
	int moffx = int(px/rsegw);
	int moffy = int(py/rsegw);

	if (moffx >= 0 && moffy >= 0 && moffx < int(segsx) && moffy < int(segsy))	
		coastsegments[moffy*segsx+moffx].render(px - moffx*rsegw, py - moffy*rsegw, detail);
}
