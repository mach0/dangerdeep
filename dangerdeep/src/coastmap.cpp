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
#include "texture.h"
#include "system.h"
#include "triangulate.h"
#include "tinyxml/tinyxml.h"
#include <SDL_image.h>
#include <cassert>
#include <fstream>
#include <list>
#include <vector>
using namespace std;



const unsigned BSPLINE_SMOOTH_FACTOR = 16;	// should be 3...16

const int coastmap::dmx[4] = { -1, 0, 0, -1 };
const int coastmap::dmy[4] = { -1, -1, 0, 0 };
const int coastmap::dx[4] = { 0, 1, 0, -1 };
const int coastmap::dy[4] = { -1, 0, 1, 0 };



/* fixme:
  maybe reuse already generated data: give old point array to. if new detail is higher than
  old detail, just generate new points (50% computation time saving on increasing the detail),
  or drop old points with decreasing detail (100% computation time saving).
*/
void coastline::create_points(vector<vector2f>& points, float begint, float endt, int detail) const
{
assert(endt>begint);//fixme quick hack to avoid bugs caused by other fixme's etc.
	// compute number of points to be generated
	unsigned totalpts = curve.control_points().size();
	unsigned nrpts = unsigned(round(float(totalpts) * (endt-begint)));
	if (detail < 0) nrpts = (nrpts >> -detail); else nrpts = (nrpts << detail);
	if (nrpts < 2) nrpts = 2;

	points.reserve(points.size() + nrpts);
	float t = begint, tstep = (endt - begint) / float(nrpts - 1);
	for (unsigned i = 0; i < nrpts; ++i) {
		if (t > 1.0f) t = 1.0f;
		points.push_back(curve.value(t));
		t += tstep;
	}
}



void coastline::draw_as_map(int detail) const
{
	vector<vector2f> pts;
	create_points(pts, 0, 1, detail);

	glBegin(GL_LINE_STRIP);
	for (vector<vector2f>::const_iterator it = pts.begin(); it != pts.end(); ++it) {
		glTexCoord2f(it->x, it->y);
		glVertex2f(it->x, it->y);
	}
	glEnd();

#if 0
	glPointSize(2.0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_POINTS);
	for (vector<vector2f>::const_iterator it = pts.begin(); it != pts.end(); ++it) {
		glVertex2f(it->x, it->y);
	}
	glEnd();
#endif
}



#if 0
void coastline::render(const vector2& p, int detail) const
{
	// fixme: cache that somehow
	vector<vector2f> pts = create_points(p, 1, 0, points.size(), detail);
	
	glPushMatrix();
	glTranslatef(-p.x, -p.y, 0);
	
	unsigned ps = pts.size();
	glBegin(GL_QUAD_STRIP);
	double coastheight = 50;
	float t = 0.0;
	for (unsigned s = 0; s < ps; ++s) {
		const vector2f& p = pts[s];
		unsigned prevpt = (s > 0) ? s-1 : s;
		unsigned nextpt = (s < ps-1) ? s+1 : s;
		vector2f n = (pts[nextpt] - pts[prevpt]).orthogonal().normal() * 50.0;
		if (s == 0) {
			glTexCoord2f(t, 1);
			glVertex3f(p.x, p.y, -10);
			glTexCoord2f(t, 0);
			glVertex3f(p.x+n.x, p.y+n.y, coastheight);
		} else {
			glTexCoord2f(t, 0);	// texture coordinates are wrong, fixme
			glVertex3f(p.x+n.x, p.y+n.y, coastheight);
			glTexCoord2f(t, 1);
			glVertex3f(p.x, p.y, -10);
		}
		t += 1.0;
	}
	glEnd();
	
	glPopMatrix();
}
#endif



// the least distance between two points is around 0.014
void coastsegment::cacheentry::push_back_point(const vector2f& p)
{
//	debug code that avoids double points, not needed any longer
//	if (points.size() == 0 || (points.back().square_distance(p) >= 1.0f))
		points.push_back(p);
}



