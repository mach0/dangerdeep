// ships
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "ship.h"
#include "model.h"
#include "game.h"
#include "date.h"
#include "tokencodes.h"
#include "sensors.h"
#include "ai.h"
#include "system.h"
#include "particle.h"
#include "tinyxml/tinyxml.h"
#include "gun_shell.h"

map<double, map<double, double> > ship::dist_angle_relation;
#define MAX_INCLINATION 45.0
#define MAX_DECLINATION -20.0
#define ANGLE_GAP 0.1
#define GUN_RELOAD_TIME 5.0

//fixme: redefine display, call base display


// empty c'tor is needed by heirs
ship::ship() : sea_object(), myai(0), smoke_type(0)
{
	init();
}



void ship::init(void)
{
	// set some default values
	heading = 0;
	throttle = 0;
	head_to_fixed = false;
	rudder_pos = 0;
	rudder_to = 0;
	max_rudder_angle = 40;
	max_rudder_turn_speed = 10;
	max_angular_velocity = 2;
	turn_rate = 1;
	max_accel_forward = 1;
	max_speed_forward = 10;
	max_speed_reverse = 0;
	myfire = 0;		
	gun_manning_is_changing = false;
	gun_turrets.clear();
	maximum_gun_range = 0.0;
}

void ship::fill_dist_angle_relation_map(const double initial_velocity)
{
	if (dist_angle_relation.find(initial_velocity) == dist_angle_relation.end())
	{
		for (double a = 0; a > MAX_DECLINATION+ANGLE_GAP; a -= ANGLE_GAP) {
			angle elevation(a);
			double z = 4;	// meters, initial height above water
			double vz = initial_velocity * elevation.sin();
			double dist = 0;
			double vdist = initial_velocity * elevation.cos();
			
			for (double dt = 0; dt < 120.0; dt += 0.001) {
				dist += vdist * dt;
				z += vz * dt;
				vz += -GRAVITY * dt;
				if (z <= 0) break;
			}
			
			dist_angle_relation[initial_velocity][dist] = a;
		}
		
		for (double a = 0; a < MAX_INCLINATION+ANGLE_GAP; a += ANGLE_GAP) {
			angle elevation(a);
			double z = 4;	// meters, initial height above water
			double vz = initial_velocity * elevation.sin();
			double dist = 0;
			double vdist = initial_velocity * elevation.cos();
			
			for (double dt = 0; dt < 120.0; dt += 0.001) {
				dist += vdist * dt;
				z += vz * dt;
				vz += -GRAVITY * dt;
				if (z <= 0) break;
			}
			
			dist_angle_relation[initial_velocity][dist] = a;
		}
	}
}

