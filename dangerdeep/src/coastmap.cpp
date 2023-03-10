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

// a coastmap
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "binstream.h"
#include "coastmap.h"
#include "datadirs.h"
#include "global_data.h"
#include "model.h"
#include "oglext/OglExt.h"
#include "primitives.h"
#include "system_interface.h"
#include "texture.h"
#include "triangulate.h"
#include "xml.h"

#include <SDL_image.h>
#include <fstream>
#include <list>
#include <memory>
#include <vector>
using namespace std;

// fixme: replace this
#ifdef DEBUG
//#define ASSERT(a,fmt,...) do{if(!(a)){char __tmp[256];vsprintf(__tmp, fmt,
//__VA_ARGS__);THROW(error, __tmp);}}while(0)
#define ASSERT(a, fmt, ...)
#else
#define ASSERT(a, fmt, ...)
#endif

const unsigned BSPLINE_SMOOTH_FACTOR = 16; // 3;//16;	// should be 3...16
const double BSPLINE_DETAIL = 8.0; // 1.0;//4.0;            // should be 1.0...x
const unsigned SEGSCALE =
    65535; // 2^16-1 so that per segment coordinates fit in a ushort value.

// order (0-3):
// 32
// 01
const int coastmap::dmx[4] = {-1, 0, 0, -1};
const int coastmap::dmy[4] = {-1, -1, 0, 0};
// order: left, down, right, up
const int coastmap::dx[4] = {0, 1, 0, -1};
const int coastmap::dy[4] = {-1, 0, 1, 0};

bool coastmap::patternprocessok[16] = {
    false,
    true,
    true,
    true,
    true,
    false,
    true,
    true,
    true,
    true,
    false,
    true,
    true,
    true,
    true,
    false};

/*
segcls are connected in ccw order.
vertices are stored in ccw order.
this means when iterating along the coast, land is left and sea is right (ccw).
lakes are not allowed (when they're completely inside a seg, because the
triangulation failes there).
borders are stored from 0-3 in ccw order: bottom,right,top,left.

the following patterns are possible when searching coastlines (0,15 illegal)

 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
..  ..  ..  ..  .#  .#  .#  .#  #.  #.  #.  #.  ##  ##  ##  ##  32
..  #.  .#  ##  ..  #.  .#  ##  ..  #.  .#  ##  ..  #.  .#  ##  01

*/

// -1 illegal,0-3 down,right,up,left:    0  1  2  3  4  5  6  7  8  9 10 11 12
// 13 14 15
const int coastmap::runlandleft[16] =
    {-1, 3, 0, 3, 1, -1, 0, 3, 2, 2, -1, 2, 1, 1, 0, -1};
const int coastmap::runlandright[16] =
    {-1, 0, 1, 1, 2, -1, 2, 2, 3, 0, -1, 1, 3, 0, 3, -1};

/*
fixme: 2005/05/05
remaining bugs:
1) triangulation faults at one island near Greece, in the Aegean. This is NOT
because the triangulation algorithm is not perfect (it could be better
though...) This is the only segment where illegal .next values are reported.
   Reason is known, see compute_successor!
   That happens when a coastline touches a segment border. The coastline then is
splitted in two parts, that aren't connected correctly by compute_successor.
This is now avoided, segcls are only splitted if two parts are NOT in the same
segment. At that island however, the coastline STARTS at the point touching the
border. 2) many double points are erased. Why are they created? this shouldn't
happen... Maybe because one coastline is divided into many small 1-line segcls.
   Yes! with the fix that avoids such divisions, no double points need to get
erased, except in one segment SW of iceland (reason not found, situation not
analyzed)
*/

void coastsegment::segcl::push_back_point(const coastsegment::segpos& sp)
{
    // fixme: avoid double points here, maybe assert that
    if (points.empty() || !(points.back() == sp))
    {
        points.push_back(sp);
    }
}

// the least distance between two points is around 0.014, fixme test again
void coastsegment::cacheentry::push_back_point(const vector2& p)
{
    if (points.size() > 0)
    {
        double d = points.back().square_distance(p);
        if (d < 1.0f)
        {
            return; // fixme test hack
        }
        ASSERT(
            d >= 1.0f,
            "error: points are too close %f    %f %f  %f %f",
            d,
            points.back().x,
            points.back().y,
            p.x,
            p.y);
    }
    points.push_back(p);
}

