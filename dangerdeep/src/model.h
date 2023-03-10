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

// A 3d model
// (C)+(W) by Thorsten Jordan. See LICENSE

#pragma once

#include "bv_tree.h"
#include "color.h"
#include "matrix3.h"
#include "matrix4.h"
#include "shader.h"
#include "texture.h"
#include "vector3.h"
#include "vertexbufferobject.h"

#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

class xml_elem;

#define DFTD_MAX_TEXTURE_UNITS 8

///\brief Handles a 3D model, it's animation and OpenGL based rendering and
/// display.
class model
{
  public:
    using ptr = std::unique_ptr<model>;

    class material
    {
        material(const material&) = delete;
        material& operator=(const material&) = delete;

      public:
        class map
        {
            map(const map&) = delete;
            map& operator=(const map&) = delete;

          public:
            std::string filename; // also in mytexture, a bit redundant

          protected:
            texture* tex{nullptr}; // set by set_layout

            // maybe unite list of skins and default-texture, both
            // have texture ptr, filename and ref_count.
            std::unique_ptr<texture> mytexture; // default "skin", MUST BE SET!
            unsigned ref_count{0};

            struct skin
            {
                texture* mytexture{nullptr};
                unsigned ref_count{0};
                std::string filename;
                skin() = default;
            };

            // layout-name to skin mapping
            std::map<std::string, skin> skins;

          public:
            map();
            ~map();
            void write_to_dftd_model_file(
                xml_elem& parent,
                const std::string& type) const;
            // read and construct from dftd model file
            map(const xml_elem& parent);
            // set up opengl texture matrix with map transformation values
            void set_gl_texture() const;
            void set_gl_texture(
                const glsl_program& prog,
                unsigned loc,
                unsigned texunitnr) const;
            void set_gl_texture(
                const glsl_shader_setup& gss,
                unsigned loc,
                unsigned texunitnr) const;
            void set_texture(texture* t);
            void register_layout(
                const std::string& name,
                const std::string& basepath,
                texture::mapping_mode mapping,
                bool makenormalmap = false,
                float detailh      = 1.0f,
                bool rgb2grey      = false);
            void unregister_layout(const std::string& name);
            void set_layout(const std::string& layout);
            void get_all_layout_names(std::set<std::string>& result) const;
        };

        std::string name;
        color diffuse;  // only used when colormap is 0.
        color specular; // material specular color, used with and without
                        // texture mapping.
        float shininess{50.0f}; // shininess (Halfangle dot Normal exponent)
        std::unique_ptr<map> colormap; // replaces diffuse color if not defined.
        std::unique_ptr<map>
            normalmap; // should be of type RGB to work properly.
        std::unique_ptr<map>
            specularmap; // should be of type LUMINANCE to work properly.
        bool two_sided{false};

        material(std::string nm = "Unnamed material");
        virtual ~material() = default;
        virtual void set_gl_values(const texture* caustic_map = nullptr) const;
        virtual void set_gl_values_mirror_clip() const;
        virtual void
        register_layout(const std::string& name, const std::string& basepath);
        virtual void unregister_layout(const std::string& name);
        virtual void set_layout(const std::string& layout);
        virtual void get_all_layout_names(std::set<std::string>& result) const;
        [[nodiscard]] virtual bool needs_texcoords() const
        {
            return colormap.get() != nullptr;
        }
        [[nodiscard]] virtual bool use_default_shader() const { return true; }
    };

    class material_glsl : public material
    {
        material_glsl() = delete;
        std::string vertexshaderfn, fragmentshaderfn;
        glsl_shader_setup shadersetup;

      public:
        material_glsl(
            const std::string& nm,
            const std::string& vsfn,
            const std::string& fsfn);
        void set_gl_values(const texture* caustic_map = nullptr) const override;
        void set_gl_values_mirror_clip() const override;
        void register_layout(
            const std::string& name,
            const std::string& basepath) override;
        void unregister_layout(const std::string& name) override;
        void set_layout(const std::string& layout) override;
        void get_all_layout_names(std::set<std::string>& result) const override;
        void compute_texloc();
        [[nodiscard]] const std::string& get_vertexshaderfn() const
        {
            return vertexshaderfn;
        }
        [[nodiscard]] const std::string& get_fragmentshaderfn() const
        {
            return fragmentshaderfn;
        }
        [[nodiscard]] bool needs_texcoords() const override
        {
            return nrtex > 0;
        }
        [[nodiscard]] bool use_default_shader() const override { return false; }
        glsl_shader_setup& get_shadersetup() { return shadersetup; }

