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

// ships
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#pragma once

#include "bv_tree.h"
#include "sea_object.h"

#include <map>

class game;

///\brief Base class for all ships and ship-like objects: ships, submarines,
/// torpedoes.
/** Handles steering and rudder simulation,
    damage control and other things.
    Ship attributes are defined via specification XML file. */
class ship : public sea_object
{
  public:
    ship() = default; // needed to store it in maps, don't use manually!

    // give negative values for fixed speeds, positive values for knots.
    enum throttle_status
    {
        reversefull = -9,
        reversehalf = -8,
        reverse     = -7 /* reverse slow */,
        aheadlisten = -6,
        aheadsonar  = -5,
        aheadslow   = -4,
        aheadhalf   = -3,
        aheadfull   = -2,
        aheadflank  = -1,
        stop        = 0
    };

    enum rudder_status
    {
        rudderfullleft  = -2,
        rudderleft      = -1,
        ruddermidships  = 0,
        rudderright     = 1,
        rudderfullright = 2
    };

    enum gun_status
    {
        TARGET_OUT_OF_RANGE     = -1, // used by deck gun
        NO_AMMO_REMAINING       = 0,  // used by deck gun
        GUN_FIRED               = 1,
        RELOADING               = 2,
        GUN_NOT_MANNED          = 3,
        GUN_TARGET_IN_BLINDSPOT = 4,
        NO_GUNS                 = 5
    };

    enum head_to_param
    {
        HEAD_TO_UNDEFINED         = 0,
        HEAD_TO_LEFT              = 0x01,
        HEAD_TO_RIGHT             = 0x02,
        HEAD_TO_FORCE_DIRECTION   = 0x04,
        HEAD_TO_ALLOW_HARD_RUDDER = 0x08
    };

    // maximum trail record length
    static const unsigned TRAIL_LENGTH = 60;

    // all data for a previous position
    struct prev_pos
    {
        vector2 pos;  // (center) pos of ship
        vector2 dir;  // direction (heading) of ship
        double time;  // absolute time when position was recorded
        double speed; // speed of ship when position was recorded
        prev_pos(const vector2& p, const vector2& d, double t, double s) :
            pos(p), dir(d), time(t), speed(s)
        {
        }
        // add xml load/save functions here, fixme
    };

  protected:
    unsigned tonnage; // in BRT, created after values from spec file, must get
                      // stored!

    int throttle; // if < 0: throttle_state, if > 0: knots

    /// rudder related stuff grouped as class
    struct generic_rudder
    {
        // read from spec file, run-time constants
        vector3 pos;           ///< 3d pos of rudder
        int axis;              ///< axis (0-z, 1-x)
        double max_angle;      ///< max. angle of rudder (+-)
        double area;           ///< area of rudder in m^2
        double max_turn_speed; ///< max turn speed in angles/sec

        double
            angle; ///< current rudder angle in degrees, do not use class angle
                   ///< here, we need explicit positive and negative values.
        double to_angle; ///< angle that rudder should move to

        /// To make it storeable in containers, don't use
        generic_rudder() = default;
        generic_rudder(
            const vector3& p,
            int a,
            double ma,
            double ar,
            double mts) :
            pos(p),
            axis(a), max_angle(ma), area(ar), max_turn_speed(mts), angle(0),
            to_angle(0)
        {
        }
        void simulate(double delta_time);
        void load(const xml_elem& parent);
        void save(xml_elem& parent) const;
        void set_to(double p) { to_angle = max_angle * p; } ///< -1 ... 1
        void midships() { to_angle = 0; }
        [[nodiscard]] double deflect_factor() const; ///< sin(angle)
        [[nodiscard]] double bypass_factor() const;  ///< cos(angle)
        /** compute force and torque generated by rudder
            @param F - force returned value
            @param T - torque returned value
            @param parent_local_velocity - local velocity of parent
            @param water_density - density of water
            @param flow_force - force that by-passes the rudder, is modified by
           angle
            @returns modified flow force
        */
        double compute_force_and_torque(
            vector3& F,
            vector3& T,
            const vector3& parent_local_velocity,
            const double& water_density,
            const double& flow_force = 0) const;
    };

    generic_rudder rudder;

    head_to_param head_to_fixed;
    angle head_to;

