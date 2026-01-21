#pragma once
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <cmath>

float magnitude2D(std::vector<float> vec)
{
    // Simple helper to calculate magnitude of 2D vector
    return sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
}

std::vector<float> normalize2D(std::vector<float> vec)
{
    // Simple helper to normalize a 2D vector
    float mag = magnitude2D(vec);
    return {vec[0] / mag, vec[1] / mag};
}

std::vector<float> heading_lerp(std::vector<float> start, std::vector<float> end, float t)
{
    // Linearly interpolate between two headings based on turning radius
    std::vector<float> result = {std::lerp(start[0], end[0], t), std::lerp(start[1], end[1], t)};
    return result;
}

struct Weapon
{
    // weapon attributes for missiles pulled from JSON
    std::string name; // weapon name
    float range; // maximum range
    float speed; // speed of missile
    float damage; // damage inflicted on hit
    float turning_radius; // turning radius for course adjustments
};
struct Sensor
{
    // sensor attributes pulled from JSON
    std::string name; // sensor name
    float detection_range; // maximum detection range
    float angle_of_view; // angle of view in degrees
};

class Missile
{
    // Missile class representing fired missiles
    protected:
    std::string side; // side/faction of missile
    Weapon missile_weapon; // contains speed, range, damage, turning radius of missile
    std::vector<float> position; // x, y coordinates
    std::vector<float> current_heading; // direction missile is currently heading (always at its assigned speed)
    std::vector<float> final_target_position; // used for final course adjustments and impact calculations
    bool final_approach; // flag for whether missile is in final approach phase
    bool exploded; // flag for whether missile has exploded
    float angle_of_rotation; // current angle of rotation for rendering

    public:
    Missile(Weapon w, std::vector<float> pos, std::vector<float> heading);
    bool isExploded();
    std::vector<float> getPosition();
    float getAngleOfRotation();
    void updatePosition(float time_step);
    void adjustCourse(std::vector<float> target_pos);
};

class Platform
{
    // Platform class representing combat platforms that can move, fire missiles, and have sensors
    protected:
    bool is_destroyed; // platform destruction status
    float debug_sensor_refresh_time; // time interval for sensor updates
    float current_sensor_refresh_time; // current time since last sensor update
    Weapon primary_weapon; // primary weapon assigned to platform
    Sensor primary_sensor; // primary sensor assigned to platform
    std::string platform_name; // name identifier for platform
    std::string platform_side; // side/faction of platform
    float max_speed; // maximum speed of platform
    float max_turn_rate; // maximum turn rate of platform
    std::vector<float> position; // x, y coordinates
    std::vector<float> current_heading; // direction platform is currently heading
    std::vector<Missile> active_missiles;
    Platform *target_platform; // currently assigned target platform
    float angle_of_rotation; // current angle of rotation for rendering

    public:
    Platform(std::vector<float> position, std::vector<float> current_heading, std::string name, std::string side, float speed, float turn_rate);
    void updatePosition(float time_step);
    void fireWeapon();
    void setWeapon(Weapon w);
    void setTarget(Platform *target);
    Missile getMissile(int index);
    bool isDestroyed();
    bool hasMissile(int index);
    void setSensor(Sensor s);
    void destroyPlatform();
    float getAngleOfRotation();
    std::vector<float> getPosition();
};
std::vector<float> normalize2D(std::vector<float> vec);
std::vector<float> heading_lerp(std::vector<float> start, std::vector<float> end, float t);
float magnitude2D(std::vector<float> vec);
const int WINDOW_WIDTH = 800; // window dimensions for SDL rendering
const int WINDOW_HEIGHT = 600; // window dimensions for SDL rendering
const int GRID_SIZE = 32; // size of grid cells for rendering the background