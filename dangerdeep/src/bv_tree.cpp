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

#include "bv_tree.h"
#include "error.h"
#include "triangle_intersection.h"
//#define DEBUG_OUTPUT
#ifdef DEBUG_OUTPUT
#include <iostream>
#endif

namespace
{
	unsigned create_bv_subtree(const std::vector<vector3f>& vertices, std::vector<bv_tree::node>& nodes, unsigned index_begin, unsigned index_end)
	{
		if (index_begin == index_end) {
			THROW(error, "bv_tree create on empty data");
		}
		// compute bounding box for leaves
		vector3f bbox_min = nodes[index_begin].get_pos(vertices, 0);
		vector3f bbox_max = bbox_min;
		for (auto index = index_begin; index < index_end; ++index) {
			auto& node = nodes[index];
			for (unsigned i = 0; i < 3; ++i) {
				bbox_min = bbox_min.min(node.get_pos(vertices, i));
				bbox_max = bbox_max.max(node.get_pos(vertices, i));
			}
		}
		// new sphere center is center of bbox
		spheref bound_sphere((bbox_min + bbox_max) * 0.5f, 0.0f);
		// compute sphere radius by vertex distances to center (more accurate than
		// approximating by bbox size)
		for (auto index = index_begin; index < index_end; ++index) {
			auto& node = nodes[index];
			for (unsigned i = 0; i < 3; ++i) {
				float r = node.get_pos(vertices, i).distance(bound_sphere.center);
				bound_sphere.radius = std::max(r, bound_sphere.radius);
			}
		}
		// if list has one entry, return that
		if (index_begin + 1 == index_end) {
			nodes[index_begin].volume = bound_sphere;
			return index_begin;
		}
		//
		// split leaf node list in two parts
		//
		vector3f deltav = bbox_max - bbox_min;
		// chose axis with longest value range, sort along that axis,
		// split in center of bound_sphere.
		unsigned split_axis = 0; // x - default
		if (deltav.y > deltav.x) {
			if (deltav.z > deltav.y) {
				split_axis = 2; // z
			} else {
				split_axis = 1; // y
			}
		} else if (deltav.z > deltav.x) {
			split_axis = 2; // z
		}
#ifdef DEBUG_OUTPUT
		std::cout << "Create subtree [" << index_begin << "..." << index_end << "[ deltav " << deltav << " split_axis=" << split_axis << "\n";
#endif
		float vcenter[3];
		bound_sphere.center.to_mem(vcenter);
		auto index_end_left = index_begin;
		auto index_begin_right = index_end;
		while (index_end_left < index_begin_right) {
			float vc[3];
			nodes[index_end_left].get_center(vertices).to_mem(vc);
			if (vc[split_axis] < vcenter[split_axis]) {
				// node is left, keep left of split and advance
				++index_end_left;
			} else if (index_end_left + 1 < index_begin_right) {
				// node is right, swap with last node in range that has no side defined and test again
				std::swap(nodes[index_end_left], nodes[index_begin_right-1]);
				--index_begin_right;
			} else {
				// last node in range is right
				--index_begin_right;
			}
		}
		if (index_begin == index_end_left || index_begin_right == index_end) {
			// special case: force division
			index_end_left = index_begin_right = (index_begin + index_end) / 2;
		}
		// Create subtrees for left and right part of nodes
		auto left_child_index = create_bv_subtree(vertices, nodes, index_begin, index_end_left);
		auto right_child_index = create_bv_subtree(vertices, nodes, index_begin_right, index_end);
		// Create new node as parent for the sub trees and use the bounding sphere over all nodes for it
		nodes.push_back({{left_child_index, right_child_index, bv_tree::node::invalid_index}, bound_sphere});
		return unsigned(nodes.size() - 1);
	}

	bool is_inside(const vector3f& v, const std::vector<bv_tree::node>& nodes, unsigned node_index)
	{
		auto& node = nodes[node_index];
		if (node.volume.is_inside(v)) {
			if (!node.is_leaf()) {
				// Check children
				for (unsigned i = 0; i < 2; ++i) {
					if (node.tri_idx[i] != bv_tree::node::invalid_index) {
						if (is_inside(v, nodes, node.tri_idx[i])) {
							return true;
						}
					}
				}
			}
			return true;
		}
		return false;
	}