    double turn_rate; // in angle/time (at max. speed/throttle), read from spec
                      // file
    // fixme: value seems to be angle/meter, meaning angle change per m forward
    // motion...

    double max_accel_forward; // read from spec file. can get computed from
                              // engine_torque, screw diameter and ship's mass.
    double max_speed_forward; // read from spec file.
    double max_speed_reverse; // read from spec file.

    // fixme: replace by finer model: -> damage editor!
    damage_status stern_damage, midship_damage, bow_damage;

    // Fuel percentage: 0 = empty, 1 = full.
    double fuel_level;
    double fuel_value_a;    // read from spec file
    double fuel_value_t;    // read from spec file
    unsigned fuel_capacity; // read from spec file

    // sonar / underwater sound specific constants, read from spec file
    noise_signature noise_sign;

    std::list<prev_pos> previous_positions;

    shipclass myclass; // read from spec file, e.g. warship/merchant/escort/...

    //
    // sinking simulation
    //
    // for each voxel the mass that has flooded into (max. mass is computed from
    // voxel data. [SAVE]
    std::vector<float> flooded_mass;
    // speed by that water floods into the ship, in kg per second
    double flooding_speed;
    // maximum of additional mass because of flooding, computed from spec/mdl
    // file can be volume * density of water.
    double max_flooded_mass;

    void compute_force_and_torque(vector3& F, vector3& T, game& gm)
        const override; // drag must be already included!

    /// implementation of the steering logic: helmsman simulation, or simpler
    /// model for torpedoes.
    virtual void steering_logic();
    /// return the acceleration factor for computing torque (depends on rudder
    /// area etc.)
    [[nodiscard]] virtual double get_turn_accel_factor() const
    {
        return 20000.0;
    }
    /// return drag coefficient for turn drag
    [[nodiscard]] virtual double get_turn_drag_coeff() const { return 1.0; }
    /// return the side area for drag computation multiplied by drag
    /// coefficient.
    [[nodiscard]] virtual double get_turn_drag_area() const;

    /**
        This method calculates the hourly fuel consumption. An
        exponential is used as a model basing on some fuel consumption values.
        @return double hourly percentage fuel consumption value
    */
    [[nodiscard]] virtual double get_fuel_consumption_rate() const
    {
        return fuel_value_a * (exp(get_throttle_speed() / fuel_value_t) - 1.0f);
    }

    /**
        This method is called by the simulate method to recalculate the
        actual fuel status.
        @param delta_time time advance since last call
    */
    virtual void calculate_fuel_factor(double delta_time);

    // smoke, list of smoke generators. Give type and relative position for each
    // generator
    std::list<std::pair<unsigned, vector3>> smoke;

    // pointer to fire particle (0 means ship is not burning)
    particle* myfire;

    [[nodiscard]] virtual bool causes_spray() const { return true; }

    // some experience values of the crews to fire a grenade with right angle at
    // any target. This depends on canon type (shot speed, min/max angles etc.)
    // so we need several ai classes later.
    static std::map<double, std::map<double, double>> dist_angle_relation;
    static void fill_dist_angle_relation_map(const double initial_velocity);

    // deck gun
    struct gun_barrel
    {
        double load_time_remaining;
        angle last_elevation;
        angle last_azimuth;

        gun_barrel()
        {
            load_time_remaining = 0.0;
            last_elevation      = 0;
            last_azimuth        = 0;
        }
    };

    struct gun_turret
    {
        int num_shells_remaining;
        int shell_capacity;
        double initial_velocity;
        int max_declination;
        int max_inclination;
        double time_to_man;
        double time_to_unman;
        bool is_gun_manned;
        double manning_time;
        double shell_damage;
        int start_of_exclusion_radius;
        int end_of_exclusion_radius;
        double calibre;

        std::list<struct gun_barrel> gun_barrels;

        gun_turret()
        {
            num_shells_remaining      = 0;
            shell_capacity            = 0;
            initial_velocity          = 0.0;
            max_declination           = 0;
            max_inclination           = 0;
            time_to_man               = 0.0;
            time_to_unman             = 0.0;
            is_gun_manned             = false;
            manning_time              = 0.0;
            shell_damage              = 0.0;
            start_of_exclusion_radius = 0;
            end_of_exclusion_radius   = 0;
            calibre                   = 0.0;
        }
    };
    bool gun_manning_is_changing;
    std::list<struct gun_turret> gun_turrets;
    using gun_turret_itr       = std::list<struct gun_turret>::iterator;
    using const_gun_turret_itr = std::list<struct gun_turret>::const_iterator;
    using gun_barrel_itr       = std::list<struct gun_barrel>::iterator;
    using const_gun_barrel_itr = std::list<struct gun_barrel>::const_iterator;
    double maximum_gun_range;