float gmd=1e30;
void coastsegment::generate_point_cache(const class coastmap& cm, const vector2f& roff, unsigned detail) const
{
	if (type > 1) {
		// cache generated and unchanged?
		if (pointcache.size() > 0 && pointcachedetail == detail) return;

		// invalidate cache (detail changed or initial generation)
		pointcachedetail = detail;
		pointcache.clear();

		unsigned nrcl = segcls.size();
		vector<bool> cl_handled(nrcl, false);
		for (unsigned i = 0; i < nrcl; ++i) {
			if (cl_handled[i]) continue;

			cacheentry ce;

		cout << "about to find area!\n";
			// find area
			unsigned current = i;
			do {
				/* NOTE:
				   double shouldn't occour by design. But they do. So we do
				   an quick, simple and dirty workaround here and just avoid
				   inserting them. Finally we check if the last point is the
				   same as the first, in that case we remove it, too.
				   Reason for double points:
				   this seem to happen because sometimes endborder/beginborder
				   is >= 0 even if the corresponding points are not on the border.
				   so the coast lines are treated as coast, not as part of an
				   island -> end of first cl is equal to begin of next ->
				   double points result.
				*/
				const segcl& cl = segcls[current];
				cm.coastlines[cl.mapclnr].create_points(ce.points, cl.begint, cl.endt, detail);
				unsigned next = cl.next;
			cout << "current="<<current<<" next="<<next<<"\n";
				cl_handled[current] = true;
			//if(next==0xffffffff)return;
			//if(current==next)break;
				// insert corners if needed
				int ed = cl.endborder, bg = segcls[next].beginborder;
				if (ed >= 0 && bg >= 0) {
					if (ed == bg) {
						if (dist_to_corner(ed, cl.endp, cm.segw_real) < dist_to_corner(bg, segcls[next].beginp, cm.segw_real))
							bg += 4;
					} else if (bg < ed) {
							bg += 4;
					}
					for (int j = ed; j < bg; ++j) {
						int k = j % 4;//fixme: offset von segment fehlt!!!!!
						if (k == 0) ce.push_back_point(roff + vector2f(cm.segw_real, 0));
						else if (k == 1) ce.push_back_point(roff + vector2f(cm.segw_real, cm.segw_real));
						else if (k == 2) ce.push_back_point(roff + vector2f(0, cm.segw_real));
						else ce.push_back_point(roff);
					}
				}
				current = next;
			} while (current != i);
		cout << "area found!\n";

			//more a quick hack, it shouldn't be necessary, fixme!
			if (ce.points.back().square_distance(ce.points.front()) < 1.0f)
				ce.points.pop_back();
				

//detect double points, debug code, not needed any longer
//fixme: one time on the new map there are double points (d==0), but there are 8 triangulation faults
float mdd=1e30;
for(unsigned m=0;m<ce.points.size();++m){
float d=ce.points[m].distance(ce.points[(m+1)%ce.points.size()]);
if(d<mdd)mdd=d;
if(d<gmd)gmd=d;
if(d <= 0.01f)cout<<"fault: "<<d<<","<<m<<","<<(m+1)%ce.points.size()<<"\n";
//assert(d > 0.01f);
}


			ce.indices = triangulate::compute(ce.points);
			pointcache.push_back(ce);
		}
	}
}



