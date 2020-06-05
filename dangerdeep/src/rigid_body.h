/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2016  Thorsten Jordan, Luis Barrancos and others.

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

// physical rigid body
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef RIGID_BODY_H
#define RIGID_BODY_H

#include "units.h"
class xml_elem;

/// a physical rigid body with simulation
class rigid_body
{
public:
	//
	// ---------------- rigid body variables ----------------
	//
	vector3 position;		// position, [SAVE]
	momentum3d linear_momentum;	// l.m./impulse ("P") P = M * v [SAVE], world space!
	quaternion orientation;		// orientation, [SAVE]
	momentum3d angular_momentum;	// angular momentum ("L") L = I * w = R * I_k * R^T * w [SAVE], world space!
	mass1d mass;			// total weight, later read from spec file (kg)
	matrix3 inertia_tensor;		// object local (I_k). [could be a reference into a model object...]
	matrix3 inertia_tensor_inv;	// object local (I_k), inverse of inertia tensor.

	// ------------- computed from rigid body variables ----------------
	velocity3d velocity;	// world space velocity
	angular_velocity turn_velocity;	// angular velocity around local z-axis (mathematical CCW)
	angular_velocity pitch_velocity;	// angular velocity around local x-axis (mathematical CCW)
	angular_velocity roll_velocity;	// angular velocity around local y-axis (mathematical CCW)
	angle heading;		// global z-orientation is stored additionally
	velocity3d local_velocity;	// recomputed every frame by simulate() method

	force3d compute_default_gravity_force() const;

	void set_mass_and_inertia_tensor(mass1d mass_, const matrix3& inertia_tensor = matrix3::one());

	/// recomputes *_velocity, heading etc.
	void compute_helper_values();

	/// Load data
	void load(const xml_elem& parent);
	/// Save data
	void save(xml_elem& parent) const;

	void create_storage_definition(class data_node& parent);

	void simulate(duration delta_time, std::initializer_list<std::pair<vector3, vector3>> local_forces);
	
	velocity3d compute_linear_velocity(const vector3& position_global) const;
	double compute_collision_response_value(const vector3& collision_pos, const vector3& N) const;
	void apply_collision_impulse(const vector3& collision_pos, const momentum3d& J);

	void manipulate_position(const vector3& newpos);
	void manipulate_speed(velocity1d localforwardspeed);
	void manipulate_heading(angle hdg);

protected:
	void simulate(duration delta_time, const force3d& force, const torque3d& torque);
};

#endif