	void collect_volumes_of_tree_depth(std::vector<spheref>& volumes, unsigned depth, const std::vector<bv_tree::node>& nodes, unsigned node_index)
	{
		auto& node = nodes[node_index];
		if (depth == 0) {
			volumes.push_back(node.volume);
			return;
		}
		if (!node.is_leaf()) {
			collect_volumes_of_tree_depth(volumes, depth - 1, nodes, node.tri_idx[0]);
			collect_volumes_of_tree_depth(volumes, depth - 1, nodes, node.tri_idx[1]);
		}
	}
}

bv_tree::bv_tree(const std::vector<vector3f>& vertices, std::vector<bv_tree::node>&& leaf_nodes)
 :	nodes(std::move(leaf_nodes))
{
	if (!nodes.empty()) {
		create_bv_subtree(vertices, nodes, 0, unsigned(nodes.size()));
	}
	// Note that ships and objects are mostly of box shape we could store an additional bounding box
	// For a bit more precise checking. It would even be sufficient to check for box intersections
	// ONLY!
	// However we have to apply the transformations to each box then and check for intersections which
	// is a bit of code but not too complicated.
	// However normally objects don't get that close to each other, so we don't need this acceleration.
}


bool bv_tree::is_inside(const vector3f& v) const
{
	return ::is_inside(v, nodes, unsigned(nodes.size() - 1));
}



bool bv_tree::collides(const param& p0, const param& p1, std::vector<vector3f>& contact_points)
{
	// if bounding volumes do not intersect, there can't be any collision of leaf elements
	auto transformed_volume0 = p0.get_transformed_volume();
	auto transformed_volume1 = p1.get_transformed_volume();
	if (!transformed_volume0.intersects(transformed_volume1)) {
		return false;
	}

	// handle case that this is a leaf node
	if (p0.is_leaf()) {
		// we have a leaf node
		if (p1.is_leaf()) {
			// direct face to face collision test
			// handle transform here
			vector3f v0t = p0.transform.mul4vec3xlat(p0.vertices[p0.get_node().tri_idx[0]]);
			vector3f v1t = p0.transform.mul4vec3xlat(p0.vertices[p0.get_node().tri_idx[1]]);
			vector3f v2t = p0.transform.mul4vec3xlat(p0.vertices[p0.get_node().tri_idx[2]]);
			vector3f v3t = p1.transform.mul4vec3xlat(p1.vertices[p1.get_node().tri_idx[0]]);
			vector3f v4t = p1.transform.mul4vec3xlat(p1.vertices[p1.get_node().tri_idx[1]]);
			vector3f v5t = p1.transform.mul4vec3xlat(p1.vertices[p1.get_node().tri_idx[2]]);
			// note that degenerated triangles would be a critical problem here, but
			// they would have a bounding sphere of radius zero and thus we
			// never would compare with them, so we don't need to check for them
			// here.
			bool c = triangle_intersection::compute<float>(v0t, v1t, v2t, v3t, v4t, v5t);
			if (c) {
				// fixme: compute more accurate position here, maybe
				// weight by triangle area between centers of triangles.
				contact_points.push_back((v0t+v1t+v2t+v3t+v4t+v5t)*(1.f/6));
			}
			return c;
		} else {
			// other node is no leaf, recurse there, swap roles of this and other
			bool col1 = collides(p1.children(0), p0, contact_points);
			bool col2 = collides(p1.children(1), p0, contact_points);
			return col1 || col2;
		}
	}

	// split larger volume of this and other, go recursivly down all children
	if (p0.get_node().volume.radius > p1.get_node().volume.radius || p1.is_leaf()) {
		// recurse this node, so don't swap roles of this and other
		bool col1 = collides(p0.children(0), p1, contact_points);
		bool col2 = collides(p0.children(1), p1, contact_points);
		return col1 || col2;
	} else {
		// recurse other node - other is no leaf here, swap roles of this and other
		bool col1 = collides(p1.children(0), p0, contact_points);
		bool col2 = collides(p1.children(1), p0, contact_points);
		return col1 || col2;
	}
}