void coastsegment::draw_as_map(const class coastmap& cm, int x, int y, const vector2f& roff, int detail) const
{
//cout<<"segment draw " << x << "," << y << " dtl " << detail << " tp " << type << "\n";
	if (type == 1) {
		glBegin(GL_QUADS);
		glTexCoord2d(roff.x, roff.y);
		glVertex2d(roff.x, roff.y);
		glTexCoord2d(roff.x+cm.segw_real, roff.y);
		glVertex2d(roff.x+cm.segw_real, roff.y);
		glTexCoord2f(roff.x+cm.segw_real, roff.y+cm.segw_real);
		glVertex2d(roff.x+cm.segw_real, roff.y+cm.segw_real);
		glTexCoord2f(roff.x, roff.y+cm.segw_real);
		glVertex2d(roff.x, roff.y+cm.segw_real);
		glEnd();
	} else if (type > 1) {
		//fixme: disable, causes only bugs and crashes for now :-(
		//generate_point_cache(cm, roff, detail);
	
		glBegin(GL_TRIANGLES);
		for (vector<cacheentry>::const_iterator cit = pointcache.begin(); cit != pointcache.end(); ++cit) {
			for (vector<unsigned>::const_iterator tit = cit->indices.begin(); tit != cit->indices.end(); ++tit) {
				const vector2f& v = cit->points[*tit];
				glTexCoord2f(v.x, v.y);
				glVertex2f(v.x, v.y);
			}
		}
		glEnd();
#if 0
		// test
		for (unsigned i = 0; i < segcls.cm.segw_real(); ++i) {
			segcls[i].draw_as_map(off, cm.segw_real, t, ts, detail);
		}
#endif		
	}
}



void coastsegment::render(const vector2& p, int detail) const
{
/*
	if (type > 1) {
		for (vector<coastline>::const_iterator it = coastlines.begin(); it != coastlines.end(); ++it) {
			it->render(p, detail);
		}
	}
*/
}



float coastsegment::dist_to_corner(int b, const vector2f& p, float segw)
{
	float dist = 1e30;
	if (b == 0) dist = segw - p.x;
	else if (b == 1) dist = segw - p.y;
	else if (b == 2) dist = p.x;
	else if (b == 3) dist = p.y;
	return dist;
}



float coastsegment::compute_border_dist(int b0, const vector2f& p0, int b1, const vector2f& p1, float segw) const
{
	float dist0 = dist_to_corner(b0, p0, segw), dist1 = dist_to_corner(b1, p1, segw);
	if (b0 == b1 && dist1 > dist0) b1 += 4;
	else if (b1 < b0) b1 += 4;
	float fulldist = dist0;
	for (int i = b0; i < b1; ++i)
		fulldist += segw;
	fulldist -= dist1;
	return fulldist;
}



unsigned coastsegment::get_successor_for_cl(unsigned cln, float segw) const
{
//fixme: this function sometimes returns shit.
	float mindist = 1e30;
	unsigned next = 0xffffffff;
//coastlines[cln].debug_print(cout);
//cout << " succ?\n";
	for (unsigned i = 0; i < segcls.size(); ++i) {
//coastlines[i].debug_print(cout);
		// i's successor can be i itself and may already have been handled (i itself!)
		const segcl& scl0 = segcls[cln];
		const segcl& scl1 = segcls[i];
		// compare for distance, 5 cases
		if (scl0.endborder < 0 && scl1.beginborder < 0) {	// part of an island
			float dist = scl0.endp.distance(scl1.beginp);
			if (dist < mindist) {
				mindist = dist;
				next = i;
			}
		} else if (scl0.endborder >= 0 && scl1.beginborder >= 0) {
			float dist = compute_border_dist(scl0.endborder, scl0.endp, scl1.beginborder, scl1.beginp, segw);
			if (dist < mindist) {
				mindist = dist;
				next = i;
			}
		} // else not compareable
	}
//	assert(next != 0xffffffff);
//cout << "next was " << next << "\n";	
	return next;
}









