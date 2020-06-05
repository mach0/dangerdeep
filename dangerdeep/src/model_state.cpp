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

// A 3d model state
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "helper.h"
#include "model_state.h"

/// Default c'tor
model_state::model_state()
	: mymodel(nullptr)
{
}



/// Construct with given model
model_state::model_state(const model& m, const std::string& layout_)
	: mymodel(&m)
	, layout(layout_)
	, object_parameters(m.get_nr_of_objects())
	, transformation(matrix4::one())	// set neutral transformation
{
	for (unsigned i = 0; i < m.get_nr_of_objects(); ++i) {
		object_parameters[i] = m.get_object_transformation_parameters(i);
	}
	// Check that requested skin is valid for the model!
	auto all_layouts = m.get_all_layout_names();
	if (!helper::contains(all_layouts, layout)) {
		THROW(error, std::string("layout ") + layout + " not known in model");
	}
}



/// Set animation values for this model's object
void model_state::set_object_parameters(unsigned object_id, float translation, float angle)
{
	if (object_id >= object_parameters.size()) {
		THROW(error, "invalid object id");
	}
	// note: check constraints of model - would be good, but computation of transformation checks it anyway
	object_parameters[object_id] = vector2f(translation, angle);
}



/// Set general transformation for model
void model_state::compute_transformation(const vector3& position, const quaternion& orientation)
{
	// create matrix with translational part and rotational part (don't apply rotation on translation!)
	transformation = matrix4::trans(position) * orientation.rotmat4();
}



/// Request object parameters
const auto& model_state::get_object_parameters(unsigned object_id) const
{
	if (object_id >= object_parameters.size()) {
		THROW(error, "invalid object id");
	}
	return object_parameters[object_id];
}



/// Check for collision of line with model, optionally return position of first collision along line
bool model_state::check_for_collision(const vector3& start, const vector3& end, vector3* collision_pos) const
{
	// fixme recursivly iterate over objects and accumulate their transformations to get final transformation.
	// then check bv_tree of every mesh with that transformation for collision with line.
	// sort all collision positions along line and return first.
	// fixme store transformation per object by computing it once?
	// we need to know all leave node objects with their transformations at least and should have a compute function here!
	return false;
}

#if 0
/// compute bounds of object fixme obsolete, to model_state!
boxf model::object::compute_bounds(const matrix4f& transmat, const std::vector<mesh>& meshes) const
{
	matrix4f mytransmat = transmat * get_transformation();
	// handle vertices of mymesh if present
	boxf result;
	result.extend(meshes[mesh_index].compute_bounds(mytransmat));
	// handle children
	for (const auto& elem : children) {
		result.extend(elem.compute_bounds(mytransmat, meshes));
	}
	return result;
}
#endif



/// Get transformation of the object itself without parent transformation
matrix4 model_state::get_object_local_transformation(unsigned object_index) const
{
	return mymodel->get_object_local_transformation(object_index, get_object_parameters(object_index));
}
