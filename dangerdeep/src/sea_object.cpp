// sea objects
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sea_object.h"
#include "vector2.h"
#include "tokencodes.h"
#include "sensors.h"
#include "model.h"
#include "global_data.h"
#include "tinyxml/tinyxml.h"
#include "system.h"
#include "texts.h"
#include "ai.h"
#include "game.h"


const double earthperimeter2 = 20015086.795;


void sea_object::degrees2meters(bool west, unsigned degx, unsigned minx, bool south,
	unsigned degy, unsigned miny, double& x, double& y)
{
	x = (west ? -1.0 : 1.0)*(double(degx)+double(minx)/60.0)*earthperimeter2/180.0;
	y = (south ? -1.0 : 1.0)*(double(degy)+double(miny)/60.0)*earthperimeter2/180.0;
}



void sea_object::meters2degrees(double x, double y, bool& west, unsigned& degx, unsigned& minx, bool& south,
	unsigned& degy, unsigned& miny)
{
	double fracdegrx = fabs(x*180.0/earthperimeter2);
	double fracdegry = fabs(y*180.0/earthperimeter2);
	degx = unsigned(floor(fracdegrx));
	degy = unsigned(floor(fracdegry));
	minx = unsigned(60.0 * myfrac(fracdegrx) + 0.5);
	miny = unsigned(60.0 * myfrac(fracdegry) + 0.5);
	west = (x < 0.0);
	south = (y < 0.0);
}



vector3 sea_object::get_acceleration() const
{
	return vector3(0, 0, -GRAVITY);
}



/*
  fixme: later switch to model that uses force and torque.
  we need an inertia tensor for that model.

  compute torque of sub/ship to simulate moving in the waves:

  compute force at bow/stern/mid/left side/right side or some other important points.
  like where the trim tanks of subs are.

  the torque is computed as

  M = (r1 - r0) x F1, where F1 is the force acting at point r1, where r0 is center of gravity
  do r1-r0 is the vector from the object's center to the point where F1 is acting.
  M is a vector definining axis of rotation (direction of M) and strength of rotation (length of M).
  To compute total torque, sum all M over i: M_total = Sum_i (r_i - r0) x F_i

  To compute translational forces, just sum up all forces F: F_total = Sum_i F_i.

  Problem: orientation is stored as quaternion, not as three angles.
  torque changes angular velocity over time, and that changes orientation over time.
  The axes of torque or angular velocity don't need to be identical!
  That is the problem.
  Given torque, angular velocity and orientation as quaternions q_t, q_v and q_r we have
  q_v' = q_v * (q_t * delta_t)
  q_r' = q_r * (q_v * delta_t)
  But computing q * z, where q is a quaternion for rotation (unit quaternion) and z is a number
  is not that easy. q represents a rotation around an axis x and and angle w.
  q * z would then be the rotation around the same axis, but with angle w*z.
  A rotation quaternion for axis x and angle w is given as:
  (cos(w/2), sin(w/2)*x)
  Thus q*z would be
  (cos(w*z/2), sin(w*z/2)*)
  how to compute that from q? we would need an acos function call, which is very costly...
  One could also see cos(w/2),sin(w/2) as complex number. It has an absolute value of 1.
  So multiplying the angle with n would be the same as taking the n'th power of that number.
  But this computation would also be very costly - it is the same problem as we have here.
  Alternativly: Represent angular velocity and torque around three fix axes.
  Store axis and angle for each part, like x/y/z-axis and the three angles.
  Changing angular velocity by torque is easy then, orientation could still be stored as quaternion.

  Computing orientation forced by the waves:
  Compute buoyancy on points around the ship. Draught gives weight of displaced water,
  the difference of the ships weight (or the weight of that part for which the buoyancy
  is computed) gives a force, that is applied to the ship.
  Force = mass * acceleration. Acceleration is g, mass is difference between displaced water
  and the ship's part (is that correct for computation of buoyancy?).
  
*/

double sea_object::get_turn_acceleration() const
{
	return 0.0;
}