/*
2004/05/17
idea.
Store map as b/w png image.
Create coastlines from pixel border at runtime.
Create coast line with bsplines on the fly from border.
#Vertices = 2^detaillevel.
Compute vertices with binary subdivision: at t=0,1
then at t=0.5, later at t=0.25 and 0.75, t=0.125,0.375,0.625,0.875 etc.
would be fast enough for display at runtime. OK
Locally increased detail is possible, e.g. subdivide that part of a coastline TODO
that is inside the segment to a certain amount (5-10km/line) and subdivide
it more near the player (measure distance to line segment)

For each segment store which coastlines are inside with start t and end t
(t along the whole coastline). OK
When drawing multiple sectors unite the coastline lists of several sectors
(e.g. cl a is in segm 1 with t=[0.2...0.4] and cl a is also in segm2 with
t=[0.4...0.6] -> draw cl a with t=[0.2...0.6])
That way we have seamless coastlines. TODO

The map takes little memory on disk, no map compiler needed (or at least not that
time consuming one), much advantages

coastmap should move from user_interface to game because it's not only stored
for displaying purposes but should influence the game!
*/





//
// coastmap functions
//

unsigned coastmap::find_seg_for_point(const vector2i& p) const
{
	return (p.x/pixels_per_seg)+(p.y/pixels_per_seg)*segsx;
}



Uint8& coastmap::mapf(int cx, int cy)
{
	cx = clamp_zero(cx);
	cy = clamp_zero(cy);
	cx = mapw - 1 - clamp_zero(mapw - 1 - cx);
	cy = maph - 1 - clamp_zero(maph - 1 - cy);
	return themap[(maph-1-cy)*mapw+cx];
}



bool coastmap::find_begin_of_coastline(int& x, int& y)
{
	bool cyclic = false;
	int sx = x, sy = y, j2 = 0;
	while (true) {
		// compute next x,y
		int j = 0;
		char mv[4];
		for (j = 0; j < 4; ++j) {
			mv[j] = mapf(x+dmx[j], y+dmy[j]) & 0x7f;
		}
		if (mv[0] == 0 && mv[2] == 0 && mv[1] > 0 && mv[3] > 0) {
			j = (j2 + 1) % 4;
		} else if (mv[0] > 0 && mv[2] > 0 && mv[1] == 0 && mv[3] == 0) {
			j = (j2 + 1) % 4;
		} else {
			for (j = 0; j < 4; ++j) {
				if (mv[j] > 0 && mv[(j+1)%4] == 0) break;
			}
			assert(j < 4);
		}
		j2 = j;
		int nx = x + dx[j];
		int ny = y + dy[j];
		if (nx < 0 || ny < 0 || nx >= int(mapw) || ny >= int(maph))	// border reached
			break;
		x = nx;
		y = ny;

		if (sx == x && sy == y) {
			cyclic = true;
			break;	// island found
		}
	}
	return cyclic;
}



bool coastmap::find_coastline(int x, int y, vector<vector2i>& points, bool& cyclic, int& beginborder,
			      int& endborder)	// returns true if cl is valid
{
	// run backward at the coastline until we reach the border or round an island.
	// start there creating the coastline. this avoids coastlines that can never be seen.
	// In reality: north pole, ice, America to the west, Asia/africa to the east.

	assert((mapf(x, y) & 0x80) == 0);
	
	cyclic = find_begin_of_coastline(x, y);

	beginborder = -1;	
	if (!cyclic) {
		if (x == 0) beginborder = 3;
		else if (y == 0) beginborder = 0;
		else if (x == int(mapw)-1) beginborder = 1;
		else if (y == int(maph)-1) beginborder = 2;
	}
	
	int sx = x, sy = y, j2 = 0, lastj = -1, turncount = 0;
	while (true) {
		// store x,y
		points.push_back(vector2i(x, y));

		assert(x>=0&&y>=0&&x<int(mapw)&&y<int(maph));
		mapf(x, y) |= 0x80;

		// compute next x,y	
		int j = 0;
		char mv[4];
		for (j = 0; j < 4; ++j) {
			mv[j] = mapf(x+dmx[j], y+dmy[j]) & 0x7f;
		}
		if (mv[0] == 0 && mv[2] == 0 && mv[1] > 0 && mv[3] > 0) {
			j = (j2 + 3) % 4;
		} else if (mv[0] > 0 && mv[2] > 0 && mv[1] == 0 && mv[3] == 0) {
			j = (j2 + 3) % 4;
		} else {
			for (j = 0; j < 4; ++j) {
				if (mv[j] == 0 && mv[(j+1)%4] > 0) break;
			}
			assert(j < 4);
		}
		j2 = j;

		if (lastj != -1 && cyclic) {
			int jd = lastj-j;
			if (jd < 0) jd += 4;
			if (jd == 3) jd = -1;
			//if(jd<-1||jd>1)cout<<"j="<<j<<" lastj="<<lastj<<"\n";
			assert(jd>=-1&&jd<=1);
			turncount += jd;
		}
		lastj = j;
		
		int nx = x + dx[j];
		int ny = y + dy[j];
		if (nx < 0 || ny < 0 || nx >= int(mapw) || ny >= int(maph))	// border reached
			break;
		x = nx;
		y = ny;
		if (sx == x && sy == y) {
			break;	// island found
		}
	}
	
	endborder = -1;
	if (!cyclic) {
		if (x == 0) endborder = 3;
		else if (y == 0) endborder = 0;
		else if (x == int(mapw)-1) endborder = 1;
		else if (y == int(maph)-1) endborder = 2;
	}

	return turncount <= 0;
}



