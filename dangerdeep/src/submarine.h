/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2020  Thorsten Jordan, Luis Barrancos and others.

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

// submarines
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef SUBMARINE_H
#define SUBMARINE_H

#include "ship.h"
#include "tdc.h"
#include "sonar.h"
#include "sonar_operator.h"
#include "torpedo.h"
#include <vector>

class game;
//class torpedo;

///\brief Represents a submarine with all attributes like torpedo storage and handling, depth rudder control etc.
/** Submarine attributes are defined via specification XML file.
 */
class submarine : public ship
{
 private:
	submarine() = delete;
	submarine& operator= (const submarine& other) = delete;
	submarine(const submarine& other) = delete;

 public:

	struct stored_torpedo
	{
		enum st_status { st_empty, st_reloading, st_unloading, st_loaded };
		// a torpedo transfer must not copy this structure!
		std::string specfilename; ///< torpedo type, to be copied on transfer
		torpedo::setup setup;	///< tube setup, don't copy
		double temperature{15.0};	///< current torpedo temperatue - fixme: as attribute of class torpedo?
		st_status status{st_empty};	///< 0 empty 1 reloading 2 unloading 3 loaded
		unsigned associated{0};	///< reloading from/unloading to
		double remaining_time{0};	///< remaining time until work is finished
		angle addleadangle;	///< additional lead angle (only per tube) fixme: replace by lead angle reported from TDC

		stored_torpedo();
		stored_torpedo(game& gm, std::string  type);
		void load(game& gm, const xml_elem& parent);
		void save(xml_elem& parent) const;
	};

	enum hearing_device_type {
		hearing_device_KDB,
		hearing_device_GHG,
		hearing_device_BG
	};

	enum dive_states { // numbers are stored in savegame, don't change later!
		dive_state_surfaced = 0, // boat is at surface
		dive_state_preparing_for_dive = 1, // prepare to dive, gun is unmanned etc.
		dive_state_diving = 2, // boat is diving, hatches closed
		dive_state_crashdive = 3, // boat is on crash dive, diving fast down to alarm depth
		dive_state_preparing_for_crashdive = 4, // prepare to crash dive, gun is unmanned etc.
		//dive_state_running_silent,
		//dive_state_snorkeling,
		//surface torpedo transfer?
	};

protected:
	double max_depth;		// created with some randomness after spec file, must get stored!

	/// variables for dive-helmsman simulation
	double dive_to;
	bool permanent_dive;
	dive_states dive_state;

	/// bow and stern depth rudders
	generic_rudder bow_depth_rudder, stern_depth_rudder;

	/// additional mass of submarine given by filled tanks (is added to "mass")
	double mass_flooded_tanks; // recomputed from tanks-vector every simulate() round
	double ballast_tank_capacity; // computed from tanks-vector

	double max_submerged_speed;	// read from spec file

	// stored torpedoes (including tubes)
	// special functions calculate indices for bow/stern tubes etc., see below
	std::vector<stored_torpedo> torpedoes;
	unsigned number_of_tubes_at[6];	// read from spec file
	unsigned torp_transfer_times[5];// read from spec file

	float scope_raise_level;	///< current level that scope is raised (0...1)
	float scope_raise_to_level;	///< level that scope should be raised to (0...1)
	double periscope_depth;		// read from spec file
	bool electric_engine;		// true when electric engine is used.
	bool hassnorkel;		// fixme: replace by (parts[snorkel] != unused)
	double snorkel_depth;		// read from spec file
	double alarm_depth;		// read from spec file
	bool snorkelup;
//	float sonar_cross_section_factor;

	// Charge level of battery: 0 = empty, 1 = fully charged
	double battery_level;
	double battery_value_a;		// read from spec file
	double battery_value_t;		// read from spec file
	double battery_recharge_value_a;// read from spec file
	double battery_recharge_value_t;// read from spec file
	unsigned battery_capacity;	// read from spec file

	// torpedo management view image name for side/top view
	std::string torpedomanage_sidetopimg;	// read from spec file

	// the hearing device type
	hearing_device_type hearing_device;	// read from spec file and time, should be saved later
    
	std::vector<part> parts;	// read from data/spec file, fixme do that!

	// fixme: add: double temperature;	// overall temperature in submarine. used for torpedo preheating computation

	int find_stored_torpedo(bool usebow);	// returns index or -1 if none

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

	void calculate_fuel_factor ( double delta_time ) override;
	
