// submarines
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUBMARINE_H
#define SUBMARINE_H

#include "ship.h"
#include "torpedo.h"
#include "parser.h"
#include <vector>

#define SUBMARINE_SUBMERGED_DEPTH 2.0f // meters

#define CROSS_SECTION_ACT_SONAR_LSUB     0.20f        // Large Submarines (?)
#define CROSS_SECTION_ACT_SONAR_NSUB     0.15f        // Normal Submarines (?)
#define CROSS_SECTION_ACT_SONAR_MSUB_AC  0.05f        // Midget Submarines (?), Aircrafts
#define CROSS_SECTION_ACT_SONAR_TORPWB   0.01f        // Torpedo

class submarine : public ship
{
public:
	struct stored_torpedo {
		enum st_status { st_empty, st_reloading, st_unloading, st_loaded };
		unsigned type;
		st_status status;	// 0 empty 1 reloading 2 unloading 3 loaded
		unsigned associated;	// reloading from/unloading to
		double remaining_time;	// remaining time until work is finished
		stored_torpedo() : type(0), status(st_empty), associated(0), remaining_time(0) {}
		stored_torpedo(unsigned t) : type(t), status(st_loaded), associated(0), remaining_time(0) {}
	};

	// submarine parts and their damages
	// fixme: replace german names by correct translations
	// lenzpumpe (water pump???)
	// bilge???
	// kitchen - kombuese?
	// balance tank - trimmtank?
	enum damageable_parts {
		// common parts
		rudder,
		screws,
		screw_shaft,
		stern_dive_planes,
		stern_water_pump,	//?
		stern_pressure_hull,
		stern_hatch,
		electric_engines,
		air_compressor,
		machine_water_pump,	//?
		machine_pressure_hull,
		aft_battery,
		diesel_engines,
		kitchen_hatch,		//?	// there was only one hatch at the stern!? fixme
		balance_tank_valves,	//?
		forward_battery,
		periscope,	// fixme: there were two...
		central_pressure_hull,
		bilge_water_pump,	//?
		conning_tower_hatch,
		listening_device,
		radio_device,
		inner_bow_tubes,
		outer_bow_tubes,
		bow_water_pump,		//?
		bow_hatch,
		bow_pressure_hull,
		bow_dive_planes,
		aa_gun,
		ammunition_depot,
		outer_fuel_tanks,

		// parts specific to sub types
		outer_stern_tubes,
		inner_stern_tubes,
		snorkel,//fixme conflicts with bool snorkel;
		deck_gun,
		radio_detection_device,
		// radar,	// maybe
		
		nr_of_damageable_parts	// trick to count enum entries
	};
	
	enum damage_status {
		unused,
		none,
		light,
		medium,
		heavy,
		critical,
		wrecked
	};
	
protected:
	double dive_speed, dive_acceleration, max_dive_speed, max_depth, dive_to;
	bool permanent_dive;
	double max_submerged_speed;

	// stored torpedoes (including tubes)
	// special functions calculate indices for bow/stern tubes etc., see below
	vector<stored_torpedo> torpedoes;

	bool scopeup;	// fixme: maybe simulate time for moving scope up/down
	double periscope_depth;
	bool electric_engine; // true when electric engine is used.
	bool hassnorkel;	// fixme: replace by (damageable_parts[snorkel] != unused)
	double snorkel_depth;
	bool snorkel_up;
	float sonar_cross_section_factor;

	// Charge level of battery: 0 = empty, 1 = fully charged
	double battery_level;
	double battery_value_a;
	double battery_value_t;
	double battery_recharge_value_a;
	double battery_recharge_value_t;
    
	vector<damage_status> damageable_parts;

	submarine();
	submarine& operator= (const submarine& other);
	submarine(const submarine& other);

	int find_stored_torpedo(bool usebow);	// returns index or -1 if none

