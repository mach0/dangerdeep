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

// Sensors
// subsim (C) + (W). See LICENSE

#include "angle.h"
#include "vector3.h"

#pragma once

#define MIN_VISIBLE_DISTANCE 0.01f

class active_sensor;
class radar_sensor;
class passive_sonar_sensor;
class active_sonar_sensor;
class hfdf_sensor;
class lookout_sensor;
class sea_object;
class particle;
class game;
class sea_object;

///\brief Base class for all sensor types.
class sensor
{
  public:
    enum sensor_move_mode
    {
        rotate,
        sweep
    };

  private:
    /// Range of sensor. Mostly a decline value.
    double range;
    /// Bearing of dectector.
    angle bearing;
    /// Size of detector cone.
    double detection_cone;
    /// This flag shows in what direction the sensor moves.
    /// 1 right, -1 left.
    int move_direction{1};

  protected:
    /**
        This method calculates the decline of the signal strength.
        @parm d distance value in meters
        @return factor of declined signal
    */
    [[nodiscard]] virtual double get_distance_factor(double d) const;
    /**
        A detector can be directed to a specified bearing and has a detection
        cone. A target can only be detected when the target is within this cone.
        This method gets a location vector and uses then known bearing of the
        sensor and its detection angle to verify if the specified target is
       within this cone.
        @param r location vector from detecting to target unit
        @param h heading of the detecting unit
        @return is target within detection cone or not
    */
    [[nodiscard]] virtual bool
    is_within_detection_cone(const vector2& r, const angle& h) const;

  public:
    /**
        Constructor.
        @param range Range of sensor
        @param detectionAngle Size of detection cone of detector
    */
    sensor(double range = 0.0f, double detectionCone = 360.0f);
    /// Destructor
    virtual ~sensor() = default;
    ;

    /**
        Sets the range value.
        @param range new range value
    */
    virtual void set_range(double range) { this->range = range; }
    /**
        Sets the bearing value.
        @param bearing new bearing value
    */
    virtual void set_bearing(const angle& bearing) { this->bearing = bearing; }
    /**
        Sets the detection angle value.
        @param detectionAngle new detection angle
    */
    virtual void set_detection_cone(double detection_cone)
    {
        this->detection_cone = detection_cone;
    }
    /**
        Returns the range value.
        @return range
    */
    [[nodiscard]] virtual double get_range() const { return range; }
    /**
        Returns the bearing of the detector.
        @return bearing
    */
    [[nodiscard]] virtual angle get_bearing() const { return bearing; }
    /**
        Returns the detection angle.
        @return detectionAngle
    */
    [[nodiscard]] virtual double get_detection_cone() const
    {
        return detection_cone;
    }
    /**
        This method can be used to move the bearing of the detector. Whenever
        this method is called the bearing is shifted about the two third
        of the detection angle.
    */
    virtual void auto_move_bearing(sensor_move_mode mode = rotate);
    /**
        This method verifies if the target unit t can be detected by
        detecting unit d.
        @param gm game object. Some parameters are stored here.
        @param d detecting unit
        @param t target unit

        fixme: this is bad for some sensor types. Sonar detects only contacts,
       and could map several objects to one contact, so this relation "a detects
       b" is not possible for sonar. This function is mostly (or only?) called
       in a loop over all objects. So it could do the loop itself and return a
       list of objects or contacts, this would make the problem less worse. A
       general problem remains: sonar reports only contacts, not directly usable
       pointer to objects. Some subs need to aim after sonar contacts (XXI), so
       would need a pointer here, but this can be solved in a different way. So
       it would be ok for sonar to return just contacts, not pointers. But this
       would lead to a non-uniform interface for sensors. To be fixed...
    */
    virtual bool is_detected(
        const game* gm,
        const sea_object* d,
        const sea_object* t) const = 0;
};

///\brief Class for lookout.
class lookout_sensor : public sensor
{
  public:
    enum lookout_type
    {
        lookout_type_default
    };

