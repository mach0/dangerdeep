// game
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef GAME_H
#define GAME_H

#define PINGREMAINTIME 1.0	// seconds
#define PINGANGLE 15		// angle
#define PINGLENGTH 1000		// meters. for drawing
#define ASDICRANGE 1500.0	// meters fixme: historic values?
#define MAX_ACUSTIC_CONTACTS 5	// max. nr of simultaneous trackable acustic contacts

#include <list>
#include <vector>
using namespace std;

// use forward declarations to avoid unneccessary compile dependencies
class ship;
class submarine;
class airplane;
class torpedo;
class depth_charge;
class gun_shell;
class model;
class global_data;
class sea_object;
class network_connection;
class user_interface;
class particle;
class convoy;

#include "angle.h"
#include "date.h"
#include "vector2.h"
#include "vector3.h"
#include "color.h"
#include "logbook.h"

// network messages
#define MSG_length	16
#define MSG_cancel	"DFTD-cancel!    "
#define MSG_ask		"DFTD-ask?       "
#define MSG_offer	"DFTD-offer!     "
#define MSG_join	"DFTD-join?      "
#define MSG_joined	"DFTD-joined!    "
#define MSG_initgame	"DFTD-init!      "
#define MSG_ready	"DFTD-ready!     "
#define MSG_start	"DFTD-start!     "
#define MSG_gamestate	"DFTD-gamestate: "
#define MSG_command	"DFTD-command:   "



class game	// our "world" with physics etc.
{
public:
	struct ping {
		vector2 pos;
		angle dir;
		double time;
		double range;
		angle ping_angle;
		ping(const vector2& p, angle d, double t, const double& range,
			const angle& ping_angle_ ) :
			pos(p), dir(d), time(t), range ( range ), ping_angle ( ping_angle_ )
			{}
		~ping() {}
		ping(istream& in);
		void save(ostream& out) const;
	};

	struct sink_record {
		date dat;
		string descr;	// fixme: store type, use a static ship function to retrieve a matching description
		string mdlname;	// model file name string
		unsigned tons;
		sink_record(date d, const string& s, const string& m, unsigned t) : dat(d), descr(s), mdlname(m), tons(t) {}
		sink_record(const sink_record& s) : dat(s.dat), descr(s.descr), mdlname(s.mdlname), tons(s.tons) {}
		~sink_record() {}
		sink_record& operator= (const sink_record& s) { dat = s.dat; descr = s.descr; mdlname = s.mdlname; tons = s.tons; return *this; }
		sink_record(istream& in);
		void save(ostream& out) const;
	};
	
	struct job {
		job() {}
		virtual void run(void) = 0;
		virtual double get_period(void) const = 0;
		virtual ~job() {}
	};

	// in which state is the game (return value of exec())
	// normal mode (running), or stop on next cycle (reason given by value)
	enum run_state { running, player_killed, mission_complete, contact_lost };
	
	user_interface* get_ui() { return ui; }
	
protected:
	list<ship*> ships;	// fixme: maybe vectors would be better here (simpler, faster access)
	list<submarine*> submarines; // reserve space for torps,dcs,gunshells,watersplashes
	list<airplane*> airplanes; // the rest doesn't change very often, array is small etc.
	list<torpedo*> torpedoes;
	list<depth_charge*> depth_charges;
	list<gun_shell*> gun_shells;
	list<convoy*> convoys;
	list<particle*> particles;
	run_state my_run_state;
	bool stopexec;	// if this is true, execution stops and the menu is displayed
	
	list<pair<double, job*> > jobs;	// generated by interface construction, no gameplay data
	
	// the player and matching ui (note that playing is not limited to submarines!)
	sea_object* player;

	// in theory, the game should know nothing about the user interface! fixme
	// what is ui needed for: display messages, play sounds, add to captains
	// log, add sunken ship record, get time scale*, display ui*
	// if game is paused*, *=should move from game::exec to subsim::exec_game
	// instead of calling ui we should store events and let the ui create messages
	// or sounds from them on its own. record and logbook should be part of game.
	user_interface* ui;	// can be zero, is set from game's owner

	list<sink_record> sunken_ships;

	logbook players_logbook;
	
	double time;	// global time (in seconds since 1.1.1939, 0:0 hrs) (universal time!)
	double last_trail_time;	// for position trail recording
	
	enum weathers { sunny, clouded, raining, storm };//fixme
	double max_view_dist;	// maximum visibility according to weather conditions
	
	list<ping> pings;
	
	// network game type (0 = single player, 1 = server, 2 = client)
	unsigned networktype;
	// the connection to the server (zero if this is the server)
	network_connection* servercon;
	// the connections to the clients (at least one if this is the server, else empty)
	vector<network_connection*> clientcons;

	game();	
	game& operator= (const game& other);
	game(const game& other);
	
	unsigned listsizes(unsigned n) const;	// counts # of list elemens over n lists above

public:
	// create new custom mission
	// expects: size small,medium,large, escort size none,small,medium,large,
	// time of day [0,4) night,dawn,day,dusk
	game(const string& subtype, unsigned cvsize, unsigned cvesc, unsigned timeofday,
		unsigned timeperiod, unsigned nr_of_players = 1);

	// create from mission file
	game(class TiXmlDocument* doc);

	virtual ~game();

	// game types: mission, career, multiplayer mission, multiplayer career/patrol (?)
	// fixme: add partial_load flag for network vs. savegame (full save / variable save)
	// maybe only needed for loading (build structure or not)
	virtual void save(const string& savefilename, const string& description) const;
	game(const string& savefilename);	// load a game
	game(istream& in);			// create a game from a stream
	static string read_description_of_savegame(const string& filename);
	virtual void save_to_stream(ostream& out) const;
	virtual void load_from_stream(istream& in);

