// ai
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "ai.h"
#include "game.h"

void ai::relax(void)
{
	has_contact = false;
	state = (followme != 0) ? followobject : followpath;
	parent->set_throttle(sea_object::aheadsonar);
}

void ai::attack_contact(const vector3& c)
{
	has_contact = true;
	contact = c;
	state = attackcontact;
}

void ai::follow(sea_object* t)
{
	followme = t;
	state = (followme != 0) ? followobject : followpath;
}

void ai::act(class game& gm, double delta_time)
{
	remaining_time -= delta_time;
	if (remaining_time > 0) {
		return;
	} else {
		remaining_time = AI_THINK_CYCLE_TIME;
	}

	switch (type) {
		case escort: act_escort(gm, delta_time); break;
		default: act_dumb(gm, delta_time); break;
	}
	
	if (zigzagstate > 0) {
		if (zigzagstate == 5)
			parent->rudder_left(0.5);
		else if (zigzagstate == 15)
			parent->rudder_right(0.5);
		++zigzagstate;
		if (zigzagstate > 20)
			zigzagstate = 1;
	}
}

void ai::set_zigzag(bool stat)
{
	if (stat)
		zigzagstate = 1;
	else
		zigzagstate = 0;
}

void ai::act_escort(game& gm, double delta_time)
{
	// always watch out/listen/ping for the enemy
	// watch around

	// fixme: a list of submarine* is bad (more information given than
	// what is really visible, not compatible to network play!
	// but how else should the ai ask for course and speed?
	// a contact should be of the form: position, course, type.
	// contact's speed should be determined by the ai itself.
	// but how can the ai identify contacts? by the objects adress i.e. pointer?
	// this would be nearly the same as returning a list of pointers (see above).

	double dist = 1e12;
	submarine* nearest_contact = 0;
	list<submarine*> subs = gm.visible_submarines(parent->get_pos());
	for (list<submarine*>::iterator it = subs.begin(); it != subs.end(); ++it) {
		double d = (*it)->get_pos().xy().square_distance(parent->get_pos().xy());
		if (d < dist) {
			dist = d;
			nearest_contact = *it;
		}
	}
	if (nearest_contact) {	// is there a contact?
		fire_shell_at(gm, *nearest_contact);
		attack_contact(nearest_contact->get_pos());
		parent->set_throttle(sea_object::aheadflank);
	}

	if (state != attackcontact) {	// nothing found? try a ping or listen
		// ping around to find something
		list<vector3> contacts = gm.ping_ASDIC(parent->get_pos().xy(),
			parent->get_heading()+angle(rnd(360)));	//fixme
		if (contacts.size() > 0) {
			// fixme: choose best contact!
			attack_contact(contacts.front());
		}
	}

	if (state == followpath || state == followobject) {
		act_dumb(gm, delta_time);
	} else if (state == attackcontact) {	// attack sonar/visible contact

		set_course_to_pos(contact.xy());

		vector2 delta = contact.xy() - parent->get_pos().xy();
		double cd = delta.length();
		if (cd > DC_ATTACK_RUN_RADIUS) {
			list<vector3> contacts = gm.ping_ASDIC(parent->get_pos().xy(),
				angle(delta));
			if (contacts.size() > 0) {	// update contact
				// fixme: choose best contact!
				attack_contact(contacts.front());
			}
		} else {
			parent->set_throttle(sea_object::aheadflank);
		}

		if (cd < DC_ATTACK_RADIUS) {
			gm.spawn_depth_charge(new depth_charge(*parent, -contact.z));
			// the escort must run with maximum speed until the depth charges
			// have exploded to avoid suicide. fixme
			// fixme: just ai hacking/testing.
			// after spawning a DC start pinging again.
			relax();
		}
	}
}