bool bv_tree::closest_collision(const param& p0, const param& p1, vector3f& contact_point)
{
	// if bounding volumes do not intersect, there can't be any collision of leaf elements
	auto transformed_volume0 = p0.get_transformed_volume();
	auto transformed_volume1 = p1.get_transformed_volume();
	if (!transformed_volume0.intersects(transformed_volume1)) {
		return false;
	}

	// handle case that this is a leaf node
	if (p0.is_leaf()) {
		// we have a leaf node
		if (p1.is_leaf()) {
			// direct face to face collision test
			// handle transform here
			vector3f v0t = p0.transform.mul4vec3xlat(p0.vertices[p0.get_node().tri_idx[0]]);
			vector3f v1t = p0.transform.mul4vec3xlat(p0.vertices[p0.get_node().tri_idx[1]]);
			vector3f v2t = p0.transform.mul4vec3xlat(p0.vertices[p0.get_node().tri_idx[2]]);
			vector3f v3t = p1.transform.mul4vec3xlat(p1.vertices[p1.get_node().tri_idx[0]]);
			vector3f v4t = p1.transform.mul4vec3xlat(p1.vertices[p1.get_node().tri_idx[1]]);
			vector3f v5t = p1.transform.mul4vec3xlat(p1.vertices[p1.get_node().tri_idx[2]]);
			// note that degenerated triangles would be a critical problem here, but
			// they would have a bounding sphere of radius zero and thus we
			// never would compare with them, so we don't need to check for them
			// here.
			bool c = triangle_intersection::compute<float>(v0t, v1t, v2t, v3t, v4t, v5t);
			if (c) {
				// fixme: compute more accurate position here, maybe
				// weight by triangle area between centers of triangles.
				contact_point = (v0t+v1t+v2t+v3t+v4t+v5t)*(1.f/6);
			}
			return c;
		} else {
			// other node is no leaf, recurse there, swap roles of this and other
			// use logical or to return on first true result
			unsigned i = p1.get_index_of_closer_child(transformed_volume0.center);
			return closest_collision(p1.children(i), p0, contact_point) ||
				closest_collision(p1.children(1-i), p0, contact_point);
		}
	}

	// split larger volume of this and other, go recursivly down all children
	if (p0.get_node().volume.radius > p1.get_node().volume.radius || p1.is_leaf()) {
		// recurse this node, so don't swap roles of this and other
		// use logical or to return on first true result
		unsigned i = p0.get_index_of_closer_child(transformed_volume1.center);
		return closest_collision(p0.children(i), p1, contact_point) ||
			closest_collision(p0.children(1-i), p1, contact_point);
	} else {
		// recurse other node - other is no leaf here, swap roles of this and other
		// use logical or to return on first true result
		unsigned i = p1.get_index_of_closer_child(transformed_volume0.center);
		return closest_collision(p1.children(i), p0, contact_point) ||
			closest_collision(p1.children(1-i), p0, contact_point);
	}
}


bool bv_tree::collides(const param& p, const spheref& sp)
{
	// if bounding volumes do not intersect, there can't be any collision of leaf elements
	auto transformed_volume = p.get_transformed_volume();
	if (!transformed_volume.intersects(sp)) {
		return false;
	}

	// handle case that this is a leaf node
	if (p.is_leaf()) {
		// leaf's bounding sphere and sp intersect, so we have a collision
		return true;
	}

	// split larger volume of this, go recursivly down all children
	// use logical or to return on first true result
	unsigned i = p.get_index_of_closer_child(sp.center);
	return collides(p.children(i), sp) || collides(p.children(1-i), sp);
}



void bv_tree::transform(const matrix4f& mat)
{
	for (auto& node : nodes) {
		node.volume.center = mat.mul4vec3xlat(node.volume.center);
	}
}



void bv_tree::compute_min_max(vector3f& minv, vector3f& maxv) const
{
	for (auto& node : nodes) {
		node.volume.compute_min_max(minv, maxv);
	}
}



void bv_tree::collect_volumes_of_tree_depth(std::vector<spheref>& volumes, unsigned depth) const
{
	::collect_volumes_of_tree_depth(volumes, depth, nodes, unsigned(nodes.size() - 1));
}
