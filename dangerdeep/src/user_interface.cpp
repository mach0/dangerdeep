// user interface common code
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "oglext/OglExt.h"
#include <GL/glu.h>
#include <SDL.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include "user_interface.h"
#include "system.h"
#include "game.h"
#include "texts.h"
#include "sound.h"
#include "model.h"
#include "airplane.h"
#include "depth_charge.h"
#include "gun_shell.h"
#include "water_splash.h"
#include "vector3.h"
#include "widget.h"
#include "command.h"
#include "submarine_interface.h"
#include "submarine.h"	// needed for underwater sound reduction
//#include "ship_interface.h"
//#include "airplane_interface.h"
#include "sky.h"
#include "water.h"
#include "matrix4.h"
using namespace std;

#define MAX_PANEL_SIZE 256


/*
	a note on our coordinate system (11/10/2003):
	We simulate earth by projecting objects according to curvature from earth
	space to Euclidian space. This projection is yet a identity projection, that means
	we ignore curvature yet.
	The map forms a cylinder around the earth, that means x,y position on the map translates
	to longitude,latitude values. Hence valid coordinates go from -20000km...20000km in x
	direction and -10000km to 10000km in y direction. (we could use exact values, around
	20015km). The wrap around is a problem, but that's somewhere in the Pacific ocean, so
	we just ignore it. This mapping leads to some distorsion and wrong distance values
	when coming to far north or south on the globe. We just ignore this for simplicity's
	sake. The effect shouldn't be noticeable.
*/

user_interface::user_interface(game& gm) :
	pause(false), time_scale(1),
	panel_visible(true),
	current_display(0), target(0), mysky(0), mywater(0),
	mycoastmap(get_map_dir() + "default.xml")
{
	mysky = new sky();
	mywater = new class water(128, 128, 0.0);//fixme: make detail configureable
	panel = new widget(0, 768-128, 1024, 128, "", 0, panelbackgroundimg);
	panel_messages = new widget_list(8, 8, 512, 128 - 2*8);
	panel->add_child(panel_messages);
	panel->add_child(new widget_text(528, 8, 0, 0, texts::get(1)));
	panel->add_child(new widget_text(528, 8+24+5, 0, 0, texts::get(4)));
	panel->add_child(new widget_text(528, 8+48+10, 0, 0, texts::get(5)));
	panel->add_child(new widget_text(528, 8+72+15, 0, 0, texts::get(2)));
	panel->add_child(new widget_text(528+160, 8, 0, 0, texts::get(98)));
	panel_valuetexts[0] = new widget_text(528+100, 8, 0, 0, "000");
	panel_valuetexts[1] = new widget_text(528+100, 8+24+5, 0, 0, "000");
	panel_valuetexts[2] = new widget_text(528+100, 8+48+10, 0, 0, "000");
	panel_valuetexts[3] = new widget_text(528+100, 8+72+15, 0, 0, "000");
	panel_valuetexts[4] = new widget_text(528+160+100, 8, 0, 0, "000");
	for (unsigned i = 0; i < 5; ++i)
		panel->add_child(panel_valuetexts[i]);
	panel->add_child(new widget_caller_button<game, void (game::*)(void)>(&gm, &game::stop, 1024-128-8, 128-40, 128, 32, texts::get(177)));
}

user_interface* user_interface::create(game& gm)
{
	sea_object* p = gm.get_player();
	submarine* su = dynamic_cast<submarine*>(p); if (su) return new submarine_interface(gm);
	//ship* sh = dynamic_cast<ship*>(p); if (sh) return new ship_interface(gm);
	//airplane* ap = dynamic_cast<airplane*>(p); if (ap) return new airplane_interface(gm);
	return 0;
}

user_interface::~user_interface ()
{
	for (vector<user_display*>::iterator it = displays.begin(); it != displays.end(); ++it)
		delete *it;

	delete panel;

	delete mysky;
	delete mywater;
}



void user_interface::display(game& gm) const
{
	displays[current_display]->display(gm);
}



