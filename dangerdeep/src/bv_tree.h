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
//  A bounding volume tree (spheres) (C)+(W) 2009 Thorsten Jordan
//

#ifndef BV_TREE_H
#define BV_TREE_H

#include <array>
#include <list>		// note! vector could be faster nowadays.
#include <memory>
#include <vector>
#include "sphere.h"
#include "matrix4.h"

/// a binary tree representing a bounding volume hierarchy
class bv_tree
{
 public:
	/// data representing a triangle for tree construction
	struct leaf_data
	{
		std::array<uint32_t, 3> tri_idx;
		leaf_data() { tri_idx[0] = tri_idx[1] = tri_idx[2] = uint32_t(-1); }
		const vector3f& get_pos(const std::vector<vector3f>& vertices, unsigned corner) const {
			return vertices[tri_idx[corner]];
		}
		vector3f get_center(const std::vector<vector3f>& vertices) const { return (get_pos(vertices, 0) + get_pos(vertices, 1) + get_pos(vertices, 2)) * (1.f/3); }
	};

	/// parameters for collision
	struct param
	{
		const bv_tree& tree;
		const std::vector<vector3f>& vertices;
		matrix4f transform;
		param(const bv_tree& t, const std::vector<vector3f>& v, const matrix4f& m)
			: tree(t), vertices(v), transform(std::move(m)) {}
		param children(unsigned i) const {
			return param(*tree.children[i], vertices, transform);
		}
		spheref get_transformed_sphere() const {
			return spheref(transform.mul4vec3xlat(tree.volume.center), tree.volume.radius);
		}
		unsigned get_index_of_closer_child(const vector3f& pos) const {
			if (tree.is_leaf())
				return 2; // invalid index
			vector3f cp0 = transform.mul4vec3xlat(tree.children[0]->volume.center);
			vector3f cp1 = transform.mul4vec3xlat(tree.children[1]->volume.center);
			return (cp0.square_distance(pos) < cp1.square_distance(pos)) ? 0 : 1;
		}
	};

	bv_tree(const spheref& sph, const leaf_data& ld)
		: volume(sph), leafdata(ld) {}
	bv_tree(const spheref& sph, std::unique_ptr<bv_tree> left_tree, std::unique_ptr<bv_tree> right_tree);
	static std::unique_ptr<bv_tree> create(const std::vector<vector3f>& vertices, std::list<leaf_data>& nodes);
	bool is_inside(const vector3f& v) const;

	/** determine if two bv_trees intersect each other (are colliding). A list of contact points is computed. */
	static bool collides(const param& p0, const param& p1, std::list<vector3f>& contact_points);
	/** determine if two bv_trees intersect each other (are colliding). The closest contact point is computed. */
	static bool closest_collision(const param& p0, const param& p1, vector3f& contact_point);
	/** determine if two bv_trees intersect each other (are colliding). */
	static bool collides(const param& p, const spheref& sp);
	void transform(const matrix4f& mat);
	void compute_min_max(vector3f& minv, vector3f& maxv) const;
	void debug_dump(unsigned level = 0) const;
	const spheref& get_sphere() const { return volume; }
	void collect_volumes_of_tree_depth(std::list<spheref>& volumes, unsigned depth) const;
	bool is_leaf() const { return children[0].get() == nullptr; }

 protected:
	spheref volume;
	leaf_data leafdata;
	std::unique_ptr<bv_tree> children[2];

 private:
	bv_tree() = delete;
	bv_tree(const bv_tree& ) = delete;
	bv_tree(bv_tree&& ) = delete;
	bv_tree& operator= (const bv_tree& ) = delete;
	bv_tree& operator= (bv_tree&& ) = delete;
};

#endif