ship::ship(TiXmlDocument* specfile, const char* topnodename) : sea_object(specfile, topnodename)
{
	init();	
	TiXmlHandle hspec(specfile);
	TiXmlHandle hdftdship = hspec.FirstChild(topnodename);
	TiXmlElement* eclassification = hdftdship.FirstChildElement("classification").Element();
	sys().myassert(eclassification != 0, string("ship: classification node missing in ")+specfilename);
	string typestr = XmlAttrib(eclassification, "type");
	if (typestr == "warship") shipclass = WARSHIP;
	else if (typestr == "escort") shipclass = ESCORT;
	else if (typestr == "merchant") shipclass = MERCHANT;
	else if (typestr == "submarine") shipclass = SUBMARINE;
	else sys().myassert(false, string("illegal ship type in ") + specfilename);
	TiXmlElement* etonnage = hdftdship.FirstChildElement("tonnage").Element();
	sys().myassert(etonnage != 0, string("tonnage node missing in ")+specfilename);
	unsigned minton = XmlAttribu(etonnage, "min");
	unsigned maxton = XmlAttribu(etonnage, "max");
	tonnage = minton + rnd(maxton - minton + 1);
	TiXmlElement* emotion = hdftdship.FirstChildElement("motion").Element();
	sys().myassert(emotion != 0, string("motion node missing in ")+specfilename);
	double tmp = 0;
	if (emotion->Attribute("maxspeed", &tmp))
		max_speed_forward = kts2ms(tmp);
	tmp = 0;
	if (emotion->Attribute("maxrevspeed", &tmp))
		max_speed_reverse = kts2ms(tmp);
	emotion->Attribute("acceleration", &max_accel_forward);
	tmp = 0;
	if (emotion->Attribute("turnrate", &tmp))
		turn_rate = tmp;
	TiXmlElement* esmoke = hdftdship.FirstChildElement("smoke").Element();
	smoke_type = 0;
	if (esmoke) {
		int smtype = 0;
		esmoke->Attribute("type", &smtype);
		if (smtype > 0) {
			TiXmlElement* esmpos = esmoke->FirstChildElement("position");
			sys().myassert(esmpos != 0, string("no smoke position given in ")+specfilename);
			esmpos->Attribute("x", &smokerelpos.x);
			esmpos->Attribute("y", &smokerelpos.y);
			esmpos->Attribute("z", &smokerelpos.z);
			smoke_type = smtype;
		}
	}
	TiXmlElement* eai = hdftdship.FirstChildElement("ai").Element();
	sys().myassert(eai != 0, string("ai node missing in ")+specfilename);
	string aitype = XmlAttrib(eai, "type");
	if (aitype == "dumb") myai = new ai(this, ai::dumb);
	else if (aitype == "escort") myai = new ai(this, ai::escort);
	else if (aitype == "none") myai = 0;
	else sys().myassert(false, string("illegal AI type in ") + specfilename);
	TiXmlElement* efuel = hdftdship.FirstChildElement("fuel").Element();
	sys().myassert(efuel != 0, string("fuel node missing in ")+specfilename);
	fuel_capacity = XmlAttribu(efuel, "capacity");
	efuel->Attribute("consumption_a", &fuel_value_a);
	efuel->Attribute("consumption_t", &fuel_value_t);
		
	TiXmlElement* turrets = hdftdship.FirstChildElement("gun_turrets").Element();
	if (NULL != turrets)
	{
		TiXmlElement* turret = NULL;

		while(turret = (TiXmlElement*)turrets->IterateChildren(turret))
		{
			struct gun_turret new_turret;
			int num_barrels = 0;
			
			if (NULL == turret->Attribute("barrels", &num_barrels))
				assert(false);
			if (NULL == turret->Attribute("shell_capacity", &new_turret.shell_capacity))
				assert(false);
			new_turret.num_shells_remaining = new_turret.shell_capacity;
			if (NULL == turret->Attribute("initial_velocity", &new_turret.initial_velocity))
				assert(false);
			if (NULL == turret->Attribute("max_declination", &new_turret.max_declination))
				assert(false);
			if (NULL == turret->Attribute("max_inclination", &new_turret.max_inclination))
				assert(false);
			if (NULL == turret->Attribute("time_to_man", &new_turret.time_to_man))
				assert(false);
			if (NULL == turret->Attribute("time_to_man", &new_turret.time_to_unman))
				assert(false);
			if (NULL == turret->Attribute("shell_damage", &new_turret.shell_damage))
				assert(false);
			if (NULL == turret->Attribute("exclusion_radius_start", &new_turret.start_of_exclusion_radius))
				assert(false);
			if (NULL == turret->Attribute("exclusion_radius_end", &new_turret.end_of_exclusion_radius))
				assert(false);
			if (NULL == turret->Attribute("calibre", &new_turret.calibre))
				assert(false);						
			
			for (int x = 0; x < num_barrels; x++)
			{
				struct gun_barrel new_barrel;												
				new_turret.gun_barrels.push_back(new_barrel);
			}
			
			// setup angles map for this initial velocity
			fill_dist_angle_relation_map(new_turret.initial_velocity);
			calc_max_gun_range(new_turret.initial_velocity);
			
			gun_turrets.push_back(new_turret);		
		}
	}		
}



ship::~ship()
{
	delete myai;
}



void ship::sink(void)
{
	sea_object::set_inactive();
	if (myfire) {
		myfire->kill();
		myfire = 0;
	}
}



void ship::ignite(game& gm)
{
	if (myfire) {
		myfire->kill();
	}
	myfire = new fire_particle(get_pos());
	gm.spawn_particle(myfire);
}