    int propeller_1_id, propeller_2_id; // for display()
    int rudder_1_id, rudder_2_id;       // for display()

    bool
    is_target_in_blindspot(const struct gun_turret* gun, angle bearingToTarget);
    bool calculate_gun_angle(
        const double distance,
        angle& elevation,
        const double initial_velocity);
    void calc_max_gun_range(double initial_velocity);

    [[nodiscard]] bool detect_other_sea_objects() const override
    {
        return true;
    }

  public:
    // create empty object from specification xml file
    // construct a sea_object. called by heirs
    ship(game& gm_, const xml_elem& parent);

    void load(const xml_elem& parent) override;
    void save(xml_elem& parent) const override;

    [[nodiscard]] virtual shipclass get_class() const { return myclass; }

    void simulate(double delta_time, game& gm) override;

    virtual void sink();

    virtual void ignite(game& gm);
    [[nodiscard]] bool is_burning() const { return myfire != nullptr; }

    // command interface
    virtual gun_status fire_shell_at(const vector2& pos, game& gm);

    /** set up steering logic so object turns to new course.
        @param a - angle to turn to
        @param direction - give -1 to turn left, 1 to turn right, 0 for
       automatic
        @param hard_rudder - turn with hard rudder instead of normal rudder
       angle
    */
    virtual void
    head_to_course(const angle& a, int direction = 0, bool hard_rudder = true);

    virtual void set_rudder(double to); // -2...2
    virtual void set_throttle(int thr);

    virtual void remember_position(double t);
    [[nodiscard]] virtual const std::list<prev_pos>&
    get_previous_positions() const
    {
        return previous_positions;
    }

    [[nodiscard]] virtual bool has_smoke() const { return !smoke.empty(); }

    bool damage(const vector3& fromwhere, unsigned strength, game& gm) override;
    [[nodiscard]] unsigned
    calc_damage() const override; // returns damage in percent (100 means dead)
    // this depends on ship's tonnage, type, draught and depth (subs/sinking
    // ships)
    [[nodiscard]] virtual unsigned get_tonnage() const { return tonnage; }
    [[nodiscard]] virtual double get_fuel_level() const { return fuel_level; }
    [[nodiscard]] virtual angle get_turn_rate() const { return turn_rate; };
    [[nodiscard]] virtual double get_max_speed() const
    {
        return max_speed_forward;
    };
    virtual throttle_status get_throttle()
    {
        return (throttle_status) throttle;
    }
    [[nodiscard]] virtual double get_throttle_speed() const;
    [[nodiscard]] virtual double get_throttle_accel()
        const; // returns acceleration caused by current throttle
    [[nodiscard]] virtual bool
    screw_cavitation() const; // returns true if screw causes cavitation
    [[nodiscard]] virtual double get_rudder_pos() const { return rudder.angle; }
    [[nodiscard]] double get_noise_factor() const override;

    // needed for launching torpedoes
    std::pair<angle, double>
    bearing_and_range_to(const sea_object* other) const;
    [[nodiscard]] angle
    estimate_angle_on_the_bow(angle target_bearing, angle target_heading) const;

    // gun
    [[nodiscard]] virtual bool has_guns() const { return !gun_turrets.empty(); }
    virtual bool man_guns();
    virtual bool unman_guns();
    virtual bool is_gun_manned();
    virtual void gun_manning_changed(bool is_gun_manned, game& gm) { }
    virtual long num_shells_remaining();
    virtual double max_gun_range() { return maximum_gun_range; };

    // sonar
    [[nodiscard]] virtual const noise_signature& get_noise_signature() const
    {
        return noise_sign;
    }

    /* NOTE! the following function(s) are only for the editor or for
       custom convoy generation (called from class convoy).
       Nobody else should manipulate objects like this.
    */
    void manipulate_heading(angle hdg) override;

    /// compute bv_tree parameter values for collision tests
    [[nodiscard]] virtual bv_tree::param compute_bv_tree_params() const;
};