        std::unique_ptr<map>
            texmaps[DFTD_MAX_TEXTURE_UNITS]; // up to DFTD_MAX_TEXTURE_UNITS
                                             // texture units
        std::string texnames[DFTD_MAX_TEXTURE_UNITS];
        unsigned loc_texunit[DFTD_MAX_TEXTURE_UNITS];
        unsigned nrtex;
    };

    class mesh
    {
      public:
        // translate primitive_type to GL enum
        int gl_primitive_type() const;
        const char* name_primitive_type() const;

        // meshes can be made of triangles (default) or other common primitives.
        enum primitive_type
        {
            pt_triangles,
            pt_triangle_strip
        };

        // classes to iterate over triangles for all primitive types
        class triangle_iterator
        {
          protected:
            unsigned _i0, _i1, _i2;
            const std::vector<uint32_t>& idx;
            unsigned ptr;

          public:
            triangle_iterator(const std::vector<uint32_t>& indices);
            virtual ~triangle_iterator() = default;
            [[nodiscard]] unsigned i0() const { return _i0; }
            [[nodiscard]] unsigned i1() const { return _i1; }
            [[nodiscard]] unsigned i2() const { return _i2; }
            virtual bool next();
        };

        class triangle_strip_iterator : public triangle_iterator
        {
          public:
            triangle_strip_iterator(const std::vector<uint32_t>& indices);
            bool next() override;
        };

        std::unique_ptr<triangle_iterator> get_tri_iterator() const;

        std::string name;
        // This data is NOT needed for rendering, as it is stored also in VBOs,
        // except for the old pipeline where we need the data (or part of it),
        // to compute lighting values per frame.
        std::vector<vector3f> vertices;
        std::vector<vector3f> normals;
        std::vector<vector3f> tangentsx;
        std::vector<vector2f> texcoords;
        // we can have a right handed or a left handed coordinate system for
        // each vertex, dependent on the direction of the u,v coordinates. We do
        // not store the third vector per vertex but a flag, which saves space.
        // fixme: research if we can have a right-handed system always. This
        // would save fetching the colors to the vertex shader and thus spare
        // memory bandwidth.
        std::vector<uint8_t>
            righthanded; // a vector of bools. takes more space than a
                         // bitvector, but faster access.
        std::vector<uint32_t> indices; // 3 indices per face
        material* mymaterial;
        vector3f min, max;
        // OpenGL VBOs for the data
        vertexbufferobject vbo_positions;
        vertexbufferobject vbo_normals;
        vertexbufferobject vbo_texcoords;
        vertexbufferobject vbo_tangents_righthanded;
        mutable vertexbufferobject
            vbo_colors; // mutable because non-shader pipeline writes to it
        vertexbufferobject index_data;
        unsigned vertex_attrib_index;
        matrix3 inertia_tensor;
        double volume;

        unsigned get_nr_of_triangles() const;
        void get_triangle(unsigned triangle, uint32_t indices[3]) const
        {
            ((*this).*(get_triangle_ptr))(triangle, indices);
        }

        void display(const texture* caustic_map = nullptr) const;
        void display_mirror_clip() const;
        void compute_vertex_bounds();
        void compute_bounds(
            vector3f& totmin,
            vector3f& totmax,
            const matrix4f& transmat);
        void compute_normals();
        bool compute_tangentx(unsigned i0, unsigned i1, unsigned i2);

        mesh(std::string nm);

        /// create mesh from height map - around world origin
        ///@param w - width of 2d field's data values
        ///@param h - height of 2d field's data values
        ///@param heights - height values
        ///@param scales - scalars for x,y,z values
        ///@param trans - translation for each vertex
        ///@nm - name
        mesh(
            unsigned w,
            unsigned h,
            const std::vector<float>& heights,
            const vector3f& scales,
            const vector3f& trans = vector3f(),
            std::string nm        = "Heightfield");

        // make display list if possible
        void compile();

        // transform vertices by matrix
        void transform(const matrix4f& m);
        void write_off_file(const std::string& fn) const;

        // give plane equation (abc must have length 1)
        std::pair<mesh*, mesh*> split(const vector3f& abc, float d) const;