void ai::set_course_to_pos(const vector2& pos)
{
	vector2 d = pos - parent->get_pos().xy();
	vector2 hd = parent->get_heading().direction();
	double a = d.x*hd.x + d.y*hd.y;
	double b = d.x*hd.y - d.y*hd.x;
	// if a is < 0 then target lies behind our pos.
	// if b is < 0 then target is left, else right of our pos.
	double r1 = (b == 0) ? 1e10 : (a*a + b*b)/fabs(2*b);
	double r2 = 1.0/parent->get_turn_rate().rad();
	if (a <= 0) {	// target is behind us
		if (b < 0) {	// target is left
			parent->head_to_ang(parent->get_heading() - angle(180), true);
		} else {
			parent->head_to_ang(parent->get_heading() + angle(180), false);
		}
	} else if (r2 > r1) {	// target can not be reached with smallest curve possible
		if (b < 0) {	// target is left
			parent->head_to_ang(parent->get_heading() + angle(180), false);
		} else {
			parent->head_to_ang(parent->get_heading() - angle(180), true);
		}
	} else {	// target can be reached, steer curve
		parent->head_to_ang(angle::from_math(atan2(d.y, d.x)), (b < 0));
//	this code computes the curve that hits the target
//	but it is much better to turn fast and then steam straight ahead
/*
		double needed_turn_rate = (r1 == 0) ? 0 : 1.0/r1; //parent->speed/r1;
		double fac = ((180.0*needed_turn_rate)/PI)/fabs(parent->turn_rate.value_pm180());
		parent->head_chg = (b < 0) ? -fac : fac;
*/		
	}
}

void ai::act_dumb(game& gm, double delta_time)
{
	if (state == followobject && followme != 0) {
		set_course_to_pos(followme->get_pos().xy());
	} else if (state == followpath) {
		if (waypoints.size() > 0) {
			set_course_to_pos(waypoints.front());
			if (parent->get_pos().xy().distance(waypoints.front()) < WPEXACTNESS) {
				if (cyclewaypoints)
					waypoints.push_back(waypoints.front());
				waypoints.erase(waypoints.begin());
			}
		}
	}
}

void ai::fire_shell_at(game& gm, const sea_object& s)
{
	vector2 deltapos = s.get_pos().xy() - parent->get_pos().xy();
	double distance = deltapos.length();
	angle direction(deltapos);
	
	angle elevation = angle(30);	// fixme
	

	// fixme adapt direction & elevation to course and speed of target!	
	gm.spawn_gun_shell(new gun_shell(*parent, direction, elevation));
}






#if 0	// gunnery code
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <list>

const float PI = 3.1415962;
const float EPS = 0.1;

using namespace std;

float quantify(float a, float b)
{
	return round(a/b)*b;
}

float random_part(float min, float max)
{
	float d = max - min;
	float r = float(rand() % 1001) / 1000.0;
	return min + r * d;
}

float deg_to_rad(float deg)
{
	return deg * PI / 180.0;
}

float rad_to_deg(float rad)
{
	return rad * 180.0 / PI;
}

float estimate_distance(float d)
{
	if (fabs(d) > 10000.0)
		return round(d/2000.0)*2000.0;
	if (fabs(d) > 5000.0)
		return round(d/1000.0)*1000.0;
	if (fabs(d) > 2000.0)
		return round(d/500.0)*500.0;
	if (fabs(d) > 200.0)
		return round(d/50.0)*50.0;
	return round(d/25.0)*25.0;
}

float shot_speed(float a, float v0, float t)
{
	return v0*exp(-a*t/v0);
}

float shot_distance(float alpha, float a, float v0, float t)
{
	return cos(alpha)*t*shot_speed(a, v0, t);
}

float shot_height(float alpha, float a, float v0, float g, float t)
{
	return sin(alpha)*t*shot_speed(a, v0, t) + g*t*t/2;
}

float impact_distance(float alpha, float a, float v0, float g)
{
	float t = 120.0, delta_t = t/2;
	while(1) {
		float h = shot_height(alpha, a, v0, g, t);
		if (h > EPS)
			t += delta_t;
		else if (h < -EPS)
			t -= delta_t;
		else
			break;
		delta_t /= 2;
	}
	return shot_distance(alpha, a, v0, t);
}

float height(float alpha, float a, float v0, float g, float dist)
{
	float t = 120.0, delta_t = t/2;
	while(1) {
		float d = dist - shot_distance(alpha, a, v0, t);
		if (d > EPS)
			t += delta_t;
		else if (d < -EPS)
			t -= delta_t;
		else
			break;
		delta_t /= 2;
	}
	return shot_height(alpha, a, v0, g, t);
}

// shot angle estimation
std::list<std::pair<double, double> > estimations;
const int STEPS = 45;

float gauss(float x)
{
	return exp(-x*x/2);
}

float initialize_angle_estimation(float a, float v0, float g, float min_elev, float max_elev, float min_angle_diff)
{
	for (int i = 0; i <= STEPS; ++i) {
		float ang = min_elev + i * (max_elev - min_elev) / STEPS;
		ang = quantify(ang, min_angle_diff);
		float dist = impact_distance(deg_to_rad(ang), a, v0, g);
		dist = quantify(dist, 50.0);
		estimations.push_back(std::make_pair(ang, dist));
	}
}