void user_interface::process_input(class game& gm, const SDL_Event& event)
{
	displays[current_display]->process_input(gm, event);
}



void user_interface::process_input(class game& gm, const list<SDL_Event>& events)
{
	for (list<SDL_Event>::const_iterator it = events.begin();
	     it != events.end(); ++it)
		process_input(gm, *it);
}



/* 2003/07/04 idea.
   simulate earth curvature by drawing several horizon faces
   approximating the curvature.
   earth has medium radius of 6371km, that means 40030km around it.
   A ship with 15m height above the waterline disappears behind
   the horizon at ca. 13.825km distance (7.465 sm)
   
   exact value 40030.17359km. (u), earth radius (r)
   
   height difference in view: (h), distance (d). Formula:
   
   h = r * (1 - cos( 360deg * d / u ) )
   
   or
   
   d = arccos ( 1 - h / r ) * u / 360deg
   
   draw ships with height -h. so (dis)appearing of ships can be
   simulated properly.
   
   highest ships are battleships (approx. 30meters), they disappear
   at 19.551km (10.557 sm).
   
   That's much shorter than I thought! But there is a mistake:
   The viewer's height is not 0 but around 6-8m for submarines,
   so the formulas are more difficult:
   
   The real distance is twice the formula, once for the viewer's
   height, once for the object:
   
   d = (arccos(1 - myh/r) + arccos(1 - h/r)) * u / 360deg
   
   or for the watched object
   
   h = r * (1 - cos( 360deg * (d - (arccos(1 - myh/r)) / u ) )
   
   so for a watcher in 6m height and other ships we have
   arccos(1-myh/r) = 0.07863384deg
   15m in height -> dist: 22.569km (12.186sm)
   30m in height -> dist: 28.295km (15.278sm)
   
   This values are useful for computing "normal" simulation's
   maximum visibility.
   Waves are disturbing sight but are ignored here.
*/	   

void user_interface::rotate_by_pos_and_wave(const vector3& pos,
	double rollfac, bool inverse) const
{
	vector3f rz = mywater->get_normal(pos.xy(), rollfac);
	vector3f rx = vector3f(1, 0, -rz.x).normal();
	vector3f ry = vector3f(0, 1, -rz.y).normal();
	if (inverse) {
		float mat[16] = {
			rx.x, ry.x, rz.x, 0,
			rx.y, ry.y, rz.y, 0,
			rx.z, ry.z, rz.z, 0,
			0,    0,    0,    1 };
		glMultMatrixf(&mat[0]);
	} else {
		float mat[16] = {
			rx.x, rx.y, rx.z, 0,
			ry.x, ry.y, ry.z, 0,
			rz.x, rz.y, rz.z, 0,
			0,    0,    0,    1 };
		glMultMatrixf(&mat[0]);
	}
}

void user_interface::draw_terrain(const vector3& viewpos, angle dir,
	double max_view_dist) const
{
#if 1
	glPushMatrix();
	glTranslatef(0, 0, -viewpos.z);
	terraintex->set_gl_texture();
	mycoastmap.render(viewpos.xy());
	glPopMatrix();
#endif
}

bool user_interface::time_scale_up(void)
{
	if (time_scale < 4096) {
		time_scale *= 2;
		return true;
	}
	return false;
}

bool user_interface::time_scale_down(void)
{
	if (time_scale > 1) {
		time_scale /= 2;
		return true;
	}
	return false;
}

void user_interface::draw_infopanel(class game& gm) const
{
	if (panel_visible) {
		ostringstream os0;
		os0 << setw(3) << left << gm.get_player()->get_heading().ui_value();
		panel_valuetexts[0]->set_text(os0.str());
		ostringstream os1;
		os1 << setw(3) << left << unsigned(fabs(round(sea_object::ms2kts(gm.get_player()->get_speed()))));
		panel_valuetexts[1]->set_text(os1.str());
		ostringstream os2;
		os2 << setw(3) << left << unsigned(round(-gm.get_player()->get_pos().z));
		panel_valuetexts[2]->set_text(os2.str());
		ostringstream os3;
		os3 << setw(3) << left << bearing.ui_value();
		panel_valuetexts[3]->set_text(os3.str());
		ostringstream os4;
		os4 << setw(3) << left << time_scale;
		panel_valuetexts[4]->set_text(os4.str());

		panel->draw();
		// let aside the fact that we should divide DRAWING and INPUT HANDLING
		// the new process_input function eats SDL_Events which we don't have here
//		panel->process_input(true);
	}
}