void coastsegment::generate_point_cache(
    const class coastmap& cm,
    int x,
    int y,
    int detail) const
{
    if (type > 1)
    {
        // cache generated and unchanged?
        if (pointcache.size() > 0 && pointcachedetail == detail)
        {
            return;
        }

        //	cout << "creating cache entry for segment " << roff << " empty? " <<
        //(pointcache.size() > 0) << " cached detail " << pointcachedetail << "
        // new detail " << detail << "\n";
        // invalidate cache (detail changed or initial generation)
        pointcachedetail = detail;
        pointcache.clear();

        unsigned nrcl = segcls.size();
        std::vector<bool> cl_handled(nrcl, false);
        for (unsigned i = 0; i < nrcl; ++i)
        {
            if (cl_handled[i])
            {
                continue;
            }

            cacheentry ce;

            // printf("handle segcl %i!!!\n",i);

            // find area
            unsigned current = i;
            do
            {
                //				cout << "segpos: x " << x << " y " << y << "\n";
                //				cout << "current is : " << current << "\n";
                //	segcls[current].print();
                ASSERT(!cl_handled[current], "illegal .next values!", 0);

                const segcl& cl = segcls[current];
                ce.points.reserve(ce.points.size() + cl.points.size());
                double sc = cm.segw_real / SEGSCALE;
                for (auto point : cl.points)
                {
                    // avoid double points here... fixme. they're removed later.
                    // see below
                    ce.points.push_back(vector2(point.x, point.y) * sc);
                }
                int next = cl.next;
                //				cout << "startpos " << cl.beginpos << " endpos:
                //"
                //<< cl.endpos << " next " << next << " next startpos: " <<
                // segcls[next].beginpos << "\n"; 				cout <<
                // "current="<<current<<" next="<<next<<"\n";
                cl_handled[current] = true;
                ASSERT(next != -1, "next unset?");
                // if(current==next)break;
                // insert corners if needed
                if (!cl.cyclic)
                {
                    int b0 = cl.endpos, b1 = segcls[next].beginpos;
                    //					cout << "fill: ed " << b0 << " bg " <<
                    // b1
                    //<<
                    //"\n";
                    if (b1 < b0)
                    {
                        b1 += 4 * SEGSCALE;
                    }
                    b0 = (b0 + SEGSCALE - 1) / SEGSCALE;
                    b1 = b1 / SEGSCALE;
                    //					cout << "fill2: ed " << b0 << " bg " <<
                    // b1
                    //<<
                    //"\n";
                    for (int j = b0; j <= b1; ++j)
                    {
                        int k = j % 4;
                        // push back destination point of edge (0-3:
                        // br,tr,tl,bl)
                        if (k == 0)
                        {
                            ce.push_back_point(vector2());
                        }
                        else if (k == 1)
                        {
                            ce.push_back_point(vector2(cm.segw_real, 0));
                        }
                        else if (k == 2)
                        {
                            ce.push_back_point(
                                vector2(cm.segw_real, cm.segw_real));
                        }
                        else
                        { /*if (k == 3)*/
                            ce.push_back_point(vector2(0, cm.segw_real));
                        }
                    }
                }
                //				printf("cyclic? %u next %i\n",cl.cyclic,next);
                current = unsigned(next);
            } while (current != i);

            //			cout << "compute triang for segment " << this << "\n";

            //			cout << "segment no: " << (this - &cm.coastsegments[0])
            //<<
            //"\n";
            unsigned dbl = 0;
            for (unsigned i = 0; i + 1 < ce.points.size(); ++i)
            {
                unsigned j = i + 1;
                if (ce.points[j].square_distance(ce.points[i]) < 0.1f)
                {
                    ce.points.erase(ce.points.begin() + j);
                    --i;
                    ++dbl;
                }
            }
            if (dbl > 0)
            {
                cout << "erased " << dbl << " double points!, seg " << x << ","
                     << y << "\n";
            }
            // remove last point that coincides with first point for islands.
            if (ce.points.back().square_distance(ce.points.front()) < 0.1f)
            {
                ce.points.pop_back();
            }

            //			printf("triangulating seg %i %i\n",x,y);
            ce.indices = triangulate::compute(ce.points);
            pointcache.push_back(ce);
        }
    }
}

void coastsegment::draw_as_map(
    const class coastmap& cm,
    int x,
    int y,
    int detail) const
{
    // cout<<"segment draw " << x << "," << y << " dtl " << detail << " tp " <<
    // type << "\n";
    // fixme, use display lists here
    if (type == 1)
    {
        vector2f tc0 = cm.segcoord_to_texc(x, y);
        vector2f tc1 = cm.segcoord_to_texc(x + 1, y + 1);
        primitives::textured_quad(
            vector2f(0, 0),
            vector2f(cm.segw_real, cm.segw_real),
            *atlanticmap,
            tc0,
            tc1)
            .render();
    }
    else if (type > 1)
    {
        vector2f tc0 = cm.segcoord_to_texc(x, y);
        vector2f tc1 = cm.segcoord_to_texc(x + 1, y + 1);
        generate_point_cache(cm, x, y, detail);
        unsigned nrv = 0;
        for (const auto& cit : pointcache)
        {
            nrv += cit.indices.size();
        }
        primitives tris(GL_TRIANGLES, nrv, color::white(), *atlanticmap);
        nrv = 0;
        for (const auto& cit : pointcache)
        {
            for (auto tit = cit.indices.begin(); tit != cit.indices.end();
                 ++tit)
            {
                const vector2& v = cit.points[*tit];
                float ex         = v.x / cm.segw_real;
                float ey         = v.y / cm.segw_real;
                float ax         = 1.0f - ex;
                float ay         = 1.0f - ey;
                vector2f tc(tc0.x * ax + tc1.x * ex, tc0.y * ay + tc1.y * ey);
                tris.vertices[nrv].x = v.x;
                tris.vertices[nrv].y = v.y;
                tris.texcoords[nrv]  = tc;
                ++nrv;
            }
        }
        tris.render();
    }
}

// return position on segment border
auto coastmap::borderpos(const coastsegment::segpos& p) const -> int
{
    if (p.y == 0)
    {
        return p.x;
    }
    else if (p.x == SEGSCALE)
    {
        return SEGSCALE + p.y;
    }
    else if (p.y == SEGSCALE)
    {
        return 2 * SEGSCALE + SEGSCALE - p.x;
    }
    else if (p.x == 0)
    {
        return 3 * SEGSCALE + SEGSCALE - p.y;
    }
    else
    {
        return -1;
    }
}

void coastsegment::segcl::print() const
{
    /*
      cout << "segcl:\nmapclnr: " << mapclnr << " begint " << begint << " endt "
      << endt
      << "\nbeginp: " << beginp << " endp: " << endp
      << "\nbeginborder: " << beginborder << " endborder " << endborder
      << "\ncyclic? " << cyclic << " next: " << next << "\n";
    */
    cout << "segcl:\n"
         << points.size() << " points, beginpos " << beginpos << " beginb "
         << " beginb " << ((beginpos == -1) ? -1 : beginpos / int(SEGSCALE))
         << " endpos " << endpos << " endb "
         << ((endpos == -1) ? -1 : endpos / int(SEGSCALE)) << " next " << next
         << " cyclic? " << cyclic << " nrpts " << points.size() << " 1st pt "
         << points.front() << ",back " << points.back() << "\n";
}