void sea_object::set_sensor ( sensor_system ss, sensor* s )
{
	if ( ss >= 0 && ss < last_sensor_system ){
		sensors[ss] = s;
	}
}



double sea_object::get_cross_section ( const vector2& d ) const
{
	model* mdl = modelcache.find(modelname);
	if (mdl) {
		vector2 r = get_pos().xy() - d;
		angle diff = angle(r) - get_heading();
		return mdl->get_cross_section(diff.value());
	}
	return 0.0;


}



sea_object::sea_object(game& gm_, const string& modelname_)
	: gm(gm_), modelname(modelname_), turn_velocity(0),
	  alive_stat(alive), target(0)
{
	model* mdl = modelcache.ref(modelname);
	size3d = vector3f(mdl->get_width(), mdl->get_length(), mdl->get_height());
}



sea_object::sea_object(game& gm_, const xml_elem& parent)
	: gm(gm_), turn_velocity(0), alive_stat(alive), target(0)
{
	xml_elem cl = parent.child("classification");
	specfilename = cl.attr("identifier");
	modelname = cl.attr("modelname");
	model* mdl = modelcache.ref(modelname);
	size3d = vector3f(mdl->get_width(), mdl->get_length(), mdl->get_height());
	//country = cl.attr("country");
	xml_elem ds = parent.child("description");
	for (xml_elem::iterator it = ds.iterate("far"); !it.end(); it.next()) {
		if (it.elem().attr("lang") == texts::get_language_code()) {
			descr_far = it.elem().child_text();
		}
	}
	for (xml_elem::iterator it = ds.iterate("medium"); !it.end(); it.next()) {
		if (it.elem().attr("lang") == texts::get_language_code()) {
			descr_medium = it.elem().child_text();
		}
	}
	for (xml_elem::iterator it = ds.iterate("near"); !it.end(); it.next()) {
		if (it.elem().attr("lang") == texts::get_language_code()) {
			descr_near = it.elem().child_text();
		}
	}
	xml_elem sn = parent.child("sensors");
	sensors.resize ( last_sensor_system );
	for (xml_elem::iterator it = sn.iterate("sensor"); !it.end(); it.next()) {
		string typestr = it.elem().attr("type");
		if (typestr == "lookout") set_sensor(lookout_system, new lookout_sensor());
		else if (typestr == "passivesonar") set_sensor(passive_sonar_system, new passive_sonar_sensor());
		else if (typestr == "activesonar") set_sensor(active_sonar_system, new active_sonar_sensor());
		else if (typestr == "radar") {
			radar_sensor::radar_type type = radar_sensor::radar_type_default;
			string radar_model = it.elem().attr("model");
			if ("British Type 271" == radar_model)
				type = radar_sensor::radar_british_type_271;
			else if ("British Type 272" == radar_model)
				type = radar_sensor::radar_british_type_272;
			else if ("British Type 273" == radar_model)
				type = radar_sensor::radar_british_type_273;
			else if ("British Type 277" == radar_model)
				type = radar_sensor::radar_british_type_277;
			else if ("German FuMO 29" == radar_model)
				type  = radar_sensor::radar_german_fumo_29;
			else if ("German FuMO 30" == radar_model)
				type  = radar_sensor::radar_german_fumo_30;
			else if ("German FuMO 61" == radar_model)
				type  = radar_sensor::radar_german_fumo_61;
			else if ("German FuMO 64" == radar_model)
				type  = radar_sensor::radar_german_fumo_64;
			else if ("German FuMO 391" == radar_model)
				type  = radar_sensor::radar_german_fumo_391;
			else
				throw error("invalid radar type name");
					
			set_sensor(radar_system, new radar_sensor(type));
		}
		// ignore unknown sensors.
	}

	// ai is filled in by heirs.
}



sea_object::~sea_object()
{
	modelcache.unref(modelname);
	for (unsigned i = 0; i < sensors.size(); i++)
		delete sensors[i];
}



