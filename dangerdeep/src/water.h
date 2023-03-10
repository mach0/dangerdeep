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

// (ocean) water simulation and display (OpenGL)
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#pragma once

/*
    This class simulates and displays the water.
    Wave with animation, Fresnel etc.
*/

#include "angle.h"
#include "color.h"
#include "framebufferobject.h"
#include "ocean_wave_generator.h"
#include "shader.h"
#include "ship.h"
#include "texture.h"
#include "thread.h"
#include "vector3.h"
#include "vertexbufferobject.h"

#include <memory>
#include <vector>

///\brief Rendering of ocean water surfaces.
class water
{
  protected:
    double mytime; // store global time in seconds

    const unsigned wave_phases;  // >= 256 is a must
    const float wavetile_length; // >= 512m makes wave look MUCH more realistic
    const float wavetile_length_rcp;  // reciprocal of former value
    const double wave_tidecycle_time; // depends on fps. with 25fps and 256
                                      // phases, use ~10seconds.

    std::unique_ptr<texture> reflectiontex;
    std::unique_ptr<texture> foamtex;
    std::unique_ptr<texture> foamamounttex;
    std::unique_ptr<texture> foamamounttrail;
    std::unique_ptr<texture> foamperimetertex;
    std::unique_ptr<texture>
        fresnelcolortex; // texture for fresnel values and water color

    std::unique_ptr<framebufferobject> reflectiontex_fbo;
    std::unique_ptr<framebufferobject> foamamounttex_fbo;

    std::unique_ptr<texture>
        waterspecularlookup; // lookup 1d texture map for water specular term

    std::vector<uint8_t> fresnelcolortexd; // stored for updates of water color

    colorf last_light_color; // used to determine when new refraction color tex
                             // must get computed

    const unsigned
        wave_resolution; // fft resolution for water tile (use 64+, better 128+)
    const unsigned wave_resolution_shift; // the log2 of wave_resolution

    struct wavetile_phase
    {
        struct mipmap_level
        {
            unsigned resolution;
            unsigned resolution_shift;
            double sampledist;
            std::vector<vector3f> wavedata;
            std::vector<vector3f> normals;
            std::vector<float> amount_of_foam;
            std::vector<uint8_t> normals_tex;
            ///> generate data from downsampled version of wd
            mipmap_level(
                const std::vector<vector3f>& wd,
                unsigned res_shift,
                double sampledist);
            ///> create data from displacements and heights (mostly for level 0)
            mipmap_level(
                const std::vector<vector2f>& displacements,
                const std::vector<float>& heights,
                unsigned res_shift,
                double sampledist);
            [[nodiscard]] const vector3f& get_data(unsigned x, unsigned y) const
            {
                return wavedata[(y << resolution_shift) + x];
            }
            [[nodiscard]] const vector3f&
            get_normal(unsigned x, unsigned y) const
            {
                return normals[(y << resolution_shift) + x];
            }
            void compute_normals();
            void debug_dump(); // used only for debugging
        };

        [[nodiscard]] float get_height(unsigned idx) const
        {
            return mipmaps.front().wavedata[idx].z;
        }

        std::vector<mipmap_level> mipmaps;
        float minh{0}, maxh{0};

        wavetile_phase() = default;
    };

    // wave tile data
    std::vector<wavetile_phase> wavetile_data;
    const wavetile_phase* curr_wtp{nullptr}; // pointer to current phase

    // test
    ocean_wave_generator<float> owg;

    // with fragment programs we need some sub-noise
    std::unique_ptr<texture> water_bumpmap;

    // Config options (only used when supported)
    bool use_hqsfx{false}; // high quality special effects.