void coastsegment::compute_successor_for_cl(unsigned cln)
{
    segcl& scl0 = segcls[cln];

    // only if not already set (already set for islands contained in segments)
    if (scl0.next == -1)
    {
        // ASSERT(scl0.beginpos >= 0, "paranoia bp1");
        // ASSERT(scl0.endpos >= 0, "paranoia ep1");

        // when we use <= below this must be enabled. it doesn't help though.
        // if (scl0.endpos == scl0.beginpos) { scl0.next = cln; return; } // fix
        // test hack fixme

        // now loop over all segcls in the segment, find minimal beginpos that
        // is greater than scl0.endpos
        int minbeginpos = 8 * SEGSCALE; // +inf
        for (unsigned i = 0; i < segcls.size(); ++i)
        {
            // i's successor can be i itself, when i enters and leaves the seg
            // without any other cl in between
            const segcl& scl1 = segcls[i];

            // avoid islands, they can't be successors.
            if (scl1.cyclic)
            {
                continue;
            }

            int beginpos = scl1.beginpos;
            // note! use <= not < here, to avoid connecting two segcl's that
            // form one cl which touches the border only (two scl's are
            // generated for this situation) fixme: but with <= many other
            // segcl's fail... If a cl touches the border from the other side,
            // either < or <= will fail!
            //			if(beginpos == scl0.endpos) {cout<<"BEGINPOS " <<
            // beginpos
            //<< " endpos " << scl0.endpos << " nr " << cln << " i " << i <<
            //"\n";}
            if (beginpos < scl0.endpos)
            {
                beginpos += 4 * SEGSCALE;
            }
            if (beginpos < minbeginpos)
            {
                scl0.next   = i;
                minbeginpos = beginpos;
            }
        }

        ASSERT(scl0.next != -1, "no successor found!");
    }
}

void coastsegment::push_back_segcl(const segcl& scl)
{
    if (scl.beginpos < 0)
    {
        //		scl.print();
        ASSERT(scl.cyclic, "begin < -1 but not cyclic");
        ASSERT(scl.endpos < 0, "begin < -1, but not end");
    }
    if (scl.endpos < 0)
    {
        //		scl.print();
        ASSERT(scl.cyclic, "end < -1 but not cyclic");
        ASSERT(scl.beginpos < 0, "end < -1, but not begin");
    }
    if (scl.points.size() >= 2)
    {
        segcls.push_back(scl);
    }
}

//
// coastmap functions
//

auto coastmap::mapf(int cx, int cy) -> uint8_t&
{
    cx = clamp_zero(cx);
    cy = clamp_zero(cy);
    cx = mapw - 1 - clamp_zero(mapw - 1 - cx);
    cy = maph - 1 - clamp_zero(maph - 1 - cy);
    return themap[cy * mapw + cx];
}

auto coastmap::find_begin_of_coastline(int& x, int& y) -> bool
{
    int sx = x, sy = y;
    int lastborder_x = -1, lastborder_y = -1;
    //	cout <<" find beg of cl " << sx << "," << sy << "\n";
    // loop until we step on a map border, with land left of it, or we detect a
    // circle (island or lake)
    int olddir = -1;
    while (true)
    {
        // compute next x,y
        int dir = -1;

        uint8_t pattern = 0;
        for (int j = 0; j < 4; ++j)
        {
            pattern |= (mapf(x + dmx[j], y + dmy[j]) & 0x7f) << j;
        }

        if (olddir == -1)
        {
            ASSERT(
                pattern != 5 && pattern != 10,
                "illegal start pattern! at %i %i",
                x,
                y);
        }

        if (patternprocessok[pattern]
            && (x % pixels_per_seg == 0 || y % pixels_per_seg == 0))
        {
            lastborder_x = x;
            lastborder_y = y;
        }

        // mirrored direction to find_coastline
        if (pattern == 10)
        {
            // check pattern:
            // #.
            // .#
            // direction: only 1,3 valid. 0->1, 2->3
            ASSERT(olddir == 0 || olddir == 2, "olddir illegal 1 (%i)", olddir);
            dir = olddir + 1;
        }
        else if (pattern == 5)
        {
            // check pattern:
            // .#
            // #.
            // direction: only 0,2 valid. 3->0, 1->2
            ASSERT(olddir == 3 || olddir == 1, "olddir illegal 2 (%i)", olddir);
            dir = (olddir + 1) % 4;
        }
        else
        {
            // check other patterns:
            dir = runlandright[pattern];
            ASSERT(dir != -1, "dir illegal 1");
        }
        olddir = dir;

        int nx = x + dx[dir];
        int ny = y + dy[dir];
        // if we left the border, stop search.
        if (nx < 0 || ny < 0 || nx > int(mapw) || ny > int(maph))
        {
            ASSERT(pattern != 5 && pattern != 10, "illegal start pattern!3");
            break;
        }
        x = nx;
        y = ny;
        // cout << "x/y now " << x << "," << y << "\n";
        if (sx == x && sy == y)
        {
            if (lastborder_x != -1)
            {
                x = lastborder_x;
                y = lastborder_y;
            }
            // kann +- 3 sein, wenn knick auf startpunkt
            // ASSERT(turncount == 4 || turncount == -4, "island but turn count
            // != +-4? %i", turncount);
            //			printf("reported island/lake found at %i %i\n",x,y);
            return true; // island found
        }
    }
    return false; // no island/lake, normal cl
}

