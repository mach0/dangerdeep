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
//  A 3d sphere (C)+(W) 2009 Thorsten Jordan
//

#pragma once

#include "vector3.h"

#include <utility>

/// a 3d sphere with template coordinate types
template<class D>
class sphere_t
{
  public:
    vector3t<D> center; ///< center of sphere
    D radius;           ///< radius of sphere

    sphere_t() : radius(0) { }
    sphere_t(vector3t<D> c, const D& r) : center(std::move(c)), radius(r) { }

    /// construct from three points (triangle).
    // sphere_t(const vector3t<D>& a, const vector3t<D>& b, const vector3t<D>&
    // c) { }
    /// determine if point is inside sphere
    [[nodiscard]] bool is_inside(const vector3t<D>& a) const
    {
        return center.square_distance(a) < radius * radius;
    }

    /// determine if spheres intersect
    [[nodiscard]] bool intersects(const sphere_t<D>& other) const
    {
        D r = radius + other.radius;
        return center.square_distance(other.center) < r * r;
    }

    /// build minimum combination sphere
    [[nodiscard]] sphere_t<D> compute_bound(const sphere_t<D>& other) const
    {
        // new center is on axis between the two spheres
        vector3t<D> delta = other.center - center;

        D distance = delta.length();

        if (distance < epsilon<D>())
            return sphere_t<D>(
                center, std::max(radius, other.radius + distance));

        D new_diameter =
            std::max(radius + distance + other.radius, radius * D(2));
        D new_radius = new_diameter / D(2);

        vector3t<D> new_center =
            center + delta * ((new_radius - radius) / distance);
        return sphere_t<D>(new_center, new_radius);
    }

    /// compute min/max x,y,z values
    void compute_min_max(vector3t<D>& minv, vector3t<D>& maxv) const
    {
        vector3t<D> R(radius, radius, radius);
        minv = minv.min(center - R);
        maxv = maxv.max(center + R);
    }
};

using sphere  = sphere_t<double>;
using spheref = sphere_t<float>;