void coastmap::divide_and_distribute_cl(const coastline& cl, unsigned clnr, const vector<vector2i>& points)
{
	coastsegment::segcl scl;
	scl.mapclnr = clnr;
	scl.cyclic = false;
	scl.next = -1;

	// divide coastline at segment borders
	// find segment that first point is into
	int sx = points.front().x / pixels_per_seg;
	int sy = points.front().y / pixels_per_seg;
	int seg = sy*segsx+sx;
	vector2f segoff(sx * segw_real + realoffset.x, sy * segw_real + realoffset.y);

	scl.begint = 0;
	scl.beginp = cl.curve.value(0);
	scl.beginborder = cl.beginborder;
	
	bool sameseg = true;
	// fixme: avoid just one line of an island/coast to get distributed to another segment.
	// thus treat borders as part of current segment, for both sides!
	// fixme: segment of integer position point[x] doesn't need to be the same
	// that the bspline point cl.curve.value(x/(points.size()-1)) is in!!!!
	// if that leads to problems or bugs is unclear!!!!!!
	for (unsigned i = 1; i < points.size(); ++i) {
		int csx = points[i].x / pixels_per_seg;
		int csy = points[i].y / pixels_per_seg;
		int cseg = csy*segsx+csx;
		if (seg != cseg) {
			sameseg = false;

			float t0 = float(i-1)/float(points.size()-1);
			float t1 = float(i)/float(points.size()-1);
/*
		cout<<"frage werte ab fuer t0="<<t0<<" und t1="<<t1<<"\n";
		if(points.size()==2){
			cout<<points[0]<<"\n";
			cout<<points[1]<<"\n";
			cout<<cl.curve.control_points().size()<<"\n";
			cout<<cl.curve.control_points()[0]<<"\n";
			cout<<cl.curve.control_points()[1]<<"\n";
			cout<<"sind sie gleich?"<<((cl.curve.control_points()[0])==(cl.curve.control_points()[1]))<<"\n";
		}
*/
			vector2f b = cl.curve.value(t0);
			vector2f d = cl.curve.value(t1) - b;
//			vector2f b2 = cl.curve.value(t1);
//			vector2f d = b2 - b;

			// fixme: what is if the next segment is right AND down (line crosses corner)?
			
			// find t so that b + t * d is on segment border between seg and cseg
			float t;
			int border = -1;
			if (cseg < seg) {	// border is left or top
				if (cseg == seg-1) {	// left
					t = (segoff.x - b.x) / d.x;
					border = 3;
				} else {		// top
					t = (segoff.y - b.y) / d.y;
					border = 0;
				}
			} else {		// border is right or bottom
				if (cseg == seg+1) {	// right
					t = (segoff.x + segw_real - b.x) / d.x;
					border = 1;
				} else {		// bottom
					t = (segoff.y + segw_real - b.y) / d.y;
					border = 2;
				}
			}
			
			// t should be between 0 and 1, but this is sometimes wrong! fixme.
			// see comment above, points[i] and points[i+1] may be in different
			// segments, but the bspline results may be in the same segment.
			// to find t here we would have to approximate it
			
			vector2f borderp = b + d * t;
			float ct = t0 + (t1 - t0) * t;
//cout << "t0 " << t0 << " t1 " << t1 << " t " << t << "\n";
//cout << "b is " << b << " d is " << d << " und b2 ist " << b2 << "\n";
			scl.endt = ct;
			scl.endp = borderp;
			scl.endborder = border;

			coastsegments[seg].segcls.push_back(scl);
			sx = csx;
			sy = csy;
			seg = cseg;
			segoff = vector2f(sx * segw_real + realoffset.x, sy * segw_real + realoffset.y);

			scl.begint = ct;
			scl.beginp = borderp;
			scl.beginborder = (border + 2) % 4;
		}
	}

	scl.endt = 1;
	scl.endp = cl.curve.value(1);
	scl.endborder = cl.endborder;

	scl.cyclic = sameseg;	// we have an island complete in one segment

	//fixme test that with new code!	
//	if (scl.cyclic) assert(segcl.beginborder==-1 && segcl.endborder==-1);

	coastsegments[seg].segcls.push_back(scl);
}



