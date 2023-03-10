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

//
//  A generic frustum (C)+(W) 2005 Thorsten Jordan
//

#include "frustum.h"

#include "matrix4.h"
#include "polygon.h"

#include <iostream>
#include <utility>

frustum::frustum(polygon poly, const vector3& viewp, double znear_) :
    viewpos(viewp), znear(znear_)
{
    planes.reserve(poly.points.size());
    for (unsigned i = 0; i < poly.points.size(); ++i)
    {
        unsigned j = (i + 1) % poly.points.size();
        planes.emplace_back(poly.points[i], viewpos, poly.points[j]);
    }
}

auto frustum::clip(polygon p) const -> polygon
{
    polygon result = std::move(p);
    for (const auto& plane : planes)
    {
        result = result.clip(plane).first;
    }
    return result;
}

/*
void frustum::draw() const
{
    viewwindow.draw();
}

void frustum::print() const
{
    std::cout << "Frustum: viewpos " << viewpos << "\n";
    viewwindow.print();
}
*/

auto frustum::from_opengl() -> frustum
{
    matrix4 mv     = matrix4::get_gl(GL_MODELVIEW_MATRIX);
    matrix4 prj    = matrix4::get_gl(GL_PROJECTION_MATRIX);
    matrix4 mvp    = prj * mv;
    matrix4 invmv  = mv.inverse();
    matrix4 invmvp = mvp.inverse();
    // since the viewer looks along neg. z-axis, the viewer
    // coordinate system (after perspective projection) has positive
    // z-values going into the screen, so the near z-value is
    // -1, the far z-value is 1.
    vector3 wbln    = invmvp * vector3(-1, -1, -1);
    vector3 wbrn    = invmvp * vector3(+1, -1, -1);
    vector3 wtln    = invmvp * vector3(-1, +1, -1);
    vector3 wtrn    = invmvp * vector3(+1, +1, -1);
    vector3 viewpos = invmv * vector3(0, 0, 0);
    polygon viewwindow(wbln, wbrn, wtrn, wtln);
    // viewwindow.print();
    double z_near_distance = viewwindow.get_plane().distance(viewpos);
    return frustum(viewwindow, viewpos, z_near_distance);
}

void frustum::translate(const vector3& delta)
{
    viewpos += delta;
    for (auto& plane : planes)
    {
        plane.translate(delta);
    }
}

auto frustum::get_mirrored() const -> frustum
{
    frustum f   = *this;
    f.viewpos.z = -f.viewpos.z;
    for (auto& it : f.planes)
    {
        // d remains the same
        it.N.z = -it.N.z;
    }
    return f;
}
