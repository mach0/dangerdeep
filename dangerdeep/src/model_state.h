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

#pragma once

#include "model.h"
#include "quaternion.h"

/// Represents a 3D model state
class model_state
{
  public:
    model_state();
    model_state(const model& m, const std::string& layout_);
    void
    set_object_parameters(unsigned object_id, float translation, float angle);
    // fixme model state transformations must be computed once per frame!
    void compute_transformation(
        const vector3& position, const quaternion& orientation);
    const auto& get_layout() const { return layout; }
    const auto& get_transformation() const { return transformation; }
    const auto& get_object_parameters(unsigned object_id) const;
    bool check_for_collision(
        const vector3& start, const vector3& end,
        vector3* collision_pos = nullptr) const;
    matrix4 get_object_local_transformation(unsigned object_index) const;
    // fixme add method to check for collision with other model_state!
    // fixme why not store voxel variable data also here?!
    // fixme add collision checks of a point/line with voxels, line with bvtree
    // and bvtree with bvtree. boxf compute_bounds(const matrix4f& transmat,
    // const std::vector<mesh>& meshes) const; // fixme

  protected:
    /// Pointer to the model
    const model* mymodel;
    /// the selected model layout name
    std::string layout;
    /// per object ID and translation/angle
    std::vector<vector2f> object_parameters;
    /// the transformation matrix to use for the model, computed from object
    /// position/orientation
    matrix4 transformation;
};