// returns true if cl is valid
auto coastmap::find_coastline(
    int x,
    int y,
    std::vector<vector2i>& points,
    bool& cyclic) -> bool
{
    // run backward at the coastline until we reach the border or round an
    // island. start there creating the coastline. this avoids coastlines that
    // can never be seen. In reality: north pole, ice, America to the west,
    // Asia/africa to the east. generate points in ccw order, that means land is
    // left, sea is right.

    //	ASSERT((mapf(x, y) & 0x80) == 0);

    cyclic = find_begin_of_coastline(x, y);

    int sx = x, sy = y;
    int olddir = -1, turncount = 0, length = 0;
    while (true)
    {
        // store x,y
        points.emplace_back(x, y);

        // compute next x,y
        int dir = -1;

        uint8_t pattern = 0;
        for (int j = 0; j < 4; ++j)
        {
            uint8_t& c = mapf(x + dmx[j], y + dmy[j]);
            pattern |= (c & 0x7f) << j;
            // mark land as handled
            c |= ((c & 0x7f) << 7);
        }

        if (olddir == -1)
        {
            ASSERT(pattern != 5 && pattern != 10, "illegal start pattern!2");
        }

        if (pattern == 10)
        {
            // check pattern:
            // #.
            // .#
            // direction: only 0,2 valid. 1->0, 3->2
            ASSERT(olddir == 1 || olddir == 3, "olddir illegal 3 (%i)", olddir);
            dir = olddir - 1;
        }
        else if (pattern == 5)
        {
            // check pattern:
            // .#
            // #.
            // direction: only 1,3 valid. 2->1, 0->3
            ASSERT(olddir == 2 || olddir == 0, "olddir illegal 4 (%i)", olddir);
            dir = (olddir + 3) % 4;
        }
        else
        {
            // check other patterns:
            dir = runlandleft[pattern];
            ASSERT(dir != -1, "dir illegal 2");
        }

        // turncount
        if (olddir != -1)
        {
            int t = (dir - olddir + 4) % 4;
            ASSERT(t != 2, "no 180 degree turns allowed!");
            if (t == 3)
            {
                t = -1;
            }
            // positive values are ccw turns.
            turncount += t;
        }
        olddir = dir;

        int nx = x + dx[dir];
        int ny = y + dy[dir];
        //		printf("x %i y %i nx %i ny %i\n",x,y,nx,ny);
        //		printf("pattern %02x%02x\npattern
        //%02x%02x\n",mv[3],mv[2],mv[0],mv[1]);
        if (nx < 0 || ny < 0 || nx > int(mapw) || ny > int(maph))
        { // border reached
            break;
        }
        ++length;
        x = nx;
        y = ny;
        if (sx == x && sy == y)
        {
            break; // island found
        }
    }

    //	cout<<"TURNCOUNT is "<<turncount<<", length is " << length << "\n";
    return (!cyclic) || (turncount > 0);
}

auto coastmap::compute_segment(const vector2i& p0, const vector2i& p1) const
    -> vector2i
{
    vector2i segnum0(p0.x / SEGSCALE, p0.y / SEGSCALE);
    vector2i segoff0(p0.x % SEGSCALE, p0.y % SEGSCALE);
    //	cout << "comp seg: " << segnum0 << " off " << segoff0 << "\n";
    // p0 can be on a corner, on an edge or really inside the segment. handle
    // the three cases
    if (segoff0.x > 0 && segoff0.y > 0)
    {
        // truly inside
        return segnum0;
    }
    else if (segoff0.x == 0 && segoff0.y == 0)
    {
        // on a corner
        // direction to p1 determines segment of p0. Land is left of line p0->p1
        unsigned q = quadrant(p1 - p0);
        switch (q)
        {
            case 0:
            case 7:
                --segnum0.x; // one left
                break;
            case 3:
            case 4:
                --segnum0.y; // one down
                break;
            case 5:
            case 6:
                --segnum0.x; // one left and down
                --segnum0.y;
                break;
                // in other cases (1, 2) segment is the same
        }
        ASSERT(segnum0.x < segsx, "segnum0.x out of bounds");
        ASSERT(segnum0.y < segsy, "segnum0.y out of bounds");
        return segnum0;
    }
    else
    {
        // on an edge, it can be the left or bottom edge
        unsigned q = quadrant(p1 - p0);
        //		cout << "quadrant: " << q << "\n";
        if (segoff0.x == 0)
        {
            // left edge. segment is left of segnum when p1 has relative angle
            // in ]180...360] that means q values of 5,6,7,0
            if (q == 0 || (q >= 5 && q <= 7))
            {
                --segnum0.x;
            }
        }
        else
        {
            // bottom edge. segment is below segnum when p1 has relative angle
            // in ]90...270] that means q values of 3..6
            if (q >= 3 && q <= 6)
            {
                --segnum0.y;
            }
        }
        ASSERT(segnum0.x < segsx, "segnum0.x out of bounds");
        ASSERT(segnum0.y < segsy, "segnum0.y out of bounds");
        return segnum0;
    }
}