void ship::change_rudder(int to)
{
	if (to >= rudderfullleft && to <= rudderfullright)
		rudder_to = to;
	else
		rudder_to = ruddermidships;
	head_to_fixed = false;
}



void ship::rudder_left(void)
{
	if (rudder_to > rudderfullleft)
		--rudder_to;
	head_to_fixed = false;
}



void ship::rudder_right(void)
{
	if (rudder_to < rudderfullright)
		++rudder_to;
	head_to_fixed = false;
}



void ship::rudder_hard_left(void)
{
	rudder_to = rudderfullleft;
	head_to_fixed = false;
}



void ship::rudder_hard_right(void)
{
	rudder_to = rudderfullright;
	head_to_fixed = false;
}



void ship::rudder_midships(void)
{
	rudder_to = ruddermidships;
	head_to_fixed = false;
}



void ship::set_throttle(throttle_status thr)
{
	throttle = thr;
}



void ship::remember_position(void)
{
	previous_positions.push_front(get_pos().xy());
	if (previous_positions.size() > MAXPREVPOS)
		previous_positions.pop_back();
}	



double ship::get_throttle_speed(void) const
{
	double ms = get_max_speed();
	if (throttle <= 0) {
		switch (throttle) {
			case reverse: return -ms*0.25f;     // 1/4
			case stop: return 0;
			case aheadlisten: return ms*0.25f;  // 1/4
			case aheadsonar: return ms*0.25f;   // 1/4
			case aheadslow: return ms*0.33333f; // 1/3
			case aheadhalf: return ms*0.5f;     // 1/2
			case aheadfull: return ms*0.75f;    // 3/4
			case aheadflank: return ms;
		}
	} else {
		double sp = kts2ms(throttle);
		if (sp > ms) sp = ms;
		return sp;
	}
	return 0;
}



double ship::get_throttle_accel(void) const
{
	// Beware: a throttle of 1/3 doesn't mean 1/3 of engine acceleration
	// This is because drag raises quadratically.
	// we have: max_accel_forward / max_speed_forward^2 = drag_factor
	// and: drag = drag_factor * speed^2
	// get acceleration for constant throttled speed: accel = drag
	// solve:
	// accel = drag_factor * speed^2 = max_accel_forward * speed^2 / max_speed_forward^2
	// fixme: 2004/07/18: throttle to some speed would mean maximum acceleration until
	// we get close to this speed... but we don't set speed here but engine throttle...
	// fixme: reverse throttle doesn't work. obvious why...
	double speed_fac = get_throttle_speed() / max_speed_forward;
	return max_accel_forward * (speed_fac * speed_fac);
}



pair<angle, double> ship::bearing_and_range_to(const sea_object* other) const
{
	vector2 diff = other->get_pos().xy() - position.xy();
	return make_pair(angle(diff), diff.length());
}



angle ship::estimate_angle_on_the_bow(angle target_bearing, angle target_heading) const
{
	return (angle(180) + target_bearing - target_heading).value_pm180();
}



void ship::parse_attributes(TiXmlElement* parent)
{
	sea_object::parse_attributes(parent);
	
	TiXmlElement* emotion = TiXmlHandle(parent).FirstChildElement("motion").Element();
	if (emotion) {
		// heading / speed are already parsed in class sea_object
/*
		double tmp = 0;
		if (emotion->Attribute("heading", &tmp))
			heading = angle(tmp);
		tmp = 0;
		if (emotion->Attribute("speed", &tmp))
			velocity.y = kts2ms(tmp);
*/
		string thr = XmlAttrib(emotion, "throttle");
		if (thr == "stop") set_throttle(stop);
		else if (thr == "reverse") set_throttle(reverse);
		else if (thr == "aheadlisten") set_throttle(aheadlisten);
		else if (thr == "aheadsonar") set_throttle(aheadsonar);
		else if (thr == "aheadslow") set_throttle(aheadslow);
		else if (thr == "aheadhalf") set_throttle(aheadhalf);
		else if (thr == "aheadfull") set_throttle(aheadfull);
		else if (thr == "aheadflank") set_throttle(aheadflank);
		else set_throttle(atoi(thr.c_str()));
	}
	// fixme: parse permanent_turn,head_chg,head_to,rudder  maybe also alive_stat,previous_positions
	// parse tonnage, fuel level, damage status, fixme
}