void coastmap::process_coastline(int x, int y)
{
	assert ((mapf(x, y) & 0x80) == 0);
	
	// find coastline, avoid "lakes", (inverse of islands), because the triangulation will fault there
	vector<vector2i> points;
	bool cyclic;
	int beginborder, endborder;
	bool valid = find_coastline(x, y, points, cyclic, beginborder, endborder);
	
	if (!valid) return;	// skip

	// create bspline curve
	vector<vector2f> tmp;
	tmp.reserve(points.size());
	for (unsigned i = 0; i < points.size(); ++i)
		tmp.push_back(vector2f(points[i].x * pixelw_real + realoffset.x, points[i].y * pixelw_real + realoffset.y));

//	cout << "points.size()="<<points.size()<<" cyclic?"<<cyclic<<"\n";
	if (cyclic) {
		// close polygon and make sure the first and last line are linear dependent
		// fixme!!!!!! tmp is now larger than points!!!! could this give errors?!
		// YES, IT DOES!!! push back points[0] twice to points as solution?!
		// or don't use points inside divide_and_dist, just tmp! fixme
		assert(tmp.size()>2);
		vector2f p0 = tmp.front();
		vector2f p2 = tmp.back();
		vector2f p1 = (p0 + p1) * 0.5;
		//tmp.push_back(p1);
		//tmp.push_back(p0);
	}

	unsigned n = tmp.size() - 1;
	if (n > 16) n = 16;
	coastline result(n, tmp, beginborder, endborder);
	divide_and_distribute_cl(result, coastlines.size(), points);

	coastlines.push_back(result);
}



void coastmap::process_segment(int sx, int sy)
{
	coastsegment& cs = coastsegments[sy*segsx+sx];
	if (cs.segcls.size() == 0) {	// no coastlines in segment.
		// segment is fully land or sea. determine what it is
		if (mapf(sx*pixels_per_seg + pixels_per_seg/2, sy*pixels_per_seg + pixels_per_seg/2) & 0x7f) {
			cs.type = 1;
		} else {
			cs.type = 0;
		}
	} else {		// there are coastlines in segment
		cs.type = 2;

		// compute cl.next info
		unsigned ncl = cs.segcls.size();
		vector<bool> handledcl(ncl, false);
		for (unsigned i = 0; i < ncl; ++i) {
			if (handledcl[i]) continue;
			unsigned current = i;
			do {
				unsigned next = cs.get_successor_for_cl(current, segw_real);
				cs.segcls[current].next = next;
				handledcl[current] = true;
			} while (current != i);
		}
	}
}