void user_interface::add_message(const string& s)
{
	panel_messages->append_entry(s);
	if (panel_messages->get_listsize() > panel_messages->get_nr_of_visible_entries())
		panel_messages->delete_entry(0);
/*
	panel_texts.push_back(s);
	if (panel_texts.size() > 1+MAX_PANEL_SIZE/font_arial->get_height())
		panel_texts.pop_front();
*/
}



void user_interface::add_rudder_message(game& gm)
{
	// this whole function should be replaced...seems ugly
	ship* s = dynamic_cast<ship*>(gm.get_player());
	if (!s) return;	// ugly hack to allow compilation
	switch (s->get_rudder_to())
		{
		case ship::rudderfullleft:
			add_message(texts::get(35));
			break;
		case ship::rudderleft:
			add_message(texts::get(33));
			break;
		case ship::ruddermidships:
			add_message(texts::get(42));
			break;
		case ship::rudderright:
			add_message(texts::get(34));
			break;
		case ship::rudderfullright:
			add_message(texts::get(36));
			break;
		}

}



#define DAY_MODE_COLOR() glColor3f ( 1.0f, 1.0f, 1.0f )

#define NIGHT_MODE_COLOR() glColor3f ( 1.0f, 0.4f, 0.4f )

void user_interface::set_display_color ( color_mode mode ) const
{
	switch ( mode )
	{
		case night_color_mode:
			NIGHT_MODE_COLOR ();
			break;
		default:
			DAY_MODE_COLOR ();
			break;
	}
}

void user_interface::set_display_color ( const class game& gm ) const
{
	if ( gm.is_day_mode () )
		DAY_MODE_COLOR ();
	else
		NIGHT_MODE_COLOR ();
}

sound* user_interface::get_sound_effect ( game& gm, sound_effect se ) const
{
	sound* s = 0;

	switch ( se )
	{
		case se_submarine_torpedo_launch:
			s = torpedo_launch_sound;
			break;
		case se_torpedo_detonation:
			{
				submarine* sub = dynamic_cast<submarine*>(gm.get_player());

				if ( sub && sub->is_submerged () )
				{
					double sid = rnd ( 2 );
					if ( sid < 1.0f )
						s = torpedo_detonation_submerged[0];
					else if ( sid < 2.0f )
						s = torpedo_detonation_submerged[1];
				}
				else
				{
					double sid = rnd ( 2 );
					if ( sid < 1.0f )
						s = torpedo_detonation_surfaced[0];
					else if ( sid < 2.0f )
						s = torpedo_detonation_surfaced[1];
				}
			}
			break;
	}

	return s;
}

void user_interface::play_sound_effect ( game& gm, sound_effect se, double volume ) const
{
	sound* s = get_sound_effect ( gm, se );

	if ( s )
		s->play ( volume );
}



void user_interface::play_sound_effect_distance(game& gm, sound_effect se, double distance) const
{
	sound* s = get_sound_effect(gm, se);

	if ( s )
	{
		double h = 3000.0f;
		submarine* sub = dynamic_cast<submarine*> ( gm.get_player () );
		if ( sub && sub->is_submerged () )
			h = 10000.0f;

		s->play ( ( 1.0f - gm.get_player()->get_noise_factor () ) * exp ( - distance / h ) );
	}
}

/*
void user_interface::add_captains_log_entry ( class game& gm, const string& s)
{
	date d(unsigned(gm.get_time()));

	if ( captains_logbook )
		captains_logbook->add_entry( d, s );
}

inline void user_interface::record_sunk_ship ( const ship* so )
{
	ships_sunk_disp->add_sunk_ship ( so );
}
*/