void coastmap::divide_and_distribute_cl(
    const std::vector<vector2i>& cl,
    bool clcyclic)
{
    ASSERT(cl.size() >= 2, "div and distri with < 2?");

    coastsegment::segcl scl(global_clnr);

    // divide coastline at segment borders
    // find segment that first point is into
    vector2i p0   = cl[0];
    vector2i segc = compute_segment(p0, cl[1]);
    //	cout << "p0 is " << p0 << " p1 is " << cl[1] << " segc: " << segc <<
    //"\n";
    int segcn       = segc.y * segsx + segc.x;
    vector2i segoff = segc * SEGSCALE; // segment offset
    vector2i segend =
        segoff + vector2i(SEGSCALE, SEGSCALE); // last coordinates IN segment
    vector2i rel = p0 - segoff;
    coastsegment::segpos ps0(static_cast<unsigned short>(rel.x), static_cast<unsigned short>(rel.y));
    scl.push_back_point(ps0);
    scl.beginpos = borderpos(ps0);

    //	cout << "es f??ngt an bei: " << scl.beginpos << "\n";

    // fixme: if there is a segcl that has several points on the same border in
    // the same segment it is divided in many scl's, each containing just one
    // line (two points). It doesn't hurt, though.

    bool sameseg = true;
    // loop over coastline
    //	cout << "new distri!\n";
    //	cout << "p0: " << p0 << " ps0 " << ps0.x << "," << ps0.y << " beginpos "
    //<< scl.beginpos << "\n";
    for (unsigned i = 1; i < cl.size();)
    {
        // fixme: with gcc 4.0 a deadlock occours in this loop!
        // most probably because p1.x is -1e9 former values around +1e6 that
        // means cl[i].x is weird! so the bug is not in this loop as cl[i] is
        // only read here!!!
        // with gcc4.0.1, scons debug=2 it works, maybe a problem of the
        // optimization. compiler bug??? or unstable numerical computation

        //		cout << "i: " << i << "/" << cl.size() << " p0: " << p0 << " p1:
        //"
        //<< cl[i] << "\n";
        // handle line from p0 to cl[i] = p1.
        const vector2i& p1 = cl[i];
        // now p1 can be inside the same segment as p0, on the border of the
        // same segment or in another segment.
        vector2i rel = p1 - segoff;
        if (rel.x >= 0 && rel.y >= 0 && rel.x <= int(SEGSCALE)
            && rel.y <= int(SEGSCALE))
        {
            // inside the same segment or on border
            ps0.x = static_cast<unsigned short>(rel.x);
            ps0.y = static_cast<unsigned short>(rel.y);
            scl.push_back_point(ps0);
            p0 = p1;
            if (rel.x > 0 && rel.y > 0 && rel.x < int(SEGSCALE)
                && rel.y < int(SEGSCALE))
            {
                // really inside the segment, just continue
                ++i;
            }
            else
            {
                // on edge or corner.
                // now switch to new segment if there are lines left
                if (i + 1 < cl.size())
                {
                    segc         = compute_segment(p1, cl[i + 1]);
                    int newsegcn = segc.y * segsx + segc.x;
                    if (newsegcn != segcn)
                    {
                        scl.endpos = borderpos(ps0);
                        ASSERT(scl.endpos != -1, "borderpos check2a");
                        coastsegments[segcn].push_back_segcl(scl);
                        segcn   = newsegcn;
                        sameseg = false;
                        segoff  = segc * SEGSCALE;
                        segend  = segoff + vector2i(SEGSCALE, SEGSCALE);
                        rel     = p1 - segoff;
                        ps0.x   = static_cast<unsigned short>(rel.x);
                        ps0.y   = static_cast<unsigned short>(rel.y);
                        scl     = coastsegment::segcl(global_clnr);
                        scl.push_back_point(ps0);
                        scl.beginpos = borderpos(ps0);
                    }
                }
                else
                {
                    scl.endpos = borderpos(ps0);
                    ASSERT(scl.endpos != -1, "borderpos check2b");
                    coastsegments[segcn].push_back_segcl(scl);
                }
                ++i; // continue from p1 on.
            }
        }
        else
        {
            // p1 is in another segment
            sameseg = false;

            // compute intersection of p0->p1 with segment borders
            vector2i delta = p1 - p0;
            // border coordinates are segoff.x/y and segend.x/y
            double mint = 1e30;
            // find nearest intersection with border along line
            int border = -1;
            if (delta.x > 0)
            {
                // check right border
                double t = double(segend.x - p0.x) / delta.x;
                if (t < mint)
                {
                    mint   = t;
                    border = 1;
                }
            }
            else if (delta.x < 0)
            {
                // check left border
                double t = double(segoff.x - p0.x) / delta.x;
                if (t < mint)
                {
                    mint   = t;
                    border = 3;
                }
            }
            if (delta.y > 0)
            {
                // check top border
                double t = double(segend.y - p0.y) / delta.y;
                if (t < mint)
                {
                    mint   = t;
                    border = 2;
                }
            }
            else if (delta.y < 0)
            {
                // check bottom border
                double t = double(segoff.y - p0.y) / delta.y;
                if (t < mint)
                {
                    mint   = t;
                    border = 0;
                }
            }
            //			cout << "mint "<< mint<<" border "<<border <<"\n";
            //			cout << "p0: " << p0 << " p1 " << p1 << " dekta " <<
            // delta
            //<< "\n";
            ASSERT(border != -1, "paranoia mint");
            vector2i p2 = vector2i(
                int(round(p0.x + mint * delta.x)),
                int(round(p0.y + mint * delta.y)));
            //			cout << "p2 real " << double(p0.x) + mint * delta.x <<
            //","
            //<< double(p0.y) + mint * delta.y << "\n";
            vector2i rel = p2 - segoff;
            coastsegment::segpos ps2(
                static_cast<unsigned short>(rel.x), static_cast<unsigned short>(rel.y));
            ASSERT(
                ps2.x == 0 || ps2.x == SEGSCALE || ps2.y == 0
                    || ps2.y == SEGSCALE,
                "paranoia border");
            // if p1 is on a border/corner this should be handled correctly
            // by avoiding segcls with < 2 points.
            // if the segcl comes from seg x and leaves to y, but touches also
            // segs w and z, we will get segcls with one point in w and z and
            // the right segcl in y. w/z's segcls will get discarded later. So
            // everything is right
            scl.push_back_point(ps2);
            scl.endpos = borderpos(ps2);
            ASSERT(scl.endpos != -1, "borderpos check3");
            coastsegments[segcn].push_back_segcl(scl);
            // now switch to new segment if there are lines left
            if (i + 1 < cl.size())
            {
                segc   = compute_segment(p2, cl[i + 1]);
                segcn  = segc.y * segsx + segc.x;
                segoff = segc * SEGSCALE;
                segend = segoff + vector2i(SEGSCALE, SEGSCALE);
                rel    = p2 - segoff;
                ps0.x  = static_cast<unsigned short>(rel.x);
                ps0.y  = static_cast<unsigned short>(rel.y);
                scl    = coastsegment::segcl(global_clnr);
                scl.push_back_point(ps0);
                scl.beginpos = borderpos(ps0);
            }
            p0 = p2;
            // do not increase i, but continue with line p2->p1.
        }
    }

    // store segcls that are completely in a segment
    if (sameseg)
    {
        ASSERT(scl.points.size() >= 2, "less than 2 points for island???");
        scl.endpos = borderpos(ps0);
        scl.cyclic = clcyclic;
        if (clcyclic)
        {
            scl.beginpos = scl.endpos = -1;
        }
        scl.next = coastsegments[segcn].segcls.size();
        coastsegments[segcn].push_back_segcl(scl);
    }
}