// load from xml description file
coastmap::coastmap(const string& filename)
{
	TiXmlDocument doc(filename);
	doc.LoadFile();
	TiXmlElement* root = doc.FirstChildElement("dftd-map");
	system::sys().myassert(root != 0, string("coastmap: no root element found in ") + filename);
	TiXmlElement* etopology = root->FirstChildElement("topology");
	system::sys().myassert(etopology != 0, string("coastmap: no topology node found in ") + filename);
	const char* img = etopology->Attribute("image");
	system::sys().myassert(img != 0, string("coastmap: no image attribute found in ") + filename);
	realwidth = 0;
	etopology->Attribute("realwidth", &realwidth);
	system::sys().myassert(realwidth != 0, string("coastmap: realwidth not given or zero in ") + filename);
	etopology->Attribute("realoffsetx", &realoffset.x);
	etopology->Attribute("realoffsety", &realoffset.y);
	TiXmlElement* ecities = root->FirstChildElement("cities");
	if (ecities) {
		TiXmlElement* ecity = ecities->FirstChildElement("city");
		for ( ; ecity != 0; ecity = ecity->NextSiblingElement()) {
			// parse name, posx, posy  fixme
		}
	}

	SDL_Surface* surf = IMG_Load((get_map_dir() + img).c_str());
	system::sys().myassert(surf != 0, string("coastmap: error loading image ") + img + string(" referenced in file ") + filename);

	mapw = surf->w;
	maph = surf->h;
	pixelw_real = realwidth/mapw;
	realheight = maph*realwidth/mapw;
	pixels_per_seg = 1 << unsigned(ceil(log2(60000/pixelw_real)));
	segsx = mapw/pixels_per_seg;
	segsy = maph/pixels_per_seg;
	segw_real = pixelw_real * pixels_per_seg;
	system::sys().myassert((segsx*pixels_per_seg == mapw) && (segsy*pixels_per_seg == maph), string("coastmap: map size must be integer multiple of segment size, in") + filename);

	themap.resize(mapw*maph);

	SDL_LockSurface(surf);
	system::sys().myassert(surf->format->BytesPerPixel == 1 && surf->format->palette != 0 && surf->format->palette->ncolors == 2, string("coastmap: image is no black/white 1bpp paletted image, in ") + filename);

	Uint8* offset = (Uint8*)(surf->pixels);
	for (int yy = 0; yy < int(maph); yy++) {
		for (int xx = 0; xx < int(mapw); ++xx) {
			Uint8 c = (*offset++);
			themap[yy*mapw+xx] = c ? 1 : 0;
		}
		offset += surf->pitch - mapw;
	}

	SDL_UnlockSurface(surf);
	SDL_FreeSurface(surf);

	// they are filled in by process_coastline
	coastsegments.resize(segsx*segsy);

	// find coastlines
	for (int yy = 0; yy < int(maph); ++yy) {
		for (int xx = 0; xx < int(mapw); ++xx) {
			if (mapf(xx, yy) & 0x80) continue;
			char m0 = mapf(xx+dmx[0], yy+dmy[0]) & 0x7f;
			char m1 = mapf(xx+dmx[1], yy+dmy[1]) & 0x7f;
			char m2 = mapf(xx+dmx[2], yy+dmy[2]) & 0x7f;
			char m3 = mapf(xx+dmx[3], yy+dmy[3]) & 0x7f;
			char m4 = m0 & m1 & m2 & m3;
			char m5 = m0 | m1 | m2 | m3;
			if (m4 == 0 && m5 == 1) {
				process_coastline(xx, yy);
			}
		}
	}

	// find coastsegment type
	for (unsigned yy = 0; yy < segsy; ++yy) {
		for (unsigned xx = 0; xx < segsx; ++xx) {
			process_segment(xx, yy);
		}
	}

/*
	// read cities, fixme move to coastmap
	parser cityfile(get_map_dir() + "cities.txt");
	while (!cityfile.is_empty()) {
		bool xneg = cityfile.type() == TKN_MINUS;
		cityfile.consume();
		int xd = cityfile.parse_number();
		cityfile.parse(TKN_COMMA);
		int xm = cityfile.parse_number();
		cityfile.parse(TKN_COMMA);
		bool yneg = cityfile.type() == TKN_MINUS;
		cityfile.consume();
		int yd = cityfile.parse_number();
		cityfile.parse(TKN_COMMA);
		int ym = cityfile.parse_number();
		cityfile.parse(TKN_COMMA);
		string n = cityfile.parse_string();
		double x, y;
		sea_object::degrees2meters(xneg, xd, xm, yneg, yd, ym, x, y);
		cities.push_back(make_pair(vector2(x, y), n));
	}
*/
}



