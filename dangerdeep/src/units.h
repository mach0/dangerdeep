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

// physical units
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#pragma once

#include "angle.h"
#include "matrix3.h"
#include "quaternion.h"
#include "vector3.h"

// Physical values, use C++ types to define valid calculations
/// Physical unit names
enum class physical_unit
{
    time_point,   // (s)
    duration,     // s
    mass,         // kg
    acceleration, // m/s^2
    velocity,     // acceleration*time, m/s
    force,        // force*acceleration, kg*m/s^2 = N
    momentum, // force*time or torque*time = mass*velocity, kg*m/s = N*s, also
              // called impulse
    torque,   // force*distance, N*m
    angular_velocity, // angle/s
    distance,         // meters
    area              // meters^2
};

/// Base class for physical unit
template <typename T, physical_unit P> struct physical_value
{
    /// The value itself, can be number or vector
    T value{T(0)};
    /// Default constructor
    physical_value() = default;
    /// Explicit construction, use with care
    physical_value(T v) : value(v) { }
    /// Rotate 3d values
    auto rotate(const quaternion& q) const
    {
        return physical_value<T, P>(q.rotate(value));
    }
    /// Cross product for 3d values
    auto cross(const vector3& v) const
    {
        return physical_value<T, P>(value.cross(v));
    }
    /// Add values
    auto operator+=(const physical_value<T, P>& other) { value += other.value; }
    /// Add values
    auto operator+(const physical_value<T, P>& other) const
    {
        return physical_value<T, P>(value + other.value);
    }
    /// Compare
    auto operator<(const physical_value<T, P>& other) const
    {
        return value < other.value;
    }
};
/// transform 3d values (also rotation)
template <physical_unit P>
auto operator*(const matrix3& m, const physical_value<vector3, P>& v)
{
    return physical_value<vector3, P>(m * v.value);
}

/// Duration in seconds
typedef physical_value<double, physical_unit::duration> duration;
duration& operator-=(duration& a, const duration& b)
{
    a.value -= b.value;
    return a;
}
bool operator>(duration& a, const duration& b) { return a.value > b.value; }

/// Time stamp in seconds
typedef physical_value<double, physical_unit::time_point> time_point;
auto duration_between(time_point t0, time_point t1)
{
    return duration(t1.value - t0.value);
}

/// Velocity in meters per second
typedef physical_value<double, physical_unit::velocity> velocity1d;
typedef physical_value<vector3, physical_unit::velocity> velocity3d;
template <typename T>
auto operator*(const physical_value<T, physical_unit::velocity>& v, duration d)
{
    return v.value * d.value;
}

/// Angular velocity
typedef physical_value<double, physical_unit::angular_velocity>
    angular_velocity;
angle operator*(angular_velocity av, duration d)
{
    return angle(av.value * d.value);
}

/// Force in newtons
typedef physical_value<double, physical_unit::force> force1d;
typedef physical_value<vector3, physical_unit::force> force3d;
template <typename T>
auto operator*(const physical_value<T, physical_unit::force>& v, duration d)
{
    return physical_value<T, physical_unit::momentum>(v.value * d.value);
}

/// Acceleration in meters per second squared
typedef physical_value<double, physical_unit::acceleration> acceleration1d;
typedef physical_value<vector3, physical_unit::acceleration> acceleration3d;
template <typename T>
auto operator*(
    const physical_value<T, physical_unit::acceleration>& v, duration d)
{
    return physical_value<T, physical_unit::velocity>(v.value * d.value);
}

/// Torque in newtons times meters
typedef physical_value<double, physical_unit::torque> torque1d;
typedef physical_value<vector3, physical_unit::torque> torque3d;
template <typename T>
auto operator*(const physical_value<T, physical_unit::torque>& t, duration d)
{
    // returns angular momentum!
    return physical_value<T, physical_unit::momentum>(t.value * d.value);
}

/// Mass in kilograms
typedef physical_value<double, physical_unit::mass> mass1d;
/// Get force value in newtons
template <typename T>
auto operator*(
    const physical_value<T, physical_unit::acceleration>& a, mass1d m)
{
    return physical_value<T, physical_unit::force>(a.value * m.value);
}
/// Get linear momentum
template <typename T>
auto operator*(const physical_value<T, physical_unit::velocity>& v, mass1d m)
{
    return physical_value<T, physical_unit::momentum>(v.value * m.value);
}
/// Get velocity from momentum
template <typename T>
auto operator/(const physical_value<T, physical_unit::momentum>& v, mass1d m)
{
    return physical_value<T, physical_unit::velocity>(
        v.value * (1.0 / m.value));
}

/// Momentum in kilograms times meters per second
typedef physical_value<double, physical_unit::momentum> momentum1d;
typedef physical_value<vector3, physical_unit::momentum> momentum3d;

/// Force caused by mass and gravity
force3d gravity_force(mass1d m)
{
    return force3d(vector3(axis::neg_z) * m.value * constant::GRAVITY);
}

/// Distance in meters
typedef physical_value<double, physical_unit::distance> distance;
distance get_distance(const vector3& a, const vector3& b)
{
    return distance((a - b).length());
}
velocity1d operator/(distance d, duration t)
{
    return velocity1d(d.value / t.value);
}

/// 2d area
typedef physical_value<double, physical_unit::area> area2d;

/// Later have some class that checks value is between 0 and 1. Maybe add class
/// for that, no physical unit.
typedef double factor;

