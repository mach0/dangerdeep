// airplanes
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "airplane.h"
#include "model.h"

airplane::airplane(unsigned type_, const vector3& pos, double heading) : sea_object()
{
	type = type_;
	position = pos;
	this->heading = heading;
	head_to = heading;
//	turn_rate = deg2rad(5);
//	length = 7;
//	width = 1;
	switch (type_) {
/*
		case 3:
			speed = 8;
			max_speed = 17.6;
			max_rev_speed = 5;
			acceleration = 0.8;
			turn_rate = 1;
			break;
*/			
	}
	vis_cross_section_factor = CROSS_SECTION_VIS_MSUB_AC;
	
	throttle = aheadfull;
}

void airplane::load(istream& in, class game& g)
{
	sea_object::load(in, g);
	type = read_u32(in);
}

void airplane::save(ostream& out, const class game& g) const
{
	sea_object::save(out, g);
	write_u32(out, type);
}

void airplane::display(void) const
{
	switch(type) {
	}
}