void ship::load(istream& in, game& g)
{
	sea_object::load(in, g);

	if (read_bool(in))
		myai = new ai(in, g);
	
	tonnage = read_u32(in);
	stern_damage = damage_status(read_u8(in));
	midship_damage = damage_status(read_u8(in));
	bow_damage = damage_status(read_u8(in));
	fuel_level = read_double(in);
	
	// gun turrets
	unsigned long num_turrets = read_u32(in);
		
	for (unsigned long x = 0; x < num_turrets; x++)
	{
		struct gun_turret turret;
		unsigned long num_barrels = 0;
		
		turret.num_shells_remaining = read_u32(in);
		turret.shell_capacity = read_u32(in);
		turret.initial_velocity = read_double(in);		
		turret.max_declination = read_i32(in);
		turret.max_inclination = read_i32(in);
		turret.time_to_man = read_double(in);
		turret.time_to_unman = read_double(in);
		turret.is_gun_manned = read_bool(in);
		turret.manning_time = read_double(in);
		turret.shell_damage = read_double(in);
		turret.start_of_exclusion_radius = read_u32(in);
		turret.end_of_exclusion_radius = read_u32(in);
		turret.calibre = read_double(in);

		num_barrels = read_u32(in);
		
		for (unsigned long x = 0; x < num_barrels; x++)
		{
			struct gun_barrel new_barrel;	
			
			new_barrel.load_time_remaining = read_double(in);
			new_barrel.last_elevation = read_double(in);
			new_barrel.last_azimuth = read_double(in);
			
			turret.gun_barrels.push_back(new_barrel);
		}
		
		// setup angles map for this initial velocity
		fill_dist_angle_relation_map(turret.initial_velocity);
		calc_max_gun_range(turret.initial_velocity);
		
		gun_turrets.push_back(turret);
	}	
}

void ship::save(ostream& out, const game& g) const
{
	sea_object::save(out, g);

	write_bool(out, (myai != 0));
	if (myai)
		myai->save(out, g);
	
	write_u32(out, tonnage);
	write_u8(out, stern_damage);
	write_u8(out, midship_damage);
	write_u8(out, bow_damage);
	write_double(out, fuel_level);
	
	// gun turrets
	write_u32(out, gun_turrets.size());
	
	const_gun_turret_itr turret = gun_turrets.begin();
	while (turret != gun_turrets.end())
	{
		write_u32(out, turret->num_shells_remaining);
		write_u32(out, turret->shell_capacity);
		write_double(out, turret->initial_velocity);		
		write_i32(out, turret->max_declination);
		write_i32(out, turret->max_inclination);
		write_double(out, turret->time_to_man);
		write_double(out, turret->time_to_unman);
		write_bool(out, turret->is_gun_manned);
		write_double(out, turret->manning_time);
		write_double(out, turret->shell_damage);
		write_u32(out, turret->start_of_exclusion_radius);
		write_u32(out, turret->end_of_exclusion_radius);
		write_double(out, turret->calibre);
		
		write_u32(out, turret->gun_barrels.size());
		
		const_gun_barrel_itr barrel = turret->gun_barrels.begin();
		while (barrel != turret->gun_barrels.begin())
		{		
			write_double(out, barrel->load_time_remaining);
			write_double(out, barrel->last_elevation.value());
			write_double(out, barrel->last_azimuth.value());
			
			barrel++;
		}
								  
		turret++;
	}
}

