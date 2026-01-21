#pragma once
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <cmath>

float magnitude2D(std::vector<float> vec)
{
    return sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
}

std::vector<float> normalize2D(std::vector<float> vec)
{
    float mag = magnitude2D(vec);
    return {vec[0] / mag, vec[1] / mag};
}

std::vector<float> heading_lerp(std::vector<float> start, std::vector<float> end, float t)
{
    std::vector<float> result = {std::lerp(start[0], end[0], t), std::lerp(start[1], end[1], t)};
    return result;
}

struct Weapon
{
    std::string name;
    float range;
    float speed;
    float damage;
    float turning_radius;
};
struct Sensor
{
    std::string name;
    float detection_range;
    float angle_of_view;
};

class Missile
{
    protected:
    std::string side;
    Weapon missile_weapon; // contains speed, range, damage, turning radius of missile
    std::vector<float> position; // x, y coordinates
    std::vector<float> current_heading; // direction missile is currently heading (always at its assigned speed)
    std::vector<float> final_target_position; // used for final course adjustments and impact calculations
    bool final_approach;
    bool exploded;
    float angle_of_rotation;

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
    protected:
    bool is_destroyed;
    float debug_sensor_refresh_time;
    float current_sensor_refresh_time;
    Weapon primary_weapon;
    Sensor primary_sensor;
    std::string platform_name;
    std::string platform_side;
    float max_speed;
    float max_turn_rate;
    std::vector<float> position; // x, y coordinates
    std::vector<float> current_heading; // direction platform is currently heading
    std::vector<Missile> active_missiles;
    Platform *target_platform; // currently assigned target platform
    float angle_of_rotation;

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
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int GRID_SIZE = 32;