        /// check if a given point is inside the mesh
        ///@param p - point in vertex space, transformation not applied
        bool is_inside(const vector3f& p) const;
        double compute_volume() const;
        vector3 compute_center_of_gravity() const;
        /// give transformation matrix for vertices here (vertex->world space)
        matrix3 compute_inertia_tensor(const matrix4f& transmat) const;

        /// Adjacency information - computed on demand - only for triangles
        std::vector<uint32_t> triangle_adjacency;
        std::vector<uint32_t> vertex_triangle_adjacency;

        static const unsigned no_adjacency; // = unsigned(-1);
        bool has_adjacency_info() const;
        void compute_adjacency();
        bool check_adjacency() const;
        unsigned get_adjacent_triangle(unsigned triangle, unsigned edge) const
        {
            return triangle_adjacency[triangle * 3 + edge];
        }
        unsigned get_triangle_of_vertex(unsigned vertex) const
        {
            return vertex_triangle_adjacency[vertex];
        }

        void compute_bv_tree();
        bool has_bv_tree() const { return !bounding_volume_tree.empty(); }
        const bv_tree& get_bv_tree() const { return bounding_volume_tree; }

        void get_plain_triangle(unsigned triangle, uint32_t indices[3]) const;
        void get_strip_triangle(unsigned triangle, uint32_t indices[3]) const;

        void set_indices_type(primitive_type pt);
        primitive_type get_indices_type() const { return indices_type; }

        /// slow intersection test on triangle-triangle tests
        bool intersects(
            const mesh& other,
            const matrix4f& transformation_this_to_other) const;

        /// check wether a triangle is degenerated
        static bool is_degenerated(
            const vector3f& v0,
            const vector3f& v1,
            const vector3f& v2,
            const float eps = 1e-3f);

      protected:
        primitive_type indices_type;
        bv_tree bounding_volume_tree;
        void (model::mesh::*get_triangle_ptr)(
            unsigned triangle,
            uint32_t indices[3]) const;

      private:
        mesh()            = delete;
        mesh(const mesh&) = delete;
        mesh& operator=(const mesh&) = delete;
    };

    /// voxel, the space of a model is partitioned in subspaces
    class voxel
    {
      public:
        /// position of center of voxel relative to the base mesh
        vector3f relative_position;
        /// part of voxel that is filled with model volume (0...1)
        float part_of_volume;
        /// third root of part_of_volume, used for collision detection
        float root3_part_of_volume;
        /// relative mass of the voxel of total mass (0...1)
        float relative_mass;
        /// relative volume of the voxel of total volume (0...1)
        float relative_volume;
        /// indices of neighbouring voxels: top, left, forward, right, backward,
        /// bottom -1 means no neighbour
        int neighbour_idx[6];
        /// construct a voxel
        voxel(const vector3f& rp, float pv, float m, float rv) :
            relative_position(rp), part_of_volume(pv),
            root3_part_of_volume(pow(pv, (float) (1.0 / 3.0))),
            relative_mass(m), relative_volume(rv)
        {
            for (int& i : neighbour_idx)
                i = -1;
        }
    };

  protected:
    // a 3d object, references meshes
    struct object
    {
        unsigned id;
        std::string name;
        mesh* mymesh;
        vector3f translation;
        int translation_constraint_axis; // can be 0/1/2 for x/y/z
        float trans_val_min; // minimum value for translation along axis
        float trans_val_max; // maximum value for translation along axis
        vector3f rotat_axis;
        float rotat_angle{0};     // in degrees
        float rotat_angle_min{0}; // in degrees
        float rotat_angle_max{0}; // in degrees
        std::vector<object> children;
        object(unsigned id_ = 0, std::string nm = "???", mesh* m = nullptr) :
            id(id_), name(std::move(nm)), mymesh(m)
        {
            rotat_axis.z = 1;
        }
        bool set_angle(float ang);
        bool set_translation(float value);
        object* find(unsigned id);
        object* find(const std::string& name);
        [[nodiscard]] const object* find(unsigned id) const;
        [[nodiscard]] const object* find(const std::string& name) const;
        void display(const texture* caustic_map = nullptr) const;
        void display_mirror_clip() const;
        void compute_bounds(
            vector3f& min,
            vector3f& max,
            const matrix4f& transmat) const;
        [[nodiscard]] matrix4f get_transformation() const;
    };