void ship::simulate(game& gm, double delta_time)
{
	sea_object::simulate(gm, delta_time);
	if (is_defunct()) return;
	if ( myai )
		myai->act(gm, delta_time);

	// calculate sinking
	if (is_inactive()) {
		position.z -= delta_time * SINK_SPEED;
		if (position.z < -50)	// used for ships.
			kill();
		throttle = stop;
		rudder_midships();
		return;
	}

	// Adjust fuel_level.
	calculate_fuel_factor(delta_time);

	// adjust fire pos if burning
	if (myfire) {
		myfire->set_pos(get_pos() + vector3(0, 0, 12));
	}

	if (causes_spray()) {
		double v = velocity.length();
		if (v > 0.1) {
			double produce_time = 2.0/v;
			double t = myfmod(gm.get_time(), produce_time);
			if (t + delta_time >= produce_time) {
				vector3 forward = global_velocity.normal();
				vector3 sideward = forward.cross(vector3(0, 0, 1)).normal() * 2.0;//speed 2.0 m/s
				vector3 spawnpos = get_pos() + forward * (get_length() * 0.5);
				gm.spawn_particle(new spray_particle(spawnpos, sideward));
				gm.spawn_particle(new spray_particle(spawnpos, -sideward));
			}
		}
	}
	
	// smoke particle generation logic
	if (is_alive() && smoke_type > 0) {//replace by has_particle
		double produce_time = 1e10;
		switch (smoke_type) {
		case 1: produce_time = smoke_particle::get_produce_time(); break;
		case 2: produce_time = smoke_particle_escort::get_produce_time(); break;
		}
		double t = myfmod(gm.get_time(), produce_time);
		if (t + delta_time >= produce_time) {
			particle* p = 0;
			vector3 ppos = position + smokerelpos;//fixme: maybe add some random offset
			switch (smoke_type) {
			case 1: p = new smoke_particle(ppos); break;
			case 2: p = new smoke_particle_escort(ppos); break;
			}
			gm.spawn_particle(p);
		}
	}

	// steering logic, adjust rudder pos so that heading matches head_to
	if (head_to_fixed) {

		// check if we should turn left or right
		bool turn_rather_right = (heading.is_cw_nearer(head_to));
//cout<<this<<" logic: heading " << heading.value() << " head_to " << head_to.value() << " trr " << turn_rather_right << " rudder_to " << rudder_to << " rudder_pos " << rudder_pos << " \n";
		rudder_to = (turn_rather_right) ? rudderfullright : rudderfullleft;
//cout <<this<<" logic2 rudder_to " << rudder_to << " turn velo " << turn_velocity << "\n";
// cout << "total time " << gm.get_time() << "\n";

		double angledist = fabs((heading - head_to).value_pm180());

		if (use_simple_turning_model()) {
			turn_velocity = rudder_pos * max_angular_velocity / max_rudder_angle;
			if (angledist < 0.1) {
				rudder_pos = 0;
				rudder_to = ruddermidships;
				turn_velocity = 0;
			}
		} else {

			// check if we approach head_to (brake by turning rudder to the opposite)
			// if time need to set the rudder to midships is smaller than the time until heading
			// passes over head_to, we have to brake.
			double time_to_pass = (fabs(turn_velocity) < 0.01) ? 1e30 : angledist / fabs(turn_velocity);
			double time_to_midships = fabs(rudder_pos) / max_rudder_turn_speed;
			double time_to_rudder_opposite = (fabs(rudder_pos) + max_rudder_angle) / max_rudder_turn_speed;

			//fixme: time_to_ms assumes when rudder is midships again that turn speed is then roughly zero.
			//this is ok for ships/subs, but not for fast turning objects (torpedoes)
			//we should rather guess time to brake here! fixme

			/*
			  double rudder_pos_backup = rudder_pos;
			  rudder_pos = (turn_rather_right) ? -max_rudder_angle : max_rudder_angle;
			  double ta = get_turn_acceleration();
			  cout << "ta is " << ta << " turn_velo " << turn_velocity << "\n";
			  rudder_pos = rudder_pos_backup;
			  double time_to_turn_stop = (fabs(ta) < 0.0001) ? 0.0 : fabs(turn_velocity / ta);
			  cout << "time to turn stop " << time_to_turn_stop << "\n";
			*/

			// back up rudder angle, set it to maximum opposite angle
			//compute get_turn_acceleration, divide turn_speed by turn_accel
			//this time factor is time_to_stop_turning
			// BUT! turn_velocity does not fall immediatly...rudder is not at once on opposite

			//cout <<this<<" logic3 angledist " << angledist << " timetopass " << time_to_pass << " time_to_ms " << time_to_midships << "\n";
			double damping_factor = 0.5;	// set to > 0 to brake earlier, fixme set some value
			if (time_to_pass < time_to_midships + damping_factor) {
				rudder_to = (turn_rather_right) ? rudderfullleft : rudderfullright;
				//cout <<this<<" near target! " << rudder_to << "\n";
			}
			// check for final rudder midships, fixme adapt values...
			if (angledist < 0.5 && fabs(rudder_pos) < 1.0) {
				rudder_to = ruddermidships;
				//cout <<this<<" dest reached " << angledist << "," << fabs(rudder_pos) << "\n";
				head_to_fixed = false;
			}
		}
	}
	
	// Adjust rudder
	// rudder_to with max_rudder_angle gives set rudder angle.
	double rudder_angle_set = max_rudder_angle * rudder_to / 2;
	// current angle is rudder_pos. rudder moves with constant speed to set pos (or 0).
	double max_rudder_turn_dist = max_rudder_turn_speed * delta_time;
	double rudder_d = rudder_angle_set - rudder_pos;
	if (fabs(rudder_d) <= max_rudder_turn_dist) {	// if rudder_d is 0, nothing happens.
		rudder_pos = rudder_angle_set;
	} else {
		if (rudder_d < 0) {
			rudder_pos -= max_rudder_turn_dist;
		} else {
			rudder_pos += max_rudder_turn_dist;
		}
	}
	
	// gun turrets
	gun_turret_itr gun_turret = gun_turrets.begin();	
	while (gun_turret != gun_turrets.end())
	{
		if (gun_turret->manning_time > 0.0)
		{
			gun_turret->manning_time -= delta_time;
			if (gun_turret->manning_time <= 0.0)
			{
				gun_turret->is_gun_manned = !gun_turret->is_gun_manned;					
				gun_manning_is_changing = false;
				gun_manning_changed(gun_turret->is_gun_manned);
			}
		}
		
		if (gun_turret->manning_time <= 0.0)
		{
			gun_barrel_itr gun_barrel = gun_turret->gun_barrels.begin();
			while (gun_barrel != gun_turret->gun_barrels.end())
			{		
				if (gun_barrel->load_time_remaining > 0.0)
					gun_barrel->load_time_remaining -= delta_time;
										
				gun_barrel++;
			}
		}
			
		gun_turret++;
	}	
}