	void gun_manning_changed(bool is_gun_manned) override;

	/// used to simulate diving
	void compute_force_and_torque(vector3& F, vector3& T) const override;

	/// open ballast tank valves
	void flood_ballast_tanks();
	/// push air to all ballast tanks
	///@param amount_cbm - amount of air to push in m^3
	///@returns rest air that did not fit to any tank in m^3
	double push_air_to_ballast_tanks(double amount_cbm);

	// in-hull temperature, depends on weather/latitude etc.
	// torpedo-temperature is set from this value for stored torpedoes (not in tubes)
	//double temperature;//maybe store for each torpedo and not here...

	tdc TDC;

	// sonar man. its contents must get saved... fixme
	sonar_operator sonarman;

	// bridge data
	std::string bridge_model_name;
	vector3 bridge_camera_pos;
	vector3 bridge_uzo_pos;
	vector3 bridge_freeview_pos;

	// tanks/diving
	class tank
	{
	public:
		enum types { trim, ballast };
		tank(xml_elem e);
		void load(const xml_elem& parent);
		void save(xml_elem& parent) const;
		void simulate(double delta_time);
		void set_flood_valve(bool flood = true);
		/// put some air into the tank
		///@note handle pressure later
		///@param amount_cbm - air to be pushed into the tank in m^3
		///@returns rest air that did not fit to tank in m^3
		double push_air_inside(double amount_cbm);

		types get_type() const { return type; }
		double get_volume() const { return volume; }
		const vector3& get_pos() const { return pos; }
		double get_fill() const { return fill; }

	protected:
		// values read from spec file, constant at runtime
		types type;
		double volume; // m^3
		double fillrate; // m^3/s
		vector3 pos;

		// runtime-changable, stored in savegame
		double fill; // m^3, maybe later kg? water density isn't handled yet!
		bool flood_valve_open; // can water enter tank?

	private:
		tank() = delete;
	};

	std::vector<tank> tanks;

	int diveplane_1_id, diveplane_2_id; // for display()

public:
	// there were more types, I, X (mine layer), XIV (milk cow), VIIf, (and VIId)
	// and some experimental types. (VIIc42, XVIIa/b)
	// there were two IXd1 boats similar to type d2, but with different
	// engines.
/*
	enum types {
		typeIIa=256, typeIIb, typeIIc, typeIId,
		typeVIIa, typeVIIb, typeVIIc, typeVIIc41,
		typeIX, typeIXb, typeIXc, typeIXc40, typeIXd2,
		typeXXI,
		typeXXIII };
*/

	// create empty object from specification xml file
	submarine(game& gm_, const xml_elem& parent);

	void load(const xml_elem& parent) override;
	void save(xml_elem& parent) const override;
	
	void simulate(double delta_time) override;

	void set_target(sea_object* s) override;

	// get bridge data
	std::string get_bridge_filename() { return bridge_model_name; }
	vector3 get_camera_position() { return bridge_camera_pos; }
	vector3 get_uzo_position() { return bridge_uzo_pos; }
	vector3 get_freeview_position() { return bridge_freeview_pos; }

	// fill available tubes with common types depending on time period (used for custom missions)
	virtual void init_fill_torpedo_tubes(const class date& d);

	const std::vector<stored_torpedo>& get_torpedoes() const { return torpedoes; }

	// give number from 0-5 (bow tubes first)
	bool is_tube_ready(unsigned nr) const;

	// get number of tubes / stored reserve torpedoes
	virtual unsigned get_nr_of_bow_tubes() const { return number_of_tubes_at[0]; }
	virtual unsigned get_nr_of_stern_tubes() const { return number_of_tubes_at[1]; }
	virtual unsigned get_nr_of_bow_reserve() const { return number_of_tubes_at[2]; }
	virtual unsigned get_nr_of_stern_reserve() const { return number_of_tubes_at[3]; }
	virtual unsigned get_nr_of_bow_deckreserve() const { return number_of_tubes_at[4]; }
	virtual unsigned get_nr_of_stern_deckreserve() const { return number_of_tubes_at[5]; }

	// get first index of storage and first index after it (computed with functions above)
	std::pair<unsigned, unsigned> get_bow_tube_indices() const;
	std::pair<unsigned, unsigned> get_stern_tube_indices() const;
	std::pair<unsigned, unsigned> get_bow_reserve_indices() const;
	std::pair<unsigned, unsigned> get_stern_reserve_indices() const;
	std::pair<unsigned, unsigned> get_bow_deckreserve_indices() const;
	std::pair<unsigned, unsigned> get_stern_deckreserve_indices() const;
	unsigned get_location_by_tubenr(unsigned tn) const; // returns 1-6 as location number, 0 if not supported

