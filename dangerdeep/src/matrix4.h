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
//  A 4x4 matrix (C)+(W) 2001 Thorsten Jordan
//

#ifndef MATRIX4_H
#define MATRIX4_H

#include "matrix.h"

template<class D> vector3t<D> operator* (const matrixt<D, 4U>& m, const vector3t<D>& v)
{
	return m.mul4vec3(v);
}

typedef matrixt<double, 4U> matrix4;
typedef matrixt<float, 4U> matrix4f;

#endif