    // Shader program
    std::unique_ptr<glsl_shader_setup> glsl_water;
    std::unique_ptr<glsl_shader_setup> glsl_under_water;
    // locations of uniforms for shaders
    unsigned loc_w_noise_xform_0;
    unsigned loc_uw_noise_xform_0;
    unsigned loc_w_noise_xform_1;
    unsigned loc_uw_noise_xform_1;
    unsigned loc_w_reflection_mvp;
    unsigned loc_w_viewpos;
    unsigned loc_uw_viewpos;
    unsigned loc_w_upwelltop;
    unsigned loc_uw_upwelltop;
    unsigned loc_w_upwellbot;
    unsigned loc_uw_upwellbot;
    unsigned loc_w_upwelltopbot;
    unsigned loc_uw_upwelltopbot;
    unsigned loc_w_tex_normal;
    unsigned loc_uw_tex_normal;
    unsigned loc_w_tex_reflection;
    unsigned loc_w_tex_foam;
    unsigned loc_w_tex_foamamount;
    unsigned loc_w_foam_transform;
    unsigned loc_w_reflection_transform;

    // indices for vertex attributes
    unsigned vattr_aof_index{0};

    // avoid unnecessary vertex generation
    mutable bool rerender_new_wtp{true};
    mutable vector3 rerender_viewpos;

    water& operator=(const water& other);
    water(const water& other);

    void setup_textures(
        const matrix4& reflection_projmvmat,
        const vector2f& transl,
        bool under_water) const;
    void cleanup_textures() const;

    vector3f get_wave_normal_at(unsigned x, unsigned y) const;

    void compute_amount_of_foam();
    void generate_wavetile(
        ocean_wave_generator<float>& myowg,
        double tiletime,
        wavetile_phase& wtp);
    void generate_subdetail_texture();

    // --------------- geoclipmap stuff
    const unsigned geoclipmap_resolution;
    const unsigned geoclipmap_levels;
    class geoclipmap_patch
    {
        vertexbufferobject vbo;
        unsigned min_vertex_index;
        unsigned max_vertex_index;
        unsigned nr_indices;
        bool use_fan;

      public:
        geoclipmap_patch(
            unsigned geoclipmap_resolution, // "N"
            unsigned level,
            unsigned border,
            unsigned xoff,
            unsigned yoff,
            unsigned columns,
            unsigned rows);
        // generate horizon patch
        geoclipmap_patch(
            unsigned geoclipmap_resolution, // "N"
            unsigned highest_level,
            unsigned border);
        void render() const;
        [[nodiscard]] unsigned get_nr_indices() const
        {
            return nr_indices;
        } // for analysis

      private:
        geoclipmap_patch()                        = delete;
        geoclipmap_patch(const geoclipmap_patch&) = delete;
        geoclipmap_patch& operator=(const geoclipmap_patch&) = delete;
    };
    std::vector<std::unique_ptr<geoclipmap_patch>> patches;
    mutable vertexbufferobject vertices;

    class worker : public ::thread
    {
        water& wa;
        ocean_wave_generator<float> owg;
        unsigned ps, pa;

      public:
        worker(water& w, unsigned s, unsigned a) :
            thread("waterwrk"), wa(w), owg(w.owg), ps(s), pa(a)
        {
        }
        void loop() override
        {
            wa.construction_threaded(owg, ps, pa);
            request_abort();
        }
    };

    void construction_threaded(
        ocean_wave_generator<float>& myowg,
        unsigned phase_start,
        unsigned phase_add);

  public:
    water(double tm = 0.0); // give day time in seconds

    /// MUST be called after construction of water and before using it!
    void finish_construction();

    void set_time(double tm);

    void draw_foam_for_ship(
        const game& gm,
        const ship* shp,
        const vector3& viewpos) const;
    void compute_amount_of_foam_texture(
        const game& gm,
        const vector3& viewpos,
        const std::vector<const ship*>& allships) const;

    // give absolute position of viewer as viewpos, but modelview matrix without
    // translational component!
    void display(
        const vector3& viewpos,
        double max_view_dist,
        bool under_water = false) const;
    float get_height(const vector2& pos) const;
    // give f as multiplier for difference to (0,0,1)
    vector3f get_normal(const vector2& pos, double f = 1.0) const;
    static float exact_fresnel(float x);
    void set_refraction_color(const colorf& light_color);

    /// prepare reflection texture for mirror drawing
    void refltex_render_bind() const;

    /// finish mirror drawing
    void refltex_render_unbind() const;
};
