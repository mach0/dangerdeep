// sea objects
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sea_object.h"
#include "vector2.h"
#include "tokencodes.h"

sea_object::sea_object()
{
	position = vector3(0, 0, 0);
	heading = 0;
	speed = 0;
	max_speed = 0;
	max_rev_speed = 0;
	throttle = stop;
	acceleration = 0;
	permanent_turn = false;
	head_chg = 0;
	head_to = 0;
	turn_rate = 0;
	length = width = 0;
	alive_stat = alive;
}

bool sea_object::parse_attribute(parser& p)
{
	switch (p.type()) {
		case TKN_POSITION: {
			p.consume();
			p.parse(TKN_ASSIGN);
			int x = p.parse_number();
			p.parse(TKN_COMMA);
			int y = p.parse_number();
			p.parse(TKN_COMMA);
			int z = p.parse_number();
			p.parse(TKN_SEMICOLON);
			position = vector3(x, y, z);
			break; }
		case TKN_HEADING:
			p.consume();
			p.parse(TKN_ASSIGN);
			heading = angle(p.parse_number());
			p.parse(TKN_SEMICOLON);
			break;
		case TKN_THROTTLE:
			p.consume();
			p.parse(TKN_ASSIGN);
			switch (p.type()) {
				case TKN_STOP: throttle = stop; break;
				case TKN_REVERSE: throttle = reverse; break;
				case TKN_AHEADLISTEN: throttle = aheadlisten; break;
				case TKN_AHEADSONAR: throttle = aheadsonar; break;
				case TKN_AHEADSLOW: throttle = aheadslow; break;
				case TKN_AHEADHALF: throttle = aheadhalf; break;
				case TKN_AHEADFULL: throttle = aheadfull; break;
				case TKN_AHEADFLANK: throttle = aheadflank; break;
				default: p.error("Expected throttle value");
			}
			p.consume();
			p.parse(TKN_SEMICOLON);
			break;
		default: return false;
	}
	return true;
}

void sea_object::simulate(game& gm, double delta_time)
{
	// calculate sinking
	if (is_defunct()) {
		return;
	} else if (is_dead()) {
		alive_stat = defunct;
		return;
	} else if (is_sinking()) {
		position.z -= delta_time * SINK_SPEED;
		if (position.z < -50)	// used for ships.
			kill();
		throttle = stop;
		rudder_midships();
		return;
	}

	// calculate directional speed
	vector2 dir_speed_2d = heading.direction() * speed;
	vector3 dir_speed(dir_speed_2d.x, dir_speed_2d.y, 0);
	
	// calculate new position
	position += dir_speed * delta_time;
	
	// calculate speed change
	double t = get_throttle_speed() - speed;
	double s = acceleration * delta_time;
	if (fabs(t) > s) {
		speed += (t < 0) ? -s : s;
	} else {
		speed = get_throttle_speed();
	}

	// calculate heading change (fixme, this turning is not physically correct)
	angle maxturnangle = turn_rate * (head_chg * delta_time * speed);
	if (permanent_turn) {
		heading += maxturnangle;
	} else {
		double fac = (head_to - heading).value_pm180()
			/ maxturnangle.value_pm180();
		if (0 <= fac && fac <= 1) {
			rudder_midships();
			heading = head_to;
		} else {
			heading += maxturnangle;
		}
	}
}

bool sea_object::is_collision(const sea_object* other)
{
	if (is_defunct() || is_dead() || other->is_defunct() || other->is_dead()) return false;
	// korrekt w�re die beiden Rechtecke von *this und other auf Schnitt zu testen, fixme
	vector2 headdir = heading.direction();
	vector2 other_headdir = other->heading.direction();
	vector2 pos = position.xy();
	vector2 other_pos = other->position.xy();
	double s, t;
	bool solved = (pos - other_pos).solve(-headdir, other_headdir, s, t);
	if (solved) {
		// quick hack. je mehr heading/other.heading unterschiedlich sind, desto besser
		return (fabs(s) <= length/2 + width/2 && fabs(t) <= other->length/2 + other->width/2);
	}
	return false;
}

bool sea_object::is_collision(const vector2& pos)
{
	if (is_defunct() || is_dead()) return false;
	// a bit slower than neccessary. fixme
	vector2 headdir = heading.direction();
	vector2 nheaddir = headdir.orthogonal();
	vector2 mypos = position.xy();
	double s, t;
	bool solved = (mypos - pos).solve(headdir, nheaddir, s, t);
	if (solved) {
		return (fabs(s) <= length/2 && fabs(t) <= width/2);
	}
	return false;
}

void sea_object::damage(const vector3& fromwhere, unsigned strength)
{
	sink();
}

unsigned sea_object::calc_damage(void) const
{
	return alive_stat == sinking ? 100 : 0;
}

void sea_object::sink(void)
{
	alive_stat = sinking;
}

void sea_object::kill(void)
{
	alive_stat = dead;
}

void sea_object::head_to_ang(const angle& a, bool left_or_right)	// true == left
{
	head_to = a;
	head_chg = (left_or_right) ? -1 : 1;
	permanent_turn = false;
}

void sea_object::rudder_left(double amount)	// 0 <= amount <= 1
{
	head_chg = -amount;
	permanent_turn = true;
}

void sea_object::rudder_right(double amount)	// 0 <= amount <= 1
{
	head_chg = amount;
	permanent_turn = true;
}

void sea_object::rudder_midships(void)
{
	head_chg = 0;
	head_to = heading;
	permanent_turn = false;
}

void sea_object::set_throttle(throttle_status thr)
{
	throttle = thr;
}

double sea_object::get_throttle_speed(void) const
{
	double ms = get_max_speed();
	switch (throttle) {
		case reverse: return -ms*0.25;
		case stop: return 0;
		case aheadlisten: return ms*0.25;
		case aheadsonar: return ms*0.25;
		case aheadslow: return ms/3.0;
		case aheadhalf: return ms*0.5;
		case aheadfull: return ms*0.75;
		case aheadflank: return ms;
	}
	return 0;
}

pair<angle, double> sea_object::bearing_and_range_to(const sea_object* other) const
{
	vector2 diff = other->get_pos().xy() - position.xy();
	return make_pair(angle(diff), diff.length());
}

angle sea_object::estimate_angle_on_the_bow(angle target_bearing, angle target_heading) const
{
	return (angle(180) + target_bearing - target_heading).value_pm180();
}
