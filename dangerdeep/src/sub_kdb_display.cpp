/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

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

// user display: submarine's kdb hearing device
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#include "sub_kdb_display.h"
#include "system.h"
#include "image.h"
#include "texture.h"
#include "game.h"
#include "submarine_interface.h"
#include "submarine.h"
#include "keys.h"
#include "cfg.h"
#include "global_data.h"
#include <sstream>
using namespace std;

static const double TK_ANGFAC = 360.0/512.0;
static const unsigned TK_PHASES = 6;

sub_kdb_display::sub_kdb_display(user_interface& ui_)
	: user_display(ui_), turnknobdrag(TK_NONE), turnknobang(TK_NR)
{
	daylight.background.reset(new image(get_image_dir() + "KDB_daylight_background.jpg"));
	redlight.background.reset(new image(get_image_dir() + "KDB_redlight_background.jpg"));
	daylight.direction_ptr.set("KDB_daylight_pointer.png", 323, 122, 377, 373);
	redlight.direction_ptr.set("KDB_redlight_pointer.png", 323, 122, 377, 373);

	for (unsigned i = 0; i < TK_PHASES; ++i) {
		ostringstream osn;
		osn << (i+1) ;
		daylight.turn_wheel[i].set("KDB_daylight_gauge" + osn.str() + ".png", 166, 682);
		redlight.turn_wheel[i].set("KDB_redlight_gauge" + osn.str() + ".png", 166, 682);
		daylight.volume_knob[i].set("KDB_daylight_knob" + osn.str() + ".png", 683, 667);
		redlight.volume_knob[i].set("KDB_redlight_knob" + osn.str() + ".png", 683, 667);
	}
}



void sub_kdb_display::process_input(class game& gm, const SDL_Event& event)
{
	submarine* sub = dynamic_cast<submarine*>(gm.get_player());
	bool is_day = gm.is_day_mode();
	int mx, my, mb;
	submarine_interface& si = dynamic_cast<submarine_interface&>(ui);

	const scheme& s = (is_day) ? daylight : redlight;

	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		mx = event.button.x;
		my = event.button.y;
		// check if mouse is over turn knobs
		turnknobdrag = TK_NONE;
		if (s.volume_knob[0].is_mouse_over(mx, my)) {
			turnknobdrag = TK_VOLUME;
		} else if (s.turn_wheel[0].is_mouse_over(mx, my, 128)) {
			turnknobdrag = TK_DIRECTION;
		}
		break;
	case SDL_MOUSEMOTION:
		mx = event.motion.xrel;
		my = event.motion.yrel;
		mb = event.motion.state;
		if (event.motion.state & SDL_BUTTON_LMASK) {
			if (turnknobdrag != TK_NONE) {
				float& ang = turnknobang[unsigned(turnknobdrag)];
				ang += mx * TK_ANGFAC;
				switch (turnknobdrag) {
				case TK_DIRECTION:
					// bring to 0...360 degree value
					ang = myfmod(ang, 720.0f);
					//sub->set_kdb_direction(ang); // fixme: set angle of player
					break;
				case TK_VOLUME:
					// 0-360 degrees possible
					ang = myclamp(ang, 0.0f, 360.0f);
					break;
				default:	// can never happen
					break;
				}
			}
		}
		break;
	case SDL_MOUSEBUTTONUP:
		mx = event.button.x;
		my = event.button.y;
		turnknobdrag = TK_NONE;
		break;
	default:
		break;
	}
}



void sub_kdb_display::display(class game& gm) const
{
	submarine* player = dynamic_cast<submarine*>(gm.get_player());

	sys().prepare_2d_drawing();
	glColor3f(1,1,1);

	// determine time of day
	bool is_day = gm.is_day_mode();

	// get hearing device angle from submarine, if it has one

	const scheme& s = (is_day) ? daylight : redlight;

	s.background->draw(0, 0);
	s.volume_knob[unsigned(myfmod(-turnknobang[TK_VOLUME]*0.5f, 90.0f)) * TK_PHASES / 90].draw();
	s.turn_wheel[unsigned(myfmod(-turnknobang[TK_DIRECTION] * 2.0f, 90.0f)) * TK_PHASES / 90].draw();
	s.direction_ptr.draw(turnknobang[TK_DIRECTION] * 0.5f /* fixme: get angle from player*/);

	// test hack: test signal strengths
	angle app_ang = angle(turnknobang[TK_DIRECTION]*0.5);
	angle sonar_ang = app_ang + player->get_heading();
	vector<double> noise_strengths = gm.sonar_listen_ships(player, sonar_ang);
	printf("noise strengths, global ang=%f, L=%f M=%f H=%f U=%f\n",
	       sonar_ang.value(), noise_strengths[0], noise_strengths[1], noise_strengths[2], noise_strengths[3]);

	// fixme: add test here
	// Simulate sonar man.
	// From current apparatus angle turn some degrees left or right, until the operator
	// can tell wether the signal gets stronger or weaker.
	// If it gets weaker, choose the other direction as initial direction.
	// Then turn the apparatus by larger steps (10-30 degrees), as long as signal gets
	// stronger. If it gets weaker, try again from strongest direction with half the
	// distance, but with opposite direction. If that direction gives weaker signal on
	// second try, reverse direction (same as global search, but with less step length).
	// If we found two angles where the signal gets weaker in between, try from the strongest
	// and reverse direction with 1� steps iterativly.
	// Example: strongest signal at 33�, initial angle at 50�, initial direction right/
	// clockwise. Operator turns at 60� and hears that signal is weaker, so he uses
	// 50� as initial angle and left/counter-clockwise as initial direction.
	// He turns left 10� to 40�, signal gets stronger.
	// He turns left 10� to 30�, signal gets stronger.
	// He turns left 10� to 20�, signal gets weaker. So he changes direction, back at 30�. Stepping down to 5�.
	// He turns right 5� to 35�, signal gets stronger.
	// He turns right 5� to 40�, signal gets weaker. So he changes direction, back at 35�. Stepping down to 1�.
	// He turns left 1� to 34�, signal gets stronger.
	// He turns left 1� to 33�, signal gets stronger.
	// He turns left 1� to 32�, signal gets weaker. So he turns back to strongest signal and stops. -> 33�
	// Example2: strongest signal at 29�, initial angle at 50�, initial direction right/
	// clockwise. Operator turns at 60� and hears that signal is weaker, so he uses
	// 50� as initial angle and left/counter-clockwise as initial direction.
	// He turns left 10� to 40�, signal gets stronger.
	// He turns left 10� to 30�, signal gets stronger.
	// He turns left 10� to 20�, signal gets weaker. So he changes direction, back at 30�. Stepping down to 5�.
	// He turns right 5� to 35�, signal gets weaker. So initial direction wrong, turn left.
	// He turns left 5� to 25�, signal gets weaker. So he changes direction, back at 30�. Stepping down to 1�.
	// He turns right 1� to 31�, signal gets weaker. So initial direction wrong, turn left.
	// He turns left 1� to 29�, signal gets stronger.
	// He turns left 1� to 28�, signal gets weaker. So he turns back to strongest signal and stops. -> 29�
	// We could simulate four steps. 30�, 10�, 5�, 1�.

	ui.draw_infopanel();

	sys().unprepare_2d_drawing();
}