void ship::fire_shell_at(const vector2& pos)
{
	// fixme!!!!!!
}



void ship::head_to_ang(const angle& a, bool left_or_right)	// true == left
{
	head_to = a;
	//fixme: very crude... or use rudderleft/rudderright here (not full rudder?)
	//not crude with steering logic somewhere else... in simulate
	rudder_to = (left_or_right) ? rudderfullleft : rudderfullright;
	head_to_fixed = true;
}



bool ship::damage(const vector3& fromwhere, unsigned strength)
{
	damage_status& where = midship_damage;//fixme
	int dmg = int(where) + strength;
	if (dmg > wrecked) where = wrecked; else where = damage_status(dmg);
	// fixme:
	if (rand() % 2 == 0) {
		stern_damage = midship_damage = bow_damage = wrecked;
		sink();
		return true;
	} else{
		stern_damage = midship_damage = bow_damage = mediumdamage;
		return false;
	}
}



unsigned ship::calc_damage(void) const
{
	if (bow_damage == wrecked || midship_damage == wrecked || stern_damage == wrecked)
		return 100;
	unsigned dmg = unsigned(round(15*(bow_damage + midship_damage + stern_damage)));
	return dmg > 100 ? 100 : dmg;
}



double ship::get_roll_factor(void) const
{
	return 400.0 / (get_tonnage() + 6000.0);	// fixme: rather simple yet. should be overloaded by each ship
}



double ship::get_noise_factor (void) const
{
    return get_throttle_speed () / max_speed_forward;
}



