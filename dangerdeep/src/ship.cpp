// ships
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "ship.h"
#include "model.h"
#include "game.h"

ship::ship(unsigned type_, const vector3& pos, angle heading)
{
	init_empty();
	type = type_;
	position = pos;
	this->heading = heading;
	head_to = heading;
	switch (type_) {
		case 0:
			speed = kts2ms(8);
			max_speed = kts2ms(8);
			max_rev_speed = 0;
			acceleration = 0.1;
			turn_rate = 0.1;
			length = merchant_medium->get_length();
			width = merchant_medium->get_width();
			myai = new ai(this, ai::dumb);
			break;
		case 1:
			speed = kts2ms(8);
			max_speed = kts2ms(14);
			max_rev_speed = 0;
			acceleration = 0.1;
			turn_rate = 0.09;
			length = troopship_medium->get_length();
			width = troopship_medium->get_width();
			myai = new ai(this, ai::dumb);
			break;
		case 2:
			speed = kts2ms(8);
			max_speed = kts2ms(34);
			max_rev_speed = 0;
			acceleration = 1.0;
			turn_rate = 0.2;
			length = destroyer_tribal->get_length();
			width = destroyer_tribal->get_width();
			myai = new ai(this, ai::escort);
			break;
		case 3:
			speed = kts2ms(8);
			max_speed = kts2ms(24);
			max_rev_speed = 0;
			acceleration = 0.4;
			turn_rate = 0.08;
			length = battleship_malaya->get_length();
			width = battleship_malaya->get_width();
			myai = new ai(this, ai::dumb);
			myai->set_zigzag();	// test
			break;
	}
	
	throttle = aheadhalf;	// fixme
}

void ship::simulate(game& gm, double delta_time)
{
	sea_object::simulate(gm, delta_time);
	myai->act(gm, delta_time);
}

void ship::fire_shell_at(const vector2& pos)
{
}

void ship::display(void) const
{
	switch(type) {
		case 0:	merchant_medium->display(); break;
		case 1:	troopship_medium->display(); break;
		case 2:	destroyer_tribal->display(); break;
		case 3:	battleship_malaya->display(); break;
	}
}
