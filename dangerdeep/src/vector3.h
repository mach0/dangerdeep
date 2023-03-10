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
//  A 3d vector (C)+(W) 2001 Thorsten Jordan
//

#pragma once

#include "vector2.h"

template<typename D2>
class vector4t;

/// Special type to identify axes
enum class axis
{
    none,
    x,
    y,
    z,
    neg_x,
    neg_y,
    neg_z
};

///\brief Template class for a mathematical vector with three coefficients.
template<typename D>
class vector3t
{
  public:
    D x, y, z;

    vector3t() : x(0), y(0), z(0) { }
    vector3t(D x_, D y_, D z_) : x(x_), y(y_), z(z_) { }
    vector3t(const vector2t<D>& v, D z_) : x(v.x), y(v.y), z(z_) { }

    template<typename E>
    vector3t(const vector3t<E>& other) :
        x(D(other.x)), y(D(other.y)), z(D(other.z))
    {
    }

    [[nodiscard]] vector3t<D> normal() const
    {
        D len = D(1.0) / length();
        return vector3t(x * len, y * len, z * len);
    }

    void normalize()
    {
        D len = D(1.0) / length();
        x *= len;
        y *= len;
        z *= len;
    }

    [[nodiscard]] vector3t<D> orthogonal(const vector3t<D>& other) const
    {
        return cross(other);
    }

    vector3t<D> operator*(D scalar) const
    {
        return vector3t(x * scalar, y * scalar, z * scalar);
    }

    vector3t<D> operator/(D scalar) const
    {
        return vector3t(x / scalar, y / scalar, z / scalar);
    }

    vector3t<D> operator+(const vector3t<D>& other) const
    {
        return vector3t(x + other.x, y + other.y, z + other.z);
    }

    vector3t<D> operator-(const vector3t<D>& other) const
    {
        return vector3t(x - other.x, y - other.y, z - other.z);
    }

    vector3t<D> operator-() const { return vector3t(-x, -y, -z); }