    // store that for debugging purposes.
    std::string filename;

    std::vector<material*> materials;
    std::vector<mesh*> meshes;

    object scene;

    std::string
        basename; // base name of the scene/model, computed from filename
    std::string
        basepath; // base path name of the scene/model, computed from filename

    vector3f min, max;
    double boundsphere_radius;

    std::string current_layout;

    // class-wide variables: shaders supported and enabled, shader number and
    // init count
    static unsigned init_count;

    // Shader programs
    static std::unique_ptr<glsl_shader_setup> glsl_plastic;
    static std::unique_ptr<glsl_shader_setup> glsl_color;
    static std::unique_ptr<glsl_shader_setup> glsl_color_normal;
    static std::unique_ptr<glsl_shader_setup> glsl_color_normal_caustic;
    static std::unique_ptr<glsl_shader_setup> glsl_color_normal_specular;
    static std::unique_ptr<glsl_shader_setup>
        glsl_color_normal_specular_caustic;
    static std::unique_ptr<glsl_shader_setup> glsl_mirror_clip;
    static unsigned loc_c_tex_color;
    static unsigned loc_cn_tex_normal;
    static unsigned loc_cn_tex_color;
    static unsigned loc_cnc_tex_normal;
    static unsigned loc_cnc_tex_color;
    static unsigned loc_cnc_tex_caustic;
    static unsigned loc_cns_tex_normal;
    static unsigned loc_cns_tex_color;
    static unsigned loc_cns_tex_specular;
    static unsigned loc_cnsc_tex_normal;
    static unsigned loc_cnsc_tex_color;
    static unsigned loc_cnsc_tex_specular;
    static unsigned loc_cnsc_tex_caustic;
    static unsigned loc_mc_tex_color;

    // init / deinit
    static void render_init();
    static void render_deinit();

    void compute_bounds();
    void compute_normals();

    std::vector<float> cross_sections; // array over angles

    /// nr of voxels in every dimension
    vector3i voxel_resolution;
    /// size of a voxel in 3-space
    vector3f voxel_size;
    /// "radius" of a voxel in 3-space
    float voxel_radius;
    /// total volume of model defined by voxels
    double total_volume_by_voxels;
    /// per voxel: relative 3d position and part of volume that is inside
    /// (0...1)
    std::vector<voxel> voxel_data;
    /// voxel for 3-space coordinate of it, -1 if not existing
    std::vector<int> voxel_index_by_pos;

    void read_phys_file(const std::string& filename);

    model(const model&);
    model& operator=(const model&);

    void read_off_file(const std::string& fn);

    void read_dftd_model_file(const std::string& filename);
    void write_color_to_dftd_model_file(
        xml_elem& parent,
        const color& c,
        const std::string& type) const;
    color read_color_from_dftd_model_file(
        const xml_elem& parent,
        const std::string& type);

    // store shared lookup functions for pow function (specular lighting)
    // fixme: check if exponent is integer or float.
    // map<double, texture*> powlookup_functions;

    void read_objects(const xml_elem& parent, object& parentobj);

  public:
    model();

    static texture::mapping_mode
        mapping; // GL_* mapping constants (default GL_LINEAR_MIPMAP_LINEAR)

    model(std::string filename, bool use_material = true);
    ~model();
    static const std::string default_layout;
    void set_layout(const std::string& layout = default_layout);
    // extend method by matrix4(f) for additional transformation, to avoid
    // that the user has to du glPushMatrix/manipulate/glPopMatrix
    void display(const texture* caustic_map = nullptr) const;
    /** display model but clip away coords with z < 0 in world space.
        @note! set up texture matrix for unit 1 so that it contains
        object to world-space transformation, and set up modelview
        matrix so that it contains worldspace to viewer transformation
        with z-mirroring.
    */
    void display_mirror_clip() const;
    mesh& get_mesh(unsigned nr);
    [[nodiscard]] const mesh& get_mesh(unsigned nr) const;
    /// get mesh at root of object tree or first mesh if no tree defined
    mesh& get_base_mesh();
    [[nodiscard]] const mesh& get_base_mesh() const;
    material& get_material(unsigned nr);
    [[nodiscard]] const material& get_material(unsigned nr) const;
    [[nodiscard]] unsigned get_nr_of_meshes() const { return meshes.size(); }
    [[nodiscard]] unsigned get_nr_of_materials() const
    {
        return materials.size();
    }
    [[nodiscard]] vector3f get_min() const { return min; }
    [[nodiscard]] vector3f get_max() const { return max; }
    [[nodiscard]] float get_length() const { return (max - min).y; }
    [[nodiscard]] float get_width() const { return (max - min).x; }
    [[nodiscard]] float get_height() const { return (max - min).z; }
    [[nodiscard]] vector3f get_boundbox_size() const { return max - min; }
    [[nodiscard]] float
    get_cross_section(float angle) const; // give angle in degrees.
    static std::string tolower(const std::string& s);
    void add_mesh(mesh* m)
    {
        meshes.push_back(m);
    } // fixme: maybe recompute bounds
    void add_material(material* m) { materials.push_back(m); }
    // transform meshes by matrix (attention: scaling destroys normals)
    void transform(const matrix4f& m);
    // compile display lists
    void compile();