void coastmap::draw_as_map(const vector2& droff, double mapzoom, int detail) const
{
	int x, y, w, h;
//cout << "mapzoom pix/m = " << mapzoom << " segwreal " << segw_real << "\n";
	w = int(ceil((1024/mapzoom)/segw_real)) +2;
	h = int(ceil((768/mapzoom)/segw_real)) +2;	// fixme: use 640 and translate map y - 64
//cout << " w " << w << " h " << h << " waren's mal.\n";
	x = int(floor((droff.x - realoffset.x)/segw_real)) - w/2;
	y = int(floor((droff.y - realoffset.y)/segw_real)) - h/2;
	if (x < 0) {
		w += x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		y = 0;
	}
	if (x + w > int(segsx)) {
		w = int(segsx) - x;
	}
	if (y + h > int(segsy)) {
		h = int(segsy) - y;
	}
//cout<<"draw map   segsx " << segsx << " segsy " << segsy << " x " << x << " y " << y << " w " << w << " h " << h << "\n";

	atlanticmap->set_gl_texture();
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glTranslated(0.0, 1.0, 0.0);
	glScaled(1.0/realwidth, -1.0/realheight, 1.0);
	glTranslated(-realoffset.x, -realoffset.y, 0.0);
	glMatrixMode(GL_MODELVIEW);
	for (int yy = y; yy < y + h; ++yy) {
		for (int xx = x; xx < x + w; ++xx) {
			vector2f offset(realoffset.x + segw_real * xx, realoffset.y + segw_real * yy);
		//cout<<"drawing segment " << xx << "," << yy << "\n";			
			coastsegments[yy*segsx+xx].draw_as_map(*this, xx, yy, offset, detail);
		}
	}

	// testing
	for (unsigned i = 0; i < coastlines.size(); ++i)
		coastlines[i].draw_as_map();

	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

/*
	// draw cities, fixme move to coastmap
	for (list<pair<vector2, string> >::const_iterator it = cities.begin(); it != cities.end(); ++it) {
		sys.no_tex();
		draw_square_mark(gm, it->first, -offset, color(255, 0, 0));
		vector2 pos = (it->first - offset) * mapzoom;
		font_arial->print(int(512 + pos.x), int(384 - pos.y), it->second);
	}
*/
}



void coastmap::render(const vector2& p, int detail, bool withterraintop) const
{
	vector2 pnew = p - realoffset;

	// determine which coast segment can be seen (at most 4)
	// fixme
	double rsegw = pixelw_real * pixels_per_seg;
	int moffx = int(pnew.x/rsegw);
	int moffy = int(pnew.y/rsegw);

	if (moffx >= 0 && moffy >= 0 && moffx < int(segsx) && moffy < int(segsy))	
		coastsegments[moffy*segsx+moffx].render(pnew - vector2(moffx*rsegw, moffy*rsegw), detail);
}