//fixme: deceleration is to low at low speeds, causing the sub the turn a LONG time after
//rudder is midships/screws stopped. Is fixed by setting drag to linear at speeds < 1.0
//fixme: drag can go nuts when time is scaled causing NaN in double...
//this is because damping becomes to crude at high time scale
//fixme: in reality drag is v and v^2 combined... on low speeds v is significant term, on higher speeds
//it is v^2. It is: v > v^2 for v < 1.
vector3 ship::get_acceleration(void) const		// drag must be already included!
{
	// acceleration of ship depends on rudder.
	// forward acceleration is max_accel_forward * cos(rudder_ang)
	//fixme 2004/07/18: the drag is too small. engine stop -> the ship slows down but
	//to slowly, especially on low speeds. it would take a LONG time until it comes
	//to an halt.
	//compute max_accel_forward from mass and engine power to have a rough guess.
	//in general: Power/(rpm * screw_radius * mass) = accel
	//Power: engine Power (kWatts), rpm (screw turns per second), mass (ship's mass)
	//SubVIIc: ~3500kW, rad=0.5m, rpm=2 (?), mass=750000kg -> acc=4,666. a bit much...
	double speed = get_speed();
	double speed2 = speed*speed;
	if (fabs(speed) < 1.0) speed2 = fabs(speed)*max_speed_forward;
	double drag_factor = (speed2) * max_accel_forward / (max_speed_forward*max_speed_forward);
	double acceleration = get_throttle_accel() * cos(rudder_pos * M_PI / 180.0);
	if (speed > 0) drag_factor = -drag_factor;
	return vector3(0, acceleration + drag_factor, 0);
}



double ship::get_turn_acceleration(void) const	// drag must be already included!
{
	// acceleration of ship depends on rudder state and actual forward speed (linear).
	// angular acceleration (turning) is speed * sin(rudder_ang) * factor
	// this is acceleration around local z-axis.
	// the factor depends on rudder area etc.
	//fixme: do we have to multiply in some factor? we have angular values here not linear...
	//double drag_factor = some_factor * current_turn_speed^2
	//some_factor is given by turn rate
	double speed = get_speed();
	double tv2 = turn_velocity*turn_velocity;
	if (fabs(turn_velocity) < 1.0) tv2 = fabs(turn_velocity) * max_angular_velocity;
	double accel_factor = 1.0;	// given by turn rate, influenced by rudder area...
	double max_turn_accel = accel_factor * max_speed_forward * sin(max_rudder_angle * M_PI / 180.0);
	double drag_factor = (tv2) * max_turn_accel / (max_angular_velocity*max_angular_velocity);
	double acceleration = accel_factor * speed * sin(rudder_pos * M_PI / 180.0);
	if (turn_velocity > 0) drag_factor = -drag_factor;
//cout << "TURNING: accel " << acceleration << " drag " << drag_factor << " max_turn_accel " << max_turn_accel << " turn_velo " << turn_velocity << " heading " << heading.value() << " tv2 " << tv2 << "\n";
//cout << "get_rot_accel for " << this << " rudder_pos " << rudder_pos << " sin " << sin(rudder_pos * M_PI / 180.0) << " max_turn_accel " << max_turn_accel << "\n";
	return acceleration + drag_factor;
}



void ship::calculate_fuel_factor ( double delta_time )
{
	fuel_level -= delta_time * get_fuel_consumption_rate ();
}