void coastmap::process_coastline(int x, int y)
{
    ASSERT((mapf(x, y) & 0x80) == 0, "map pos already handled!");

    // find coastline, avoid "lakes", (inverse of islands), because the
    // triangulation will fault there
    std::vector<vector2i> points;
    bool cyclic;
    bool valid = find_coastline(x, y, points, cyclic);

    //	printf("coastline found from %i %i # pts %u cyclic? %u, bb %i eb %i
    // valid %u\n", x,y,points.size(),cyclic,beginborder,endborder,valid);

    if (!valid)
    {
        return; // skip
    }

    // create bspline curve
    std::vector<vector2> tmp;
    tmp.reserve(points.size());
    for (auto& point : points)
    {
        tmp.emplace_back(point.x, point.y);
        //		cout << i << "/" << points.size() << " is " << points[i] << " /
        //"
        //<< vector2(points[i].x, points[i].y) << "\n";
    }

    // now close coastline for islands. Must be done with double coordinates,
    // since we can have new points between old (integer coordinate) points.
    if (cyclic)
    {
        // close polygon and make sure the first and last line are linear
        // dependent this must be done so that the start point still is kept on
        // any border, if it was on a border. this can be done that way: take
        // points 0, 1, and n-1 (last) = b,c,a, with a != b so a,b,c are the
        // points over start/end, closing the coastline loop. either a,b,c are
        // on the same line, then all is right, or either a,b or b,c are both on
        // a border. In the latter case introduce a new point between a,b or b,c
        // and use that as start point. That way we have three points a,b,c or
        // a,ab',b or b,bc',c that form a line, the bspline is tangential to
        // that line at begin and end, so the bspline is smooth there.
        ASSERT(points.size() > 2, "points size < 2?");
        vector2i a = points.back();
        vector2i b = points[0];
        vector2i c = points[1];
        // check if x,y,z are not on a line, they are when all x or all y
        // coordinates are the same
        if ((a.x != b.x || b.x != c.x) && (a.y != b.y || b.y != c.y))
        {
            // not on one line, now either a or c are on a border
            //			cout << "a " << a << " b " << b << " c " << c << "\n";
            if (a.x % pixels_per_seg == 0 || a.y % pixels_per_seg == 0)
            {
                // a is on border
                vector2 a2, b2;
                a2.assign(a);
                b2.assign(b);
                vector2 ab = (a2 + b2) * 0.5;
                tmp.insert(tmp.begin(), ab); // push_front
                tmp.push_back(ab);
                //				cout << "a " << a << " b " << b << " c " << c <<
                //" ab " << ab << "\n";
            }
            else
            {
                //				ASSERT(c.x % pixels_per_seg == 0 || c.y %
                // pixels_per_seg == 0, "neither a nor c are on a border?");
                // c is on border, or this is an island without any border
                // contact.
                vector2 b2, c2;
                b2.assign(b);
                c2.assign(c);
                vector2 bc = (b2 + c2) * 0.5;
                b2         = tmp[0];
                tmp[0]     = bc;
                tmp.push_back(b2);
                tmp.push_back(bc);
                //				cout << "a " << a << " b " << b << " c " << c <<
                //" bc " << bc << "\n";
            }
        }
        else
        {
            // on one line. close coastline.
            tmp.push_back(tmp.front());
        }
    }
    // clean up
    points.clear();

    unsigned n = tmp.size() - 1;
    // A high n on small islands leads to a non-uniform spatial distribution of
    // bspline generated points. This looks ugly and is a serious drawback to
    // the old technique. Limiting n to tmp.size()/2 or tmp.size()/4 would be a
    // solution. tmp.size()/2 looks good.
    if (n > BSPLINE_SMOOTH_FACTOR)
    {
        n = BSPLINE_SMOOTH_FACTOR;
    }
    if (n > tmp.size() / 2)
    {
        n = tmp.size() / 2;
    }

    // create smooth version of the coastline.
    bsplinet<vector2> curve(n, tmp); // points in map pixel coordinates

    // smooth points will be scaled so that each segment is (2^16)-1 units long.
    auto nrpts = unsigned(tmp.size() * BSPLINE_DETAIL);
    ASSERT(nrpts >= 2, " nrpts < 2?");
    tmp.clear();
    std::vector<vector2i> spoints;
    spoints.reserve(nrpts);
    double sscal = double(SEGSCALE) / pixels_per_seg;
    //	cout << "generating " << nrpts << " pts\n";
    for (unsigned i = 0; i < nrpts; ++i)
    {
        vector2 cv = curve.value(float(i) / (nrpts - 1));
        // fixme: gcc4.0.1 strange values occour, are written here. but also
        // generated here? either cv values are weird/out of bounds or the
        // transformation here is buggy
        vector2i cvi =
            vector2i(int(round(cv.x * sscal)), int(round(cv.y * sscal)));
        /*
                cout << "generated point " << i << "/" << nrpts << " -> " << cvi
           << " cv is " << cv << " sscal " << sscal << " roundx: " << round(cv.x
           * sscal) << " roundy " << round(cv.y * sscal) << " intrx " <<
           int(round(cv.x * sscal)) << " intry " << int(round(cv.y * sscal)) <<
           "\n";
        */
        // avoid double points here., fixme assert them?
        if (spoints.empty() || !(spoints.back() == cvi))
        {
            spoints.push_back(cvi);
        }
    }

    divide_and_distribute_cl(spoints, cyclic);
    ++global_clnr;
}