float estimate_angle(float a, float v0, float g, float dist)
{
	std::list<std::pair<double, double> >::iterator it = estimations.begin();

//	std::list<std::pair<double, double> >::iterator it3 = estimations.begin();
//	for ( ; it3 != estimations.end(); ++it3)
//		cout << "learned: angle " << it3->first << ", " << it3->second << "\n";

	while (it != estimations.end() && it->second < dist)
		++it;
	if (it == estimations.end())
		return -1;
	if (it == estimations.begin())
		return 0;
	std::list<std::pair<double, double> >::iterator it2 = it;
	--it;
	float diff = (dist - it->second) / (it2->second - it->second);
	float ang = it->first + diff * (it2->first - it->first);
	return deg_to_rad(ang);
}

void learn_angle_dist_relation(float ang, float dist)
{
	std::list<std::pair<double, double> >::iterator it = estimations.begin();
	while (it != estimations.end() && it->first < ang)
		++it;
	if (it == estimations.end())
		return;
	if (it == estimations.begin())
		return;
	if (fabs(it->first - ang) < EPS)
		it->second = dist;
	else
		estimations.insert(it, std::make_pair(ang, dist));

	// sort distance part
	it = estimations.begin();
	while (it != estimations.end()) {
		std::list<std::pair<double, double> >::iterator it2 = it;
		++it2;
		while (it2 != estimations.end()) {
			if (it->second > it2->second) {
				float tmp = it2->second;
				it2->second = it->second;
				it->second = tmp;
			}
			++it2;
		}
		++it;
	}
}

float hit(float h1, float h2, float eh)
{
	return (h2 <= eh && h1 >= -10.0);
}

int main(int argc, char** argv)
{
	float enemy_distance = 22000;
	float enemy_width = 30;
	float min_elev = 0;
	float max_elev = 45;
	float g = -9.8062;
	float v0 = 600;
	float a = 2.0;	// air resistance
	float min_angle_diff = 0.1;	// very small. but must be that size!?
	float enemy_height = 30.0;

	if (argc > 1)
		enemy_distance = atof(argv[1]);
	
	srand(time(0));
	
	initialize_angle_estimation(a, v0, g, min_elev, max_elev, min_angle_diff);

	// see enemy and estimate range
	cout << "Gunnery test.\nEnemy range " << enemy_distance << "\n";
	float estimated_distance = estimate_distance(enemy_distance * random_part(0.95, 1.05));
	cout << "Estimated range " << estimated_distance << "\n";

	float wind_resist = random_part(-0.5, 0.5);
	a += wind_resist;

	float alpha = estimate_angle(a, v0, g, estimated_distance);
	alpha = quantify(alpha, deg_to_rad(8*min_angle_diff));
	float alpha2 = rad_to_deg(alpha);
	if (alpha < 0) {
		cout << "Target out of range!\n";
		return 0;
	}
	cout << "Inital elevation " << alpha2 << " degrees.\n";

	// now fire, check for hit and retry
	int hits = 0;
	while (hits < 4) {
		// calculate height of projectile over target
		float h1 = height(alpha, a, v0, g, enemy_distance - enemy_width/2);
		float h2 = height(alpha, a, v0, g, enemy_distance + enemy_width/2);
		float impact_angle = rad_to_deg(atan((h1 - h2)/enemy_width));
		if (hit(h1, h2, enemy_height)) {
			cout << "Hit!!! Angle " << impact_angle << "\n";
			++hits;
		}
		cout << "Heights over target " << h1 << ", " << h2 << "\n";
		float id = impact_distance(alpha, a, v0, g);
		cout << "Impact at " << id << "\n";
		float estimated_fault = estimate_distance(id - enemy_distance + random_part(-10, 10));
		cout << "Estimated fault " << estimated_fault << "\n";

		estimated_distance -= estimated_fault;

		alpha = estimate_angle(a, v0, g, estimated_distance);
		alpha = quantify(alpha, deg_to_rad(min_angle_diff));
		alpha2 = rad_to_deg(alpha);
		if (alpha < 0) {
			cout << "Target out of range!\n";
			return 0;
		}
		cout << "New elevation " << alpha2 << " degrees.\n";
	}
}
#endif
