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
    // Linear interpolation between two 2D points, will be used for missile course adjustments and finds a middle point between start and end based on t value (0.0 to 1.0)
    //std::vector<float> result({start[0] * t + end[0] * (1.0f - t), start[1] * t + end[1] * (1.0f - t)});
    /*
    if (result[0] < 0.0f && start[0] > 0.0f) {
        result[0] = start[0];
    }
    else if (result[0] > 0.0f && start[0] < 0.0f) {
        result[0] = start[0];
    }
    if (result[1] < 0.0f && start[1] > 0.0f) {
        result[1] = start[1];
    }
    else if (result[1] > 0.0f && start[1] < 0.0f) {
        result[1] = start[1];
    }
        */
    //result = normalize2D(result);
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
    Missile(Weapon w, std::vector<float> pos, std::vector<float> heading)
    {
        missile_weapon = w;
        position = pos;
        current_heading = heading;
        exploded = false;
        final_approach = false;
        angle_of_rotation = std::atan2(current_heading[1], current_heading[0]) * (180.0f / 3.14159f) + 90.0f;
    }

    bool isExploded()
    {
        return exploded;
    }

    std::vector<float> getPosition()
    {
        return position;
    }

    float getAngleOfRotation()
    {
        return angle_of_rotation;
    } 

    void updatePosition(float time_step)
    {
        angle_of_rotation = std::atan2(current_heading[1], current_heading[0]) * (180.0f / 3.14159f) + 90.0f;
        position[0] += current_heading[0] * missile_weapon.speed * time_step;
        position[1] += current_heading[1] * missile_weapon.speed * time_step;
        if (final_target_position.size() == 2) {
            //std::cout << final_target_position[0] << ", " << final_target_position[1] << std::endl;
            float distance_to_target = magnitude2D({final_target_position[0] - position[0], final_target_position[1] - position[1]});
            //std::cout << "Missile distance to target: " << distance_to_target << std::endl;
            if (distance_to_target < 14.0f && !exploded) {
                exploded = true;
                std::cout << "Missile exploded at target position (" << final_target_position[0] << ", " << final_target_position[1] << ")\n";
            }
        }
    }

    void adjustCourse(std::vector<float> target_pos)
    {
        // Simulates getting course correction from platform's sensor and adjusting heading towards target
        std::vector<float> to_target = {target_pos[0] - position[0], target_pos[1] - position[1]};
        to_target = normalize2D(to_target);
        std::vector<float> old_heading = current_heading;
        current_heading = heading_lerp(current_heading, to_target, missile_weapon.turning_radius);
        if (abs(current_heading[0] - old_heading[0]) < 0.04f && abs(current_heading[1] - old_heading[1]) < 0.04f) {
            final_approach = true;   
        }
        if (final_approach) {
            final_target_position = target_pos;
        }
    }
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
    Platform(std::vector<float> position, std::vector<float> current_heading, std::string name, std::string side, float speed, float turn_rate)
    {
        is_destroyed = false;
        current_sensor_refresh_time = 0.0f;
        debug_sensor_refresh_time = 0.05f;
        this->position = position;
        this->current_heading = current_heading;
        platform_name = name;
        platform_side = side;
        max_speed = speed;
        max_turn_rate = turn_rate;
        angle_of_rotation = 0.0f;
        std::cout << "Created platform: " << platform_name << " of side: " << platform_side << std::endl;
    }

    void setWeapon(Weapon w)
    {
        primary_weapon = w;
    }

    void setTarget(Platform *target)
    {
        // set the target platform for this platform, used for missile guidance
        target_platform = target;
    }

    Missile getMissile(int index)
    {
        if (!hasMissile(index)) {
            std::cerr << "Error: Missile index out of range." << std::endl;
            exit(1);
        }
        return active_missiles[index];
    }

    bool isDestroyed()
        {
            return is_destroyed;
        }

    bool hasMissile(int index)
    {
        if (index < 0 || index >= active_missiles.size()) {
            return false;
        }
        return true;
    }

    bool checkForMissiles()
    {
        return active_missiles.size() > 0;
    }

    void setSensor(Sensor s)
    {
        primary_sensor = s;
    }

    void destroyPlatform()
    {
        std::cout << "Platform " << platform_name << " destroyed!" << std::endl;
        is_destroyed = true;
    }

    float getAngleOfRotation()
    {
        return angle_of_rotation;
    }

    void updatePosition(float time_step)
    {
        if (!isDestroyed()) { 
            angle_of_rotation = std::atan2(current_heading[1], current_heading[0]) * (180.0f / 3.14159f) + 90.0f;
            position[0] += current_heading[0] * this->max_speed * time_step;
            position[1] += current_heading[1] * this->max_speed * time_step;
        }   
        for (auto& m : active_missiles) {
            m.updatePosition(time_step);
        }
        current_sensor_refresh_time += time_step;
        if (current_sensor_refresh_time >= debug_sensor_refresh_time) {
            int missile_index = 0;
            int to_delete = 0;
            bool delete_missile = false;
            for (auto& m : active_missiles) {
                if (m.isExploded()) {
                    delete_missile = true;
                    to_delete = missile_index;
                    continue;
                }
                if (!isDestroyed()){ // if platform is destroyed, missiles no longer get course updates
                    m.adjustCourse(target_platform->getPosition());
                }
                
                missile_index++;
            }
            std::vector<Missile>::iterator it = active_missiles.begin();
            std::advance(it, to_delete);
            if (hasMissile(to_delete) && delete_missile)
            {
                active_missiles.erase(it);
                delete_missile = false;
                target_platform->destroyPlatform();
            }
            current_sensor_refresh_time = 0.0f;
        }
    }

    std::vector<float> getPosition()
    {
        return position;
    }

    void fireWeapon()
    {
        std::cout << platform_name << " firing weapon: " << primary_weapon.name << std::endl;
        Missile m(primary_weapon, position, current_heading);
        active_missiles.push_back(m);
    }
};
std::vector<float> normalize2D(std::vector<float> vec);
std::vector<float> heading_lerp(std::vector<float> start, std::vector<float> end, float t);
float magnitude2D(std::vector<float> vec);
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int GRID_SIZE = 32;