  public:
    lookout_sensor(lookout_type type = lookout_type_default);
    ~lookout_sensor() override = default;
    ;
    /**
        This method verifies if the target unit t can be detected by
        detecting unit d.
        @param gm game object. Some parameters are stored here.
        @param d detecting unit
        @param t target unit
    */
    bool is_detected(const game* gm, const sea_object* d, const sea_object* t)
        const override;
    virtual bool
    is_detected(const game* gm, const sea_object* d, const particle* p) const;
};

///\brief Class for passive sonar based sensors.
class passive_sonar_sensor : public sensor
{
  public:
    // fixme: make heirs for special types here.
    enum passive_sonar_type
    {
        passive_sonar_type_default, /* fixme: tt_t4 is missing here */
        passive_sonar_type_tt_t5,
        passive_sonar_type_tt_t11
    };
    // fixme: add kdb, ghg, bg sonars.

  private:
    void init(passive_sonar_type type);

  public:
    passive_sonar_sensor(passive_sonar_type type = passive_sonar_type_default);
    ~passive_sonar_sensor() override = default;
    ;
    /**
        This method verifies if the target unit t can be detected by
        detecting unit d.
        @param gm game object. Some parameters are stored here.
        @param d detecting unit
        @param t target unit
    */
    bool is_detected(const game* gm, const sea_object* d, const sea_object* t)
        const override;
    /**
        This method verifies if the target unit t can be detected by
        detecting unit d.
        @param sound_level noise level of object t
        @param gm game object. Some parameters are stored here.
        @param d detecting unit
        @param t target unit
    */
    virtual bool is_detected(
        double& sound_level,
        const game* gm,
        const sea_object* d,
        const sea_object* t) const;
};

///\brief Base class for active sensors.
class active_sensor : public sensor
{
  protected:
    /**
        This method calculates the decline of the signal strength. For
        active sensors another function must be used than for passive
        sensors.
        @parm d distance value in meters
        @return factor of declined signal
    */
    [[nodiscard]] double get_distance_factor(double d) const override;

  public:
    active_sensor(double range = 0.0f);
    ~active_sensor() override = default;
    ;
    /**
        This method verifies if the target unit t can be detected by
        detecting unit d.
        @param gm game object. Some parameters are stored here.
        @param d detecting unit
        @param t target unit
    */
    bool is_detected(const game* gm, const sea_object* d, const sea_object* t)
        const override = 0;
};

///\brief Class for radar based sensors.
class radar_sensor : public active_sensor
{
  public:
    enum radar_type
    {
        radar_type_default,
        radar_british_type_271,
        radar_british_type_272,
        radar_british_type_273,
        radar_british_type_277,
        radar_german_fumo_29,
        radar_german_fumo_30,
        radar_german_fumo_61,
        radar_german_fumo_64,
        radar_german_fumo_391
    };

  private:
    void init(radar_type type);

  public:
    radar_sensor(radar_type type = radar_type_default);
    ~radar_sensor() override = default;
    ;
    /**
        This method verifies if the target unit t can be detected by
        detecting unit d.
        @param gm game object. Some parameters are stored here.
        @param d detecting unit
        @param t target unit
    */
    bool is_detected(const game* gm, const sea_object* d, const sea_object* t)
        const override;
};

///\brief Class for active sonar based sensors.
class active_sonar_sensor : public active_sensor
{
  public:
    enum active_sonar_type
    {
        active_sonar_type_default
    };

  private:
    void init(active_sonar_type type);

  public:
    active_sonar_sensor(active_sonar_type type = active_sonar_type_default);
    ~active_sonar_sensor() override = default;
    ;
    /**
        This method verifies if the target unit t can be detected by
        detecting unit d.
        @param gm game object. Some parameters are stored here.
        @param d detecting unit
        @param t target unit
    */
    bool is_detected(const game* gm, const sea_object* d, const sea_object* t)
        const override;
};