	// The simulation of acceleration when switching between electro and diesel
	// engines is done via engine simulation. So the boat "brakes" until
	// it reaches its submerged speed. This is not correct, because speed decreases
	// too fast, but it should be satisfying for now. fixme
	double get_max_speed() const override;

	// compute probabilty that sub can be seen (determined by depth, speed,
	// state: periscope state, snorkeling etc., shape)
	float surface_visibility(const vector2& watcher) const override;
	virtual float sonar_visibility ( const vector2& watcher ) const;
	double get_noise_factor () const override;
	// return pointer to torpedo in tube or NULL if tube is empty
	virtual stored_torpedo& get_torp_in_tube(unsigned tubenr);
	virtual const stored_torpedo& get_torp_in_tube(unsigned tubenr) const;
	virtual bool is_scope_up() const { return scope_raise_level >= 0.8f; }
	virtual float get_scope_raise_level() const { return scope_raise_level; }
	virtual double get_periscope_depth() const { return periscope_depth; }
	virtual bool is_submerged () const { return dive_state == dive_state_diving; }
	virtual double get_max_depth () const { return max_depth; }
	virtual bool is_electric_engine () const { return (electric_engine == true); }
	virtual bool is_snorkel_up () const { return ( snorkelup == true ); }
	virtual bool has_snorkel () const { return ( hassnorkel == true ); }
	virtual double get_snorkel_depth () const { return snorkel_depth; }
	virtual double get_alarm_depth () const { return alarm_depth; }
	virtual double get_battery_level () const { return battery_level; }
	virtual const std::vector<part>& get_damage_status() const { return parts; }

	// get/compute torpedo transfer time and helper functions (uses functions below to compute)
	virtual double get_torp_transfer_time(unsigned from, unsigned to) const;
	virtual double get_bow_reload_time() const { return torp_transfer_times[0]; }
	virtual double get_stern_reload_time() const { return torp_transfer_times[1]; }
	virtual double get_bow_deck_reload_time() const { return torp_transfer_times[2]; }
	virtual double get_stern_deck_reload_time() const { return torp_transfer_times[3]; }
	virtual double get_bow_stern_deck_transfer_time() const { return torp_transfer_times[4]; }
	
	// damage is added if dc damages sub.
	virtual void depth_charge_explosion(const class depth_charge& dc);
    
	// command interface for subs
	virtual void scope_up() { scope_to_level(1.0f); }
	virtual void scope_down() { scope_to_level(0.0f); }
	virtual void scope_to_level(float f);
	virtual bool set_snorkel_up ( bool snorkel_up );	// fixme get rid of this
	virtual void snorkel_up() { set_snorkel_up(true); }
	virtual void snorkel_down() { set_snorkel_up(false); }
	virtual void set_planes_to(double amount); // -2...2 // fixme: functions for both dive planes needed?
	virtual void crash_dive();
	virtual void dive_to_depth(unsigned meters);
	virtual void depth_steering_logic();
	virtual void ballast_tank_control_logic(double delta_time);
	virtual void transfer_torpedo(unsigned from, unsigned to);

	// give tubenr -1 for any loaded tube, or else 0-5, returns true on success
	virtual bool launch_torpedo(int tubenr, sea_object* target);
	// end of command interface

	virtual bool has_deck_gun() { return has_guns(); }
	
	virtual tdc& get_tdc() { return TDC; }
	virtual const tdc& get_tdc() const { return TDC; }
	//virtual sonar_operator& get_sonarman() { return sonarman; } // not needed. dangerous, sonarman could get manipulated.
	virtual const sonar_operator& get_sonarman() const { return sonarman; }

	virtual double get_bow_rudder() const { return bow_depth_rudder.angle; }
	virtual double get_stern_rudder() const { return stern_depth_rudder.angle; }
	virtual void set_bow_depth_rudder(double to){ bow_depth_rudder.set_to(to); permanent_dive = true; }
	virtual void set_stern_depth_rudder(double to){ stern_depth_rudder.set_to(to); permanent_dive = true; }

	const std::string& get_torpedomanage_img_name() const { return torpedomanage_sidetopimg; }

	virtual hearing_device_type get_hearing_device_type() const { return hearing_device; }
};

#endif
