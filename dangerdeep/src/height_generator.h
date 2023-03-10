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

// interface class to compute heights

#pragma once

#include "color.h"
#include "datadirs.h"
#include "texture.h"
#include "vector3.h"
#include "vector4.h"

#include <vector>

/* possible interface changes ahead:
   normals have 2x resolution than vertices,
   colors have individual factor (power of 2) related to vertices.
   Finer values as vertex resolution is fetched by requesting smaller detail
   numbers, i.e. normals for detail=k are requested with detail=k-1 to have
   twice resolution. This means we can have detail<0, which cant be handled by
   the renderer for the geometry yet. It can be simplified alltogether by
   limiting detail at 0 (minimum 0), and compute_normals/compute_colors generate
   more samples as vertices with the SAME detail value, i.e. at detail=k twice
   the number of normals is returned than the number of vertices. That factor
   shall be requested from the height generator (already possible for colors).
   see also the comment at head of geoclipmap.cpp
   Normally it means detail<0 for vertices that vertex data is generated, and
   finer than existing data. However the renderer doesn't care about the source,
   and there must be a true lowest detail level that it can handle, it won't
   work to zoom in more and more until microscopic space...
*/

/// interface class to generate heights, normals and texture data for the
/// geoclipmap renderer
class height_generator
{
  public:
    /// destructor
    virtual ~height_generator() = default;

    /// compute height values of given detail and coordinate area (including
    /// given coordinates)
    ///@param detail - detail level to be generated and also coordinate domain,
    ///                0 means a sample spacing of "L", the basic geometry
    ///                clipmap spacing, higher values mean coarser levels,
    ///                values < 0 mean extra detail, finer than basic
    ///                resolution.
    ///@param coord_bl - xy coordinates for the value to generate, scaled to
    /// match detail level, bottem left inclusive
    ///@param coord_sz - xy coordinate range for the value to generate, scaled
    /// to match detail level
    ///@param dest - destination where to write height values
    ///@param stride - distance between every value in floats, give 0 for packed
    /// values
    ///@param line_stride - distance between two lines in floats, give 0 for
    /// packed lines
    virtual void compute_heights(
        int detail,
        const vector2i& coord_bl,
        const vector2i& coord_sz,
        float* dest,
        unsigned stride      = 0,
        unsigned line_stride = 0,
        bool noise           = true) = 0;
    /* example implementation:
    {
        if (!stride) stride = 1;
        if (!line_stride) line_stride = coord_sz.x * stride;
        for (int y = 0; y < coord_sz.y; ++y) {
            float* dest2 = dest;
            for (int x = 0; x < coord_sz.x; ++x) {
                *dest2 = FUNC(detail, coord_bl + vector2i(x, y));
                dest2 += stride;
            }
            dest += line_stride;
        }
    }
    */
    [[nodiscard]] const texture& get_base_texture() const
    {
        return *base_texture;
    }
    [[nodiscard]] const texture& get_noise_texture() const
    {
        return *noise_texture;
    }
    [[nodiscard]] const texture& get_sand_texture() const
    {
        return *sand_texture;
    }
    [[nodiscard]] const texture& get_mud_texture() const
    {
        return *mud_texture;
    }
    [[nodiscard]] const texture& get_grass_texture() const
    {
        return *grass_texture;
    }
    [[nodiscard]] const texture& get_forest_texture() const
    {
        return *forest_texture;
    }
    [[nodiscard]] const texture& get_rock_texture() const
    {
        return *rock_texture;
    }
    [[nodiscard]] const texture& get_snow_texture() const
    {
        return *snow_texture;
    }
    [[nodiscard]] const texture& get_forest_brdf_texture() const
    {
        return *forest_brdf_texture;
    }
    [[nodiscard]] const texture& get_rock_brdf_texture() const
    {
        return *rock_brdf_texture;
    }

    float get_tex_stretch_factor() { return tex_stretch_factor; }
    /// compute normal values of given detail and coordinate area (including
    /// given coordinates)
    ///@note here is some reasonable implementation, normally it should be
    /// overloaded, normals are always packed
    ///@param detail - detail level to be generated and also coordinate domain,
    ///@param coord_bl - xy coordinates for the value to generate, scaled to
    /// match detail level, bottem left inclusive
    ///@param coord_sz - xy coordinate range for the value to generate, scaled
    /// to match detail level
    ///@param dest - destination where to write normal values
    virtual void compute_normals(
        int detail,
        const vector2i& coord_bl,
        const vector2i& coord_sz,
        vector3f* dest)
    {
        const auto zh = float(
            sample_spacing * 0.5f
            * (detail >= 0 ? (1 << detail) : 1.0f / (1 << -detail)));
        // compute heights to generate normals, we need one height value more in
        // every direction
        vector2i s2 = coord_sz + vector2i(2, 2);
        std::vector<float> h(s2.x * s2.y);
        compute_heights(detail, coord_bl - vector2i(1, 1), s2, &h[0], 0, 0);
        unsigned hli = s2.x + 1;
        for (int y = 0; y < coord_sz.y; ++y)
        {
            for (int x = 0; x < coord_sz.x; ++x)
            {
                *dest++ = vector3f(
                              h[hli - 1] - h[hli + 1],
                              h[hli - s2.x] - h[hli + s2.x],
                              zh * 2)
                              .normal();
                ++hli;
            }
            hli += 2;
        }
    }

    /// get absolute minimum and maximum height of all levels, used for clipping
    ///@param minh - minimum height values of all levels and samples
    ///@param maxh - maximum height values of all levels and samples
    virtual void get_min_max_height(double& minh, double& maxh) const = 0;

    /// get sample spacing of detail level 0 (geometry)
    [[nodiscard]] double get_sample_spacing() const { return sample_spacing; }

    /// get color res factor (log2 of it)
    [[nodiscard]] unsigned get_log2_color_res_factor() const
    {
        return log2_color_res_factor;
    }

  protected:
    /// normal constructor for heirs
    /// if heirs know L or l2crf right at creation, give some default parameters
    height_generator(double L = 1.0, unsigned l2crf = 1) :
        sample_spacing(L), log2_color_res_factor(l2crf)
    {
    }

    double sample_spacing; // equal to "L" value of geoclipmap renderer
    // fixme: is this still needed?
    unsigned log2_color_res_factor; // colors have 2^x more values as vertices

    std::unique_ptr<texture> sand_texture, mud_texture, forest_texture,
        grass_texture, rock_texture, snow_texture, forest_brdf_texture,
        rock_brdf_texture, base_texture, noise_texture;
    float tex_stretch_factor{0.01};
};
