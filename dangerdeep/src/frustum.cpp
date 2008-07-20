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

//
//  A generic frustum (C)+(W) 2005 Thorsten Jordan
//

#include "frustum.h"
#include "matrix4.h"
#include "polygon.h"
#include <iostream>

frustum::frustum(polygon poly, const vector3& viewp, double znear_)
	: viewwindow(poly), viewpos(viewp), znear(znear_)
{
	planes.reserve(poly.points.size());
	for (unsigned i = 0; i < poly.points.size(); ++i) {
		unsigned j = (i+1)%poly.points.size();
		planes.push_back(plane(poly.points[i], viewpos, poly.points[j]));
	}
}

polygon frustum::clip(polygon p) const
{
	polygon result = p;
	for (unsigned i = 0; i < planes.size(); ++i)
		result = result.clip(planes[i]);
	return result;
}

void frustum::draw() const
{
	glColor3f(0,1,0);
	viewwindow.draw();
}

void frustum::print() const
{
	std::cout << "Frustum: viewpos " << viewpos << "\n";
	viewwindow.print();
}



frustum frustum::from_opengl()
{
	matrix4 mv = matrix4::get_gl(GL_MODELVIEW_MATRIX);
	matrix4 prj = matrix4::get_gl(GL_PROJECTION_MATRIX);
	matrix4 mvp = prj * mv;
	matrix4 invmv = mv.inverse();
	matrix4 invmvp = mvp.inverse();
	vector3 wbln = invmvp * vector3(-1,-1,-1);
	vector3 wbrn = invmvp * vector3(+1,-1,-1);
	vector3 wtln = invmvp * vector3(-1,+1,-1);
	vector3 wtrn = invmvp * vector3(+1,+1,-1);
	vector3 viewpos = invmv * vector3(0,0,0);
	polygon viewwindow(wbln, wbrn, wtrn, wtln);
	//viewwindow.print();
	double z_near_distance = viewwindow.get_plane().distance(viewpos);
	return frustum(viewwindow, viewpos, z_near_distance);
}



void frustum::translate(const vector3& delta)
{
	viewpos += delta;
	viewwindow.translate(delta);
	for (unsigned i = 0; i < planes.size(); ++i)
		planes[i].translate(delta);
}