void sea_object::load(const xml_elem& parent)
{
	string specfilename2 = parent.attr("type");	// checks
	// this checks if the filename of the specfile matches the internal id string...
	if (specfilename != specfilename2)
		throw error(string("stored specfilename does not match, type=") + specfilename2
			    + string(", but read ") + specfilename + string(" from spec file"));
	xml_elem st = parent.child("state");
	position = st.child("position").attrv3();
	velocity = st.child("velocity").attrv3();
	orientation = st.child("orientation").attrq();
	turn_velocity = st.child("turn_velocity").attrf();
	heading = st.child("heading").attra();
	// load ai
	if (myai.get()) {
		myai->load(gm, parent.child("AI"));
	}
	// load target
	target = gm.load_ptr(parent.child("target").attru());
}



void sea_object::save(xml_elem& parent) const
{
	// specfilename is requested and stored by game or callers of this function
	xml_elem st = parent.add_child("state");
	st.add_child("position").set_attr(position);
	st.add_child("velocity").set_attr(velocity);
	st.add_child("orientation").set_attr(orientation);
	st.add_child("turn_velocity").set_attr(turn_velocity);
	st.add_child("heading").set_attr(heading);
	parent.add_child("alive_stat").set_attr(unsigned(alive_stat));
	// save ai
	if (myai.get()) {
		xml_elem ae = parent.add_child("AI");
		myai->save(gm, ae);
	}
	// save target
	parent.add_child("target").set_attr(gm.save_ptr(target));
}



string sea_object::get_description(unsigned detail) const
{
	if (detail == 0) return descr_far;
	else if (detail == 1) return descr_medium;
	else return descr_near;
}



void sea_object::simulate(double delta_time)
{
	// check target. heirs should check for "out of range" condition too
	if (target && !target->is_alive())
		target = 0;

	// check and change states
        if (is_defunct()) {
		// fixme: maybe throw a dead_object exception here and catch them in game::simulate for each object
		// that way we have to check for defunct()/dead() only here, and not also in every heir of sea_object
                return;
        } else if (is_dead()) {
		destroy();
                return;
        }

	// this leads to another model for acceleration/max_speed/turning etc.
	// the object applies force to the screws etc. with Force = acceleration * mass.
	// there is some drag caused by air/water opposite to the force.
	// this drag damps the speed curve so that acceleration is zero at speed==max_speed.
	// drag depends on speed (v or v^2).
	// v = v0 + a * delta_t, v <= v_max, a = engine_force/mass - drag
	// now we have: drag(v) = max_accel = engine_force/mass.
	// and: v=v_max, hence: drag(v_max) = max_accel, max_accel is given, v_max too.
	// so choose a drag formula: factor*v or factor*v^2 and compute factor.
	// we have: factor*v_max^2 = max_accel => factor = max_accel/v_max^2
	// finally: v = v0 + delta_t * (max_accel - dragfactor * v0^2)
	// if v0 == 0 then we have maximum acceleration.
	// acceleration lowers quadratically until we have maximum velocity.
	// we also have side drag (limit turning speed!):

	// compute force for some velocity v: find accel so that accel - dragfactor * v^2 = 0
	// hence: accel = dragfactor * v^2, this means force is proportional to square
	// of speed -> fuel comsumption depends ~quadratically on speed.
	// To throttle to a given speed, apply max_accel until we have it then apply accel.
	// In reality engine throttle could translate directly to acceleration, this means
	// with 1/2 throttle we just use constant acceleration of 1/2*max_accel.
	// This leads to a maximum speed determined by drag, but time until that
	// speed is reached is much longer compared to using max_accel.

	// drag: drag force is -b * v (low speeds), -b * v^2 (high speeds)
	// b is proportional to cross section area of body.

	// more difficult: change acceleration to match a certain position (or angle)
	// s = s0 + v0 * t + a/2 * t^2, v = v0 + a * t
	// compute a over time so that s_final = s_desired and v_final = 0, with arbitrary s0,v0.
	// a can be 0<=a<=max_accel. three phases (in time) speed grows until max_speed,
	// constant max speed, speed shrinks to zero (sometimes only phases 1 and 3 needed).
	// determine phase, in phase 1 and 2 give max. acceleration, in phase 3 give zero.
	// or even give -max_accel is phase 3 (turn rudder to opposite with full throttle
	// to stop motion fastly)

	// Screw force splits in forward force and sideward force (dependend on rudder position)
	// so compute side drag from turn_rate
	// compute turn_rate to maximum angular velocity, limited by ship's draught and shape.

	// we store velocity/acceleration, not momentum/force/torque
	// in reality applied force is transformed by inertia tensor, which is rotated according to
	// orientation. we don't use mass here, so apply rotation to stored velocity/acceleration.
	//fixme: use orientation here and compute heading from it, not vice versa!
	quaternion horientation = quaternion::rot(-heading.value(), 0, 0, 1);

//	cout << "hdg=" << heading.value() << " hori=" << horientation <<
//		" orang=" << angle(horientation.rotate(0,1,0).xy()).value() << "\n";

	// use compute_force(), compute_torque() here, with inertia tensor etc.

	vector3 acceleration = get_acceleration();
	vector3 global_acceleration = horientation.rotate(acceleration);
	global_velocity = horientation.rotate(velocity);
	double t2_2 = 0.5 * delta_time * delta_time;

	//debugging
//	cout << "object " << this << " simulate.\npos: " << position << "\nvelo: " << velocity << "\naccel: " << acceleration << "\n";
//	cout << "global velo " << global_velocity << " global acc " << global_acceleration << "\n";

	position += global_velocity * delta_time + global_acceleration * t2_2;
	velocity += acceleration * delta_time;

	//debugging
//	cout << "NEWpos: " << position << "\nNEWvelo: " << velocity << "\n";
//	cout << "(delta t = " << delta_time << ")\n";
	
	double turnaccel = get_turn_acceleration();
	double add_turn_angle = turn_velocity * delta_time + turnaccel * t2_2;
	orientation = quaternion::rot(add_turn_angle, 0, 0, 1) * orientation;
	// turning is handled in mathematical order (growing angles are ccw).
	heading += angle(-add_turn_angle);
	turn_velocity += turnaccel * delta_time;

	vector2 abo_dir = orientation.rotate(0, 1, 0).xy();
	angle hdg_by_orient = angle(abo_dir);

	//debugging
	//cout << "object " << this << " orientat: " << orientation << " hdg " << heading.value() << " abo_dir " << abo_dir << " hdb2o " << hdg_by_orient.value() << "\n";
	//cout << "object " << this << " orientat: " << orientation << " rot_velo: " << turn_velocity << " turn_accel " << turnaccel << "\n";
}