void coastmap::process_segment(int sx, int sy)
{
    coastsegment& cs = coastsegments[sy * segsx + sx];
    if (cs.segcls.size() == 0)
    { // no coastlines in segment.
        // segment is fully land or sea. determine what it is
        // check pixel value of bottom left corner (land/sea)?
        if (mapf(sx * pixels_per_seg, sy * pixels_per_seg) & 0x7f)
        {
            cs.type = 1;
        }
        else
        {
            cs.type = 0;
        }
    }
    else
    { // there are coastlines in segment
        cs.type = 2;

        // try to connect segcls that were made from the same global cl
        unsigned erased = 0;
        for (unsigned i = 0; i < cs.segcls.size(); ++i)
        {
            coastsegment::segcl& cl0 = cs.segcls[i];
            if (cl0.global_clnr == -1)
            {
                continue;
            }
            if (cl0.cyclic)
            {
                continue;
            }
            for (unsigned j = 0; j < cs.segcls.size(); ++j)
            {
                if (j == i)
                {
                    continue;
                }
                coastsegment::segcl& cl1 = cs.segcls[j];
                if (cl1.cyclic)
                {
                    continue;
                }
                if (cl0.global_clnr != cl1.global_clnr)
                {
                    continue; // also avoids erased segs.
                }
                if (cl0.endpos == cl1.beginpos)
                {
                    ASSERT(cl0.endpos != -1, "strange paranoia cl0endpos");
                    ASSERT(cl0.next == -1, "strange paranoia1 next != -1");
                    ASSERT(cl1.next == -1, "strange paranoia2 next != -1");
                    // connect the segcls.
                    cl0.points.insert(
                        cl0.points.end(),
                        ++cl1.points.begin(),
                        cl1.points.end());
                    //					cl0.points += cl1.points;
                    cl0.endpos      = cl1.endpos;
                    cl1.global_clnr = -1; // mark as erased.
                    ++erased;
                }
            }
        }
        // clear erased segcls.
        if (erased > 0)
        {
            unsigned s = cs.segcls.size();
            std::vector<coastsegment::segcl> segcls_new;
            segcls_new.reserve(s - erased);
            unsigned j = 0;
            for (unsigned i = 0; i < s; ++i)
            {
                coastsegment::segcl& scl = cs.segcls[i];
                if (scl.global_clnr != -1)
                {
                    // not erased, keep it.
                    if (scl.next != -1)
                    {
                        ASSERT(scl.next == i, "next paranoia");
                        scl.next = j;
                    }
                    segcls_new.push_back(scl);
                    ++j;
                }
            }
            cs.segcls.swap(segcls_new);
        }

        // vector2 segoff = vector2(sx * segw_real + realoffset.x, sy *
        // segw_real + realoffset.y);

        // compute cl.next info
        for (unsigned i = 0; i < cs.segcls.size(); ++i)
        {
            cs.compute_successor_for_cl(i);
        }
    }
}

auto coastmap::quadrant(const vector2i& d) -> unsigned
{
    if (d.x < 0)
    {
        if (d.y < 0)
        {
            return 5;
        }
        else if (d.y > 0)
        {
            return 7;
        }
        else
        {
            return 6;
        }
    }
    else if (d.x > 0)
    {
        if (d.y < 0)
        {
            return 3;
        }
        else if (d.y > 0)
        {
            return 1;
        }
        else
        {
            return 2;
        }
    }
    else
    {
        if (d.y < 0)
        {
            return 4;
        }
        else if (d.y > 0)
        {
            return 0;
        }
        else
        {
            ASSERT(false, "quadrant called with 0!");
            return 8;
        }
    }
}

auto coastmap::segcoord_to_real(
    int segx,
    int segy,
    const coastsegment::segpos& sp) const -> vector2
{
    vector2 tmp(
        double(segx) + double(sp.x) / SEGSCALE,
        double(segy) + double(sp.y) / SEGSCALE);
    return (tmp * segw_real) + realoffset;
}

auto coastmap::segcoord_to_texc(int segx, int segy) const -> vector2f
{
    // float get to its limit when segsx,segsy > 256, bot that doesn't really
    // matter.
    return {float(segx) / segsx, 1.0f - float(segy) / segsy};
}

// load from xml description file
coastmap::coastmap(const string& filename)
{
    atlanticmap = std::make_unique<texture>(
        get_texture_dir() + "atlanticmap.jpg", texture::LINEAR, texture::CLAMP);

    global_clnr = 0;

    xml_doc doc(filename);
    doc.load();
    xml_elem er  = doc.child("dftd-map");
    xml_elem et  = er.child("topology");
    realwidth    = et.attrf("realwidth");
    realoffset.x = et.attrf("realoffsetx");
    realoffset.y = et.attrf("realoffsety");
    if (er.has_child("cities"))
    {
        for (auto elem : er.child("cities").iterate("city"))
        {
            cities.emplace_back(
                vector2(
                    transform_nautic_posx_to_real(elem.attr("posx")),
                    transform_nautic_posy_to_real(elem.attr("posy"))),
                elem.attr("name"));
        }
    }

    if (er.has_child("props"))
    {
        for (auto elem : er.child("props").iterate("prop"))
        {
            std::string mdlname = elem.attr("model");
            std::string path    = data_file().get_rel_path(mdlname) + mdlname
                               + ".xml"; // fixme: later ddxml!
            model* m = modelcache().ref(path);
            m->register_layout();
            m->set_layout();
            props.emplace_back(
                path,
                vector2(elem.attrf("posx"), elem.attrf("posy")),
                elem.attrf("angle"));
        }
    }

    {
        sdl_image surf(get_map_dir() + et.attr("image"));
        mapw        = surf->w;
        maph        = surf->h;
        pixelw_real = realwidth / mapw;
        realheight  = maph * realwidth / mapw;
        // compute integer number of pixels per segment
        auto pixperseqnonpower2 = unsigned(ceil(60000 / pixelw_real));
        // find next power of 2 that is larger or equal than computed nonpower2
        // pixel width
        for (pixels_per_seg = 1; pixels_per_seg < pixperseqnonpower2;
             pixels_per_seg <<= 1)
        {
            ;
        }

        segsx     = mapw / pixels_per_seg;
        segsy     = maph / pixels_per_seg;
        segw_real = pixelw_real * pixels_per_seg;
        if (segsx * pixels_per_seg != mapw || segsy * pixels_per_seg != maph)
        {
            THROW(
                error,
                string("coastmap: map size must be integer multiple of "
                       "segment size, in")
                    + filename);
        }

        themap.resize(mapw * maph);

        surf.lock();
        if (surf->format->BytesPerPixel != 1 || surf->format->palette == nullptr
            || surf->format->palette->ncolors != 2)
        {
            THROW(
                error,
                string("coastmap: image is no black/white 1bpp paletted "
                       "image, in ")
                    + filename);
        }

        auto* offset = static_cast<uint8_t*>(surf->pixels);
        int mapoffy  = maph * mapw;
        for (int yy = 0; yy < int(maph); yy++)
        {
            mapoffy -= mapw;
            for (int xx = 0; xx < int(mapw); ++xx)
            {
                uint8_t c            = (*offset++);
                themap[mapoffy + xx] = (c > 0) ? 1 : 0;
            }
            offset += surf->pitch - mapw;
        }
        surf.unlock();
    }

    add_loading_screen("image transformed");

    // here spin off work to other thread
    myworker.reset(new worker(*this));
    myworker->start();
}

