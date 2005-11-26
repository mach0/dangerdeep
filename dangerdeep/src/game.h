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
#include "ptrset.h"
#include "xml.h"

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
		ping(const vector2& p, angle d, double t, double range,
			const angle& ping_angle_ ) :
			pos(p), dir(d), time(t), range ( range ), ping_angle ( ping_angle_ )
			{}
		~ping() {}
		ping(const xml_elem& parent);
		void save(xml_elem& parent) const;
	};

	struct sink_record {
		date dat;
		std::string descr;	// fixme: store type, use a static ship function to retrieve a matching description
		std::string mdlname;	// model file name string
		unsigned tons;
		sink_record(date d, const std::string& s, const std::string& m, unsigned t) : dat(d), descr(s), mdlname(m), tons(t) {}
		sink_record(const xml_elem& parent);
		void save(xml_elem& parent) const;
	};
	
	struct job {
		job() {}
		virtual void run() = 0;
		virtual double get_period() const = 0;
		virtual ~job() {}
	};

	// in which state is the game (return value of exec())
	// normal mode (running), or stop on next cycle (reason given by value)
	enum run_state { running, player_killed, mission_complete, contact_lost };
	
	user_interface* get_ui() { return ui; }
	
	// time between records of trail positions
	static const double TRAIL_TIME;

protected:
	// begin [SAVE]
	ptrset<ship> ships;
	ptrset<submarine> submarines;
	ptrset<airplane> airplanes;
	ptrset<torpedo> torpedoes;
	ptrset<depth_charge> depth_charges;
	ptrset<gun_shell> gun_shells;
	ptrset<convoy> convoys;
	ptrset<particle> particles;
	// end [SAVE]
	run_state my_run_state;
	bool stopexec;	// if this is true, execution stops and the menu is displayed
	
	std::list<pair<double, job*> > jobs;	// generated by interface construction, no gameplay data
	
	// the player and matching ui (note that playing is not limited to submarines!)
	sea_object* player;	// [SAVE]

	// in theory, the game should know nothing about the user interface! fixme
	// what is ui needed for: display messages, play sounds, add to captains
	// log, add sunken ship record, get time scale*, display ui*
	// if game is paused*, *=should move from game::exec to subsim::exec_game
	// instead of calling ui we should store events and let the ui create messages
	// or sounds from them on its own. record and logbook should be part of game.
	user_interface* ui;	// can be zero, is set from game's owner

	std::list<sink_record> sunken_ships;	// [SAVE]

	logbook players_logbook;	// [SAVE]
	
	double time;	// global time (in seconds since 1.1.1939, 0:0 hrs) (universal time!) [SAVE]
	double last_trail_time;	// for position trail recording	[SAVE]
	
	enum weathers { sunny, clouded, raining, storm };//fixme
	double max_view_dist;	// maximum visibility according to weather conditions, fixme recomputed or save?
	
	std::list<ping> pings;	// [SAVE]
	
	// network game type (0 = single player, 1 = server, 2 = client)
	unsigned networktype;	// [SAVE] later!
	// the connection to the server (zero if this is the server)
	network_connection* servercon;	// [SAVE] later!
	// the connections to the clients (at least one if this is the server, else empty)
	std::vector<network_connection*> clientcons;	// [SAVE] later!

	game();	
	game& operator= (const game& other);
	game(const game& other);

	unsigned listsizes(unsigned n) const;	// counts # of list elemens over n lists above

public:
	// create new custom mission
	// expects: size small,medium,large, escort size none,small,medium,large,
	// time of day [0,4) night,dawn,day,dusk
	game(const std::string& subtype, unsigned cvsize, unsigned cvesc, unsigned timeofday,
		unsigned timeperiod, unsigned nr_of_players = 1);

	// create from mission file or savegame (xml file)
	game(const std::string& filename);

	virtual ~game();

	virtual void save(const std::string& savefilename, const std::string& description) const;
	static std::string read_description_of_savegame(const std::string& filename);

	void stop();

	void compute_max_view_dist();	// fixme - public?
	void simulate(double delta_t);

	const std::list<sink_record>& get_sunken_ships() const { return sunken_ships; };
	const logbook& get_players_logbook() const { return players_logbook; }
	void add_logbook_entry(const std::string& s);
	double get_time() const { return time; };
	double get_max_view_distance() const { return max_view_dist; }
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
	
	sea_object* get_player() const { return player; }
	void set_user_interface(user_interface* ui_) { ui = ui_; }

	double get_last_trail_record_time() const { return last_trail_time; }

	// compute visibility data
	virtual std::vector<ship*> visible_ships(const sea_object* o) const;
	virtual std::vector<submarine*> visible_submarines(const sea_object* o) const;
	virtual std::vector<airplane*> visible_airplanes(const sea_object* o) const;
	virtual std::vector<torpedo*> visible_torpedoes(const sea_object* o) const;
	virtual std::vector<depth_charge*> visible_depth_charges(const sea_object* o) const;
	virtual std::vector<gun_shell*> visible_gun_shells(const sea_object* o) const;
	virtual std::vector<particle*> visible_particles (const sea_object* o ) const;
	// computes visible ships, submarines (surfaced) and airplanes
	virtual std::vector<sea_object*> visible_surface_objects(const sea_object* o) const;

	virtual std::vector<ship*> sonar_ships (const sea_object* o ) const;
	virtual std::vector<submarine*> sonar_submarines (const sea_object* o ) const;
	virtual ship* sonar_acoustical_torpedo_target ( const torpedo* o ) const;
	
	// std::list<*> radardetected_ships(...);	// later!
	virtual std::vector<submarine*> radar_submarines(const sea_object* o) const;
	virtual std::vector<ship*> radar_ships(const sea_object* o) const;

	std::vector<vector2> convoy_positions() const;	// fixme
	
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
	bool gs_impact(const gun_shell *gs);	// gun shell impact
	void torp_explode(const torpedo *t);	// torpedo explosion/impact
	void ship_sunk( const ship* s );	// a ship sinks

	// simulation actions, fixme send something over net for them, fixme : maybe vector not list?
	virtual void ping_ASDIC(std::list<vector3>& contacts, sea_object* d,
		const bool& move_sensor, const angle& dir = angle ( 0.0f ) );

	// various functions (fixme: sort, cleanup)
	void register_job(job* j);	// insert/remove job in job list
	void unregister_job(job* j);
	const std::list<ping>& get_pings() const { return pings; };	// fixme: maybe vector not list

	template<class C> ship* check_units ( torpedo* t, const ptrset<C>& units );

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
	run_state exec();
	
	// sun/moon and light color/brightness
	double compute_light_brightness(const vector3& viewpos) const;	// depends on sun/moon
	color compute_light_color(const vector3& viewpos) const;	// depends on sun/moon
	vector3 compute_sun_pos(const vector3& viewpos) const;
	vector3 compute_moon_pos(const vector3& viewpos) const;

	// Translate pointers to numbers and vice versa. Used for load/save
	sea_object* load_ptr(unsigned nr) const;
	ship* load_ship_ptr(unsigned nr) const;
	unsigned save_ptr(const sea_object* s) const;


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