	void stop(void) { stopexec = true; }

	void compute_max_view_dist(void);	// fixme - public?
	void simulate(double delta_t);

	const list<sink_record>& get_sunken_ships(void) const { return sunken_ships; };
	const logbook& get_players_logbook(void) const { return players_logbook; }
	void add_logbook_entry(const string& s);
	double get_time(void) const { return time; };
	double get_max_view_distance(void) const { return max_view_dist; }
	/**
		This method is needed to verify for day and night mode for the
		display methods within the user interfaces.
		@return true when day mode, false when night mode
	*/
	bool is_day_mode () const;
	/**
		This method calculates a depth depending factor. A deep diving
		submarine is harder to detect with ASDIC than a submarine at
		periscope depth.
		@param sub location vector of submarine
		@return depth factor
	*/
	virtual double get_depth_factor ( const vector3& sub ) const;
	
	sea_object* get_player(void) const { return player; }
	void set_user_interface(user_interface* ui_) { ui = ui_; }

	// compute visibility data
	// fixme: gcc3.2+ optimizes return values that are complex data types.
	// hence change signature to list<xxx*> visible_xxxs(const...);
	// fixme: these functions should be const...
	virtual void visible_ships(list<ship*>& result, const sea_object* o);
	virtual void visible_submarines(list<submarine*>& result, const sea_object* o);
	virtual void visible_airplanes(list<airplane*>& result, const sea_object* o);
	virtual void visible_torpedoes(list<torpedo*>& result, const sea_object* o);
	virtual void visible_depth_charges(list<depth_charge*>& result, const sea_object* o);
	virtual void visible_gun_shells(list<gun_shell*>& result, const sea_object* o);
	virtual void visible_particles ( list<particle*>& result, const sea_object* o );
	// computes visible ships, submarines (surfaced) and airplanes
	virtual vector<sea_object*> visible_surface_objects(const sea_object* o);

	virtual void sonar_ships ( list<ship*>& result, const sea_object* o );
	virtual void sonar_submarines ( list<submarine*>& result, const sea_object* o );
	virtual ship* sonar_acoustical_torpedo_target ( const torpedo* o );
	
	// list<*> radardetected_ships(...);	// later!

	void convoy_positions(list<vector2>& result) const;	// fixme
	
//	bool can_see(const sea_object* watcher, const submarine* sub) const;	fixme what's that?

	// create new objects
	void spawn_ship(ship* s);
	void spawn_submarine(submarine* u);
	void spawn_airplane(airplane* a);
	void spawn_torpedo(torpedo* t);
	void spawn_gun_shell(gun_shell* s, const double &calibre);
	void spawn_depth_charge(depth_charge* dc);
	void spawn_convoy(convoy* cv);
	void spawn_particle(particle* pt);

	// simulation events
//fixme: send messages about them to ui (remove sys-console printing in torpedo.cpp etc)
	void dc_explosion(const depth_charge& dc);	// depth charge exploding
	bool gs_impact(const vector3& pos, const double &damage);	// gun shell impact
	void torp_explode(const vector3& pos);	// torpedo explosion/impact
	void ship_sunk( const ship* s );	// a ship sinks

	// simulation actions, fixme send something over net for them
	virtual void ping_ASDIC(list<vector3>& contacts, sea_object* d,
		const bool& move_sensor, const angle& dir = angle ( 0.0f ) );

	// various functions (fixme: sort, cleanup)
	void register_job(job* j);	// insert/remove job in job list
	void unregister_job(job* j);
	const list<ping>& get_pings(void) const { return pings; };

	template<class C>
	ship* check_unit_list ( torpedo* t, list<C>& unit_list );

	// fixme why is this not const? if it changes game, it must be send over network, and
	// then it can't be a function!
	bool check_torpedo_hit(torpedo* t, bool runlengthfailure, bool failure);

	// dito, see check_torpedo_hit-comment
	sea_object* contact_in_direction(const sea_object* o, const angle& direction);
	ship* ship_in_direction_from_pos(const sea_object* o, const angle& direction);
	submarine* sub_in_direction_from_pos(const sea_object* o, const angle& direction);

	bool is_collision(const sea_object* s1, const sea_object* s2) const;
	bool is_collision(const sea_object* s, const vector2& pos) const;

	double water_depth(const vector2& pos) const;
	
	// main game loop
	run_state exec(void);
	
	// sun/moon and light color/brightness
	double compute_light_brightness(const vector3& viewpos) const;	// depends on sun/moon
	color compute_light_color(const vector3& viewpos) const;	// depends on sun/moon
	vector3 compute_sun_pos(const vector3& viewpos) const;
	vector3 compute_moon_pos(const vector3& viewpos) const;

	// Translate pointers to numbers and vice versa. Used for load/save
	void write(ostream& out, const ship* s) const;
	void write(ostream& out, const submarine* s) const;
	void write(ostream& out, const airplane* s) const;
	void write(ostream& out, const torpedo* s) const;
	void write(ostream& out, const depth_charge* s) const;
	void write(ostream& out, const gun_shell* s) const;
	void write(ostream& out, const convoy* s) const;
	void write(ostream& out, const sea_object* s) const;
	ship* read_ship(istream& in) const;
	submarine* read_submarine(istream& in) const;
	airplane* read_airplane(istream& in) const;
	torpedo* read_torpedo(istream& in) const;
	depth_charge* read_depth_charge(istream& in) const;
	gun_shell* read_gun_shell(istream& in) const;
	convoy* read_convoy(istream& in) const;
	sea_object* read_sea_object(istream& in) const;
	// particles don't heir from sea_object and are special
	void write(ostream& out, const particle* p) const;
	particle* read_particle(istream& in) const;
};

#endif