coastmap::~coastmap()
{
    for (auto& it : props)
    {
        modelcache().unref(it.modelname);
    }
}

void coastmap::construction_threaded()
{
    // they are filled in by process_coastline
    coastsegments.resize(segsx * segsy);
    for (auto& coastsegment : coastsegments)
    {
        coastsegment.atlanticmap = &*atlanticmap;
    }

    // find coastlines
    // when to start processing: all patterns, except: 0,5,10,15
    for (int yy = 0; yy < int(maph); ++yy)
    {
        for (int xx = 0; xx < int(mapw); ++xx)
        {
            if (mapf(xx, yy) & 0x80)
            {
                continue;
            }
            uint8_t pattern = 0;
            uint8_t marker  = 0;
            for (int j = 0; j < 4; ++j)
            {
                uint8_t c = mapf(xx + dmx[j], yy + dmy[j]);
                pattern |= (c & 0x7f) << j;
                marker |= c;
            }
            if (patternprocessok[pattern] && ((marker & 0x80) == 0))
            {
                process_coastline(xx, yy);
            }
        }
    }

    // find coastsegment type and successors of cls.
    for (unsigned yy = 0; yy < segsy; ++yy)
    {
        for (unsigned xx = 0; xx < segsx; ++xx)
        {
            process_segment(xx, yy);
        }
    }

    // fixme: clear "themap" so save space.
    // information wether a position on the map is land or sea can be computed
    // from segment data. This will save 6MB of space at least.
}

void coastmap::finish_construction()
{
    // lets the worker finish its work, then clears worker-ptr
    myworker.reset();
    add_loading_screen("coastmap created");
}

void coastmap::draw_as_map(const vector2& droff, double mapzoom, int detail)
    const
{
    int x, y, w, h;
    // cout << "mapzoom pix/m = " << mapzoom << " segwreal " << segw_real <<
    // "\n";
    w = int(ceil((1024 / mapzoom) / segw_real)) + 2;
    h = int(ceil((768 / mapzoom) / segw_real))
        + 2; // fixme: use 640 and translate map y - 64
             // cout << " w " << w << " h " << h << " waren's mal.\n";
    x = int(floor((droff.x - realoffset.x) / segw_real)) - w / 2;
    y = int(floor((droff.y - realoffset.y) / segw_real)) - h / 2;
    if (x < 0)
    {
        w += x;
        x = 0;
    }
    if (y < 0)
    {
        h += y;
        y = 0;
    }
    if (x + w > int(segsx))
    {
        w = int(segsx) - x;
    }
    if (y + h > int(segsy))
    {
        h = int(segsy) - y;
    }
    // cout<<"draw map   segsx " << segsx << " segsy " << segsy << " x " << x <<
    // " y " << y << " w " << w << " h " << h << "\n";

    atlanticmap->set_gl_texture();
    for (int yy = y; yy < y + h; ++yy)
    {
        for (int xx = x; xx < x + w; ++xx)
        {
            glPushMatrix();
            glTranslated(
                xx * segw_real + realoffset.x,
                yy * segw_real + realoffset.y,
                0);
            coastsegments[yy * segsx + xx].draw_as_map(*this, xx, yy, detail);
            glPopMatrix();
        }
    }
    /*
        // draw cities, fixme move to coastmap
        for (list<pair<vector2, string> >::const_iterator it = cities.begin();
       it != cities.end(); ++it) { draw_square_mark(gm, it->first, -offset,
       color(255, 0, 0)); vector2 pos = (it->first - offset) * mapzoom;
            font_arial->print(int(512 + pos.x), int(384 - pos.y), it->second);
        }
    */
}

// p is real world coordinate of viewer. modelview matrix is centered around
// viewer
void coastmap::render(
    const vector2& p,
    double vr,
    bool mirrored,
    int detail,
    bool withterraintop) const
{
    // render props, do some view culling for them.
    for (const auto& it : props)
    {
        if (it.pos.square_distance(p) < vr * vr)
        {
            // potentially visible
            glPushMatrix();
            glTranslatef(it.pos.x - p.x, it.pos.y - p.y, 0); //- p.z
            glRotatef(-it.dir, 0, 0, 1);
            if (mirrored)
            {
                // fixme: display_mirror_clip must be called with
                // certain conditions, that are not used here yet...
                modelcache().find(it.modelname)->display_mirror_clip();
            }
            else
            {
                modelcache().find(it.modelname)->display();
            }
            glPopMatrix();
        }
    }
}