bool sea_object::damage(const vector3& fromwhere, unsigned strength)
{
	kill();	// fixme crude hack, replace by damage simulation
	return true;
}



unsigned sea_object::calc_damage() const
{
	return is_dead() ? 100 : 0;
}



void sea_object::set_inactive()
{
	if (alive_stat == defunct)
		throw error("illegal alive_stat switch (defunct to inactive)");
	if (alive_stat == dead)
		throw error("illegal alive_stat switch (dead to inactive)");
	alive_stat = inactive;
}



void sea_object::kill()
{
	if (alive_stat == defunct)
		throw error("illegal alive_stat switch (defunct to inactive)");
	alive_stat = dead;
}



void sea_object::destroy()
{
	alive_stat = defunct;
}



float sea_object::surface_visibility(const vector2& watcher) const
{
	return get_cross_section(watcher);
}



//fixme: should move to ship or maybe return pos. airplanes have engines, but not
//dc's/shells.
vector2 sea_object::get_engine_noise_source () const
{
	return get_pos ().xy () - get_heading().direction () * 0.3f * get_length();
}



void sea_object::display() const
{
	model* mdl = modelcache.find(modelname);

	if ( mdl )
		mdl->display ();
}



sensor* sea_object::get_sensor ( sensor_system ss )
{
	if ( ss >= 0 && ss < last_sensor_system )
		return sensors[ss];

	return 0;
}



const sensor* sea_object::get_sensor ( sensor_system ss ) const
{
	if ( ss >= 0 && ss < last_sensor_system )
		return sensors[ss];

	return 0;
}