    // write our own model file format.
    void write_to_dftd_model_file(
        const std::string& filename,
        bool store_normals = true) const;

    // manipulate object angle(s), returns false on error (wrong id or angle out
    // of bounds)
    bool set_object_angle(unsigned objid, double ang);
    bool set_object_angle(const std::string& objname, double ang);
    // get min/max angles of an object. returns 0/0 if object does not exist
    vector2f get_object_angle_constraints(unsigned objid);
    vector2f get_object_angle_constraints(const std::string& objname);

    // manipulate object translation, returns false on error (wrong id or value
    // out of bounds)
    bool set_object_translation(unsigned objid, double value);
    bool set_object_translation(const std::string& objname, double value);
    // get min/max translation values of an object. returns 0/0 if object does
    // not exist
    vector2f get_object_translation_constraints(unsigned objid);
    vector2f get_object_translation_constraints(const std::string& objname);

    // find object with certain name, -1 if not found
    [[nodiscard]] int get_object_id_by_name(const std::string& name) const;

    [[nodiscard]] bool object_exists(unsigned objid) const;

    void register_layout(const std::string& name = default_layout);
    void unregister_layout(const std::string& name = default_layout);

    // collect all possible layout names from all materials/maps and insert them
    // in "result"
    void get_all_layout_names(std::set<std::string>& result) const;

    [[nodiscard]] std::string get_filename() const { return filename; }

    /*
    /// check if a given point is inside the model
    ///@param p - a point in world space
    bool is_inside(const vector3f& p) const;
    */

    /// request voxel data resolution
    [[nodiscard]] const vector3i& get_voxel_resolution() const
    {
        return voxel_resolution;
    }
    /// request voxel size
    [[nodiscard]] const vector3f& get_voxel_size() const { return voxel_size; }
    /// request voxel "radius"
    [[nodiscard]] float get_voxel_radius() const { return voxel_radius; }
    /// request total volume by voxels
    [[nodiscard]] float get_total_volume_by_voxels() const
    {
        return total_volume_by_voxels;
    }
    /// request voxel data
    [[nodiscard]] const std::vector<voxel>& get_voxel_data() const
    {
        return voxel_data;
    }
    /// get voxel data by position, may return 0 for not existing voxels
    [[nodiscard]] const voxel* get_voxel_by_pos(const vector3i& v) const
    {
        int i = voxel_index_by_pos
            [(v.z * voxel_resolution.y + v.y) * voxel_resolution.x + v.x];
        return (i >= 0) ? &voxel_data[i] : (const voxel*) nullptr;
    }

    /// get transformation of root node (object tree translation + mesh
    /// transformation)
    [[nodiscard]] matrix4f get_base_mesh_transformation() const;

    /// get voxel closest to a real world position
    ///@returns voxel index of closest voxel
    unsigned get_voxel_closest_to(const vector3f& pos);

    /// get voxels within a sphere around a real world position
    ///@returns list of voxels which center is inside the sphere
    std::vector<unsigned>
    get_voxels_within_sphere(const vector3f& pos, double radius);

    // fixme: add function to check wether a point is inside a voxel (for hit
    // check) fixme: add function to check wether a line is inside the model
    // (for hit check) this is a bit problematic, as only the base mesh is
    // considered for voxel computation, thus hits against turrets etc. are not
    // checked... not good for battleship fights.

    /// get radius of bounding sphere
    [[nodiscard]] double get_bounding_sphere_radius() const
    {
        return boundsphere_radius;
    }
};