int ship::fire_shell_at(game& gm, const sea_object& s)
{
	int res = GUN_FIRED;
	gun_turret_itr gun_turret = gun_turrets.begin();
	gun_barrel_itr gun_barrel;
	//fixme!!! move this code to class ship::fire_shell_at. move dist_angle relation also,
	//maybe approximate that relation with splines.		
	
	while (gun_turret != gun_turrets.end())
	{
		struct gun_turret *gun = &(*gun_turret);

		if (gun->num_shells_remaining > 0)
		{
			if (true == gun->is_gun_manned && gun->manning_time <= 0.0)
			{
				gun_barrel = gun_turret->gun_barrels.begin();
				while (gun_barrel != gun_turret->gun_barrels.end())
				{				
					if (gun_barrel->load_time_remaining <= 0.0)
					{
						// maybe we should not use current position but rather
						// estimated position at impact!
						vector2 deltapos = s.get_pos().xy() - get_pos().xy();
						double distance = deltapos.length();
						angle direction(deltapos);

						double max_shooting_distance = (dist_angle_relation[gun_turret->initial_velocity].rbegin())->first;
						if (distance > max_shooting_distance) 
							res = TARGET_OUT_OF_RANGE;	// can't do anything
						
						if (GUN_FIRED == res)
						{
							if (false == is_target_in_blindspot(gun, heading - direction))
							{
								// initial angle: estimate distance and fire, remember angle
								// next shots: adjust angle after distance fault:
								//	estimate new distance from old and fault
								//	select new angle, fire.
								//	use an extra bit of correction for wind etc.
								//	to do that, we need to know where the last shot impacted!							
								angle elevation;
								if (true == calculate_gun_angle(distance, elevation, gun_turret->initial_velocity))
								{														
									if (elevation.value() > gun->max_inclination)
										res = TARGET_OUT_OF_RANGE;
									else if (elevation.value() < gun->max_declination)
										res = GUN_TARGET_IN_BLINDSPOT;
									else
									{
										// fixme: for a smart ai: try to avoid firing at friendly ships that are in line
										// of fire.
										
										// fixme: snap angle values to simulate real cannon accuracy.

										// fixme: adapt direction & elevation to course and speed of target!
										gm.spawn_gun_shell(new gun_shell(*this, direction, elevation, gun->initial_velocity, gun->shell_damage), 
														   gun->calibre);
										gun->num_shells_remaining--;
										gun_barrel->load_time_remaining = GUN_RELOAD_TIME;
										gun_barrel->last_elevation = elevation;	
										gun_barrel->last_azimuth = direction;
									}
								}
								else
									res = TARGET_OUT_OF_RANGE;	// unsuccesful angle, fixme
							}
							else
								res = GUN_TARGET_IN_BLINDSPOT;
						}
					}
					else
						res = RELOADING;
					
					gun_barrel++;
				}
			}
			else
				res = GUN_NOT_MANNED;
		}
		else
			res = NO_AMMO_REMAINING;
		
		gun_turret++;
	}
		
	return res;
}

bool ship::toggle_gun_manning()
{
	if (false == gun_manning_is_changing)
	{
		gun_turrets.begin()->manning_time = (true == gun_turrets.begin()->is_gun_manned) ? 
											gun_turrets.begin()->time_to_unman : gun_turrets.begin()->time_to_man;
		gun_manning_is_changing = true;
	}
	
	return !gun_turrets.begin()->is_gun_manned;
}

void ship::gun_manning_changed(bool)
{ }

// This function determines is the target for the gun is within the exclusion radius for 
// the turret. The exclusion radius defines one constant arc where the gun cannot aim (i.e. on a sub
// this would usually be the area directly behind the gun where the conning tower is, you can't shoot 
// through that). 
bool ship::is_target_in_blindspot(const struct gun_turret *gun, angle bearingToTarget)
{
	bool isInBlindSpot = false;
	
	if (gun->start_of_exclusion_radius != gun->end_of_exclusion_radius)
	{
		if (gun->start_of_exclusion_radius < gun->end_of_exclusion_radius)
		{
			if (bearingToTarget.value() >= gun->start_of_exclusion_radius && bearingToTarget.value() <= gun->end_of_exclusion_radius)
				isInBlindSpot = true;
		}
		else
		{
			if (bearingToTarget.value() >= gun->start_of_exclusion_radius || bearingToTarget.value() <= gun->end_of_exclusion_radius)
				isInBlindSpot = true;
		}
	}
	
	return isInBlindSpot;
}

long ship::num_shells_remaining()
{
	long numShells = 0;
	gun_turret_itr gunTurret = gun_turrets.begin();
	
	while (gunTurret != gun_turrets.end())
	{
		numShells += gunTurret->num_shells_remaining;
		gunTurret++;
	}
	
	return numShells;
}

bool ship::is_gun_manned()
{
	return gun_turrets.begin()->is_gun_manned;
}

bool ship::calculate_gun_angle(const double distance, angle &elevation, const double initial_velocity)
{
	bool withinRange = false;

	map<double, double>::iterator it = dist_angle_relation[initial_velocity].lower_bound(distance);
	if (it != dist_angle_relation[initial_velocity].end()) 
	{
		elevation = angle(it->second);
		withinRange = true;
	}

	return withinRange;
}

void ship::calc_max_gun_range(double initial_velocity)
{
	double max_range = dist_angle_relation[initial_velocity].rbegin()->first;
	
	maximum_gun_range = (max_range > maximum_gun_range) ? max_range : maximum_gun_range;
}