    vector3t<D>& operator+=(const vector3t<D>& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    vector3t<D>& operator-=(const vector3t<D>& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    vector3t<D>& operator*=(D s)
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    [[nodiscard]] vector3t<D> min(const vector3t<D>& other) const
    {
        return vector3t(
            std::min(x, other.x), std::min(y, other.y), std::min(z, other.z));
    }

    [[nodiscard]] vector3t<D> max(const vector3t<D>& other) const
    {
        return vector3t(
            std::max(x, other.x), std::max(y, other.y), std::max(z, other.z));
    }

    [[nodiscard]] vector3t<D> sign() const
    {
        return vector3t(
            x < 0 ? D(-1) : (x > 0 ? D(1) : D(0)),
            y < 0 ? D(-1) : (y > 0 ? D(1) : D(0)),
            z < 0 ? D(-1) : (z > 0 ? D(1) : D(0)));
    }

    [[nodiscard]] vector3t<D> abs() const
    {
        return vector3t(
            x < D(0) ? -x : x, y < D(0) ? -y : y, z < D(0) ? -z : z);
    }

    bool operator==(const vector3t<D>& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const vector3t<D>& other) const
    {
        return x != other.x || y != other.y || z != other.z;
    }

    [[nodiscard]] D square_length() const { return x * x + y * y + z * z; }
    [[nodiscard]] D length() const { return D(::sqrt(square_length())); }

    [[nodiscard]] D square_distance(const vector3t<D>& other) const
    {
        vector3t<D> n = *this - other;
        return n.square_length();
    }

    [[nodiscard]] D distance(const vector3t<D>& other) const
    {
        vector3t<D> n = *this - other;
        return n.length();
    }

    D operator*(const vector3t<D>& other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    [[nodiscard]] vector3t<D> cross(const vector3t<D>& other) const
    {
        return vector3t(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x);
    }

    bool solve(
        const vector3t<D>& o1,
        const vector3t<D>& o2,
        const vector3t<D>& o3,
        D& s1,
        D& s2,
        D& s3) const;

    /// multiplies 3x3 matrix (given in columns c0-c2) with *this.
    [[nodiscard]] vector3t<D> matrixmul(
        const vector3t<D>& c0,
        const vector3t<D>& c1,
        const vector3t<D>& c2) const;

    [[nodiscard]] vector3t<D> coeff_mul(const vector3t<D>& other) const
    {
        return vector3t(x * other.x, y * other.y, z * other.z);
    }

    [[nodiscard]] vector2t<D> xy() const { return vector2t<D>(x, y); }
    [[nodiscard]] vector2t<D> yz() const { return vector2t<D>(y, z); }
    [[nodiscard]] vector4t<D> xyz0() const { return vector4t<D>(x, y, z, 0); }
    [[nodiscard]] vector4t<D> xyzw(D w) const
    {
        return vector4t<D>(x, y, z, w);
    }

    template<typename D2>
    friend std::ostream& operator<<(std::ostream& os, const vector3t<D2>& v);

    template<typename E>
    void assign(const vector3t<E>& other)
    {
        x = D(other.x);
        y = D(other.y);
        z = D(other.z);
    }

    void to_mem(float* p) const
    {
        p[0] = x;
        p[1] = y;
        p[2] = z;
    }

    [[nodiscard]] vector3t<D> rcp() const
    {
        return vector3t(D(1) / x, D(1) / y, D(1) / z);
    }

    [[nodiscard]] D
    determinate(const vector3t<D>& b, const vector3t<D>& c) const
    {
        return x * b.y * c.z + b.x * c.y * z + c.x * y * b.z - x * c.y * b.z
               - b.x * y * c.z - c.x * b.y * z;
    }

    /// Construct axis vector
    constexpr vector3t<D>(axis a) : vector3t<D>()
    {
        switch (a)
        {
            case axis::x:
                x = D(1);
                break;
            case axis::y:
                y = D(1);
                break;
            case axis::z:
                z = D(1);
                break;
            case axis::neg_x:
                x = D(-1);
                break;
            case axis::neg_y:
                y = D(-1);
                break;
            case axis::neg_z:
                z = D(-1);
                break;
            default:
                break;
        }
    }

    /// Get axis value
    [[nodiscard]] constexpr D at(axis a) const
    {
        switch (a)
        {
            case axis::x:
                return x;
            case axis::y:
                return y;
            case axis::z:
                return z;
            case axis::neg_x:
                return -x;
            case axis::neg_y:
                return -y;
            case axis::neg_z:
                return -z;
            default:
                break;
        }
    }
};

template<typename D2>
inline vector3t<D2> operator*(D2 scalar, const vector3t<D2>& v)
{
    return v * scalar;
}

template<typename D>
bool vector3t<D>::solve(
    const vector3t<D>& o1,
    const vector3t<D>& o2,
    const vector3t<D>& o3,
    D& s1,
    D& s2,
    D& s3) const
{
    D det = // sarrus
        o1.x * o2.y * o3.z - o1.x * o3.y * o2.z + o2.x * o3.y * o1.z
        - o2.x * o1.y * o3.z + o3.x * o1.y * o2.z - o3.x * o2.y * o1.z;

    if (!det)
        return false;

    s1 = ((o2.y * o3.z - o2.z * o3.y) * x + (o2.z * o3.x - o2.x * o3.z) * y
          + (o2.x * o3.y - o2.y * o3.x) * z)
         / det;

    s2 = ((o1.z * o3.y - o1.y * o3.z) * x + (o1.x * o3.z - o1.z * o3.x) * y
          + (o1.y * o3.x - o1.x * o3.y) * z)
         / det;

    s3 = ((o1.y * o2.z - o1.z * o2.y) * x + (o1.z * o2.x - o1.x * o2.z) * y
          + (o1.x * o2.y - o1.y * o2.x) * z)
         / det;

    return true;
}

template<typename D>
vector3t<D> vector3t<D>::matrixmul(
    const vector3t<D>& c0,
    const vector3t<D>& c1,
    const vector3t<D>& c2) const
{
    return vector3t<D>(
        c0.x * x + c1.x * y + c2.x * z,
        c0.y * x + c1.y * y + c2.y * z,
        c0.z * x + c1.z * y + c2.z * z);
}

template<typename D>
std::ostream& operator<<(std::ostream& os, const vector3t<D>& v)
{
    os << "x=" << v.x << "; y=" << v.y << "; z=" << v.z;
    return os;
}

using vector3  = vector3t<double>;
using vector3f = vector3t<float>;
using vector3i = vector3t<int>;
using vector3u = vector3t<unsigned int>;