	bool parse_attribute(parser& p);	// returns false if invalid token found
	/**
		This method calculates the battery consumption rate. This value is needed
		for the simulate function to reduce the battery_level value. An
		exponential is used as a model basing on some battery consumption values.
		@return double hourly percentage battery consumption value
	*/
	virtual double get_battery_consumption_rate () const
	{ return battery_value_a * ( exp ( get_throttle_speed () / battery_value_t ) - 1.0f ); }
	/**
		This method method calculates the battery recharge rate.
		@return battery recharge rate
	*/
	virtual double get_battery_recharge_rate () const
	{ return ( 1.0f - ( battery_recharge_value_a *
    	exp ( - get_throttle_speed () / battery_recharge_value_t ) ) ); }

	virtual void calculate_fuel_factor ( double delta_time );

public:
	// fixme type II has some subtypes.
	enum types { typeII, typeVII, typeVIIb, typeVIIc, typeVIIc41,
		typeIX, typeIXb, typeIXc, typeIXc40, typeIXd2, typeXXI, typeXXIII };
	virtual ~submarine() {}
	static submarine* create(types type_);
	static submarine* create(parser& p);
	virtual void simulate(class game& gm, double delta_time);

	const vector<stored_torpedo>& get_torpedoes(void) const { return torpedoes; }
	// get first index of storage and first index after it.
	virtual pair<unsigned, unsigned> get_bow_tube_indices(void) const = 0;
	virtual pair<unsigned, unsigned> get_stern_tube_indices(void) const = 0;
	virtual pair<unsigned, unsigned> get_bow_storage_indices(void) const = 0;
	virtual pair<unsigned, unsigned> get_stern_storage_indices(void) const = 0;
	virtual pair<unsigned, unsigned> get_bow_top_storage_indices(void) const = 0;
	virtual pair<unsigned, unsigned> get_stern_top_storage_indices(void) const = 0;

	// The simulation of acceleration when switching between electro and diesel
	// engines is done via engine simulation. So the boat "brakes" until
	// it reaches its submerged speed. This is not correct, because speed decreases
	// too fast, but it should be satisfying for now. fixme
	virtual double get_max_speed(void) const;

	// compute probabilty that sub can be seen (determined by depth, speed,
	// state: periscope state, snorkeling etc., shape)
	virtual float surface_visibility(const vector2& watcher) const;
	virtual float sonar_visibility ( const vector2& watcher ) const;
	virtual double get_noise_factor () const;

	virtual bool is_scope_up(void) const { return ( scopeup == true ); }
	virtual double get_periscope_depth() const { return periscope_depth; }
	virtual bool is_submerged () const { return get_depth() > SUBMARINE_SUBMERGED_DEPTH; }
	virtual double get_max_depth () const { return max_depth; }
	virtual bool is_electric_engine (void) const { return (electric_engine == true); }
	virtual bool is_snorkel_up () const { return ( snorkel_up == true ); }
	virtual bool has_snorkel () const { return ( hassnorkel == true ); }
	virtual double get_snorkel_depth () const { return snorkel_depth; }
	virtual double get_battery_level () const { return battery_level; }
	virtual const vector<damage_status>& get_damage_status(void) const { return damageable_parts; }

	// give relative direction and amount between 0 and 1.
	virtual void add_damage(const vector3& fromwhere, float amount) { /*fixme*/ };
    
	// command interface for subs
	virtual void scope_up(void) { scopeup = true; };	// fixme
	virtual void scope_down(void) { scopeup = false; };
	virtual bool set_snorkel_up ( bool snorkel_up );
	virtual void planes_up(double amount);
	virtual void planes_down(double amount);
	virtual void planes_middle(void);
	virtual void dive_to_depth(unsigned meters);
	// give tubenr -1 for any loaded tube, or else 0-5
	virtual bool fire_torpedo(class game& gm, int tubenr, sea_object* target,
		const angle& manual_lead_angle);

	// fixme: time that is needed depends on sub type and how many torpedoes
	// are already in transfer. So this argument is nonesense. fixme
	// returns true if transfer was initiated.
	// fixme: make virtual?
	bool transfer_torpedo(unsigned from, unsigned to, double timeneeded = 120);
};

#endif
