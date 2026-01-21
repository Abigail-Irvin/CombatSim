#include <fstream>
#include <nlohmann/json.hpp>
#include "main.hpp"
using json = nlohmann::json;

void draw_grid(SDL_Renderer *renderer)
{
    // Draws up the grid lines on the main simulation window
    SDL_SetRenderDrawColor(renderer, 0x00, 0x50, 0x00, 0x00);
    SDL_FRect row_line {.x=0, .y=0, .w=WINDOW_WIDTH, .h=0};
    SDL_FRect col_line {.x=0, .y=0, .w=0, .h=WINDOW_HEIGHT};
    for (row_line.y =0; row_line.y < WINDOW_HEIGHT; row_line.y += GRID_SIZE) {
        SDL_RenderRect(renderer, &row_line);
    }
    for (col_line.x =0; col_line.x < WINDOW_WIDTH; col_line.x += GRID_SIZE) {
        SDL_RenderRect(renderer, &col_line);
    }
}

SDL_Surface *SurfLoadHelperPng(const char *file)
{
    // Helpers to load PNG images as SDL surfaces, returns NULL on failure
    SDL_Surface *loadedImage = SDL_LoadPNG(file);
    if (loadedImage == NULL) {
        SDL_Log("Unable to load image %s! SDL Error: %s\n", file, SDL_GetError());
        return NULL;
    }
    return loadedImage;
}

SDL_Texture *TextureLoadFromSurf(SDL_Renderer *renderer, SDL_Surface *surface)
{
    // Helper to create texture from surface, returns NULL on failure
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        SDL_Log("Unable to create texture from surface! SDL Error: %s\n", SDL_GetError());
        return NULL;
    }
    return texture;
}

Platform::Platform(std::vector<float> position, std::vector<float> current_heading, std::string name, std::string side, std::string platform_icon_path, std::string platform_missile_icon_path, float speed, float turn_rate)
{
    // Initialize platform attributes
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
    this->platform_icon_path = platform_icon_path;
    this->platform_missile_icon_path = platform_missile_icon_path;
    platform_surface = SurfLoadHelperPng(platform_icon_path.c_str());
    platform_missile_surface = SurfLoadHelperPng(platform_missile_icon_path.c_str());
    std::cout << "Created platform: " << platform_name << " of side: " << platform_side << std::endl;
}

void Platform::updatePosition(float time_step)
{
    // Main Update function for platform, updates position based on current heading and speed, also updates active missiles
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

void Platform::fireWeapon()
{
    // Fires the primary weapon, creating a new missile and adding it to active missiles
    std::cout << platform_name << " firing weapon: " << primary_weapon.name << std::endl;
    Missile m(primary_weapon, position, current_heading);
    active_missiles.push_back(m);
}

void Platform::setWeapon(Weapon w)
{
    // set the primary weapon for this platform
    primary_weapon = w;
}

void Platform::setTarget(Platform *target)
{
    // set the target platform for this platform, used for missile guidance
    target_platform = target;
}

Missile Platform::getMissile(int index)
{
    // retrieve an active missile by index, exits with error if index is out of range
    if (!hasMissile(index)) {
        std::cerr << "Error: Missile index out of range." << std::endl;
        exit(1);
    }
    return active_missiles[index];
}

bool Platform::isDestroyed()
{
    // check if platform is destroyed
    return is_destroyed;
}

bool Platform::hasMissile(int index)
{
    // check if platform has an active missile at the given index
    if (index < 0 || index >= active_missiles.size()) {
        return false;
    }
    return true;
}

std::vector<Missile> Platform::getActiveMissiles()
{
    return active_missiles;
}

void Platform::setSensor(Sensor s)
{
    // set the primary sensor for this platform
    primary_sensor = s;
}

void Platform::destroyPlatform()
{ 
    // mark platform as destroyed
    std::cout << "Platform " << platform_name << " destroyed!" << std::endl;
    is_destroyed = true;
}

float Platform::getAngleOfRotation()
{
    // retrieve current angle of rotation for rendering
    return angle_of_rotation;
}

std::vector<float> Platform::getPosition()
{
    // retrieve current position of platform
    return position;
}

void Platform::setTextures(SDL_Renderer *renderer)
{
    platform_texture = TextureLoadFromSurf(renderer, platform_surface);
    platform_missile_texture = TextureLoadFromSurf(renderer, platform_missile_surface);
}

SDL_Texture *Platform::getPlatformTextures()
{
    return platform_texture;
}

SDL_Texture *Platform::getMissileTextures()
{
    return platform_missile_texture;
}

Missile::Missile(Weapon w, std::vector<float> pos, std::vector<float> heading)
{
    // Initialize missile attributes
    missile_weapon = w;
    position = pos;
    current_heading = heading;
    exploded = false;
    final_approach = false;
    angle_of_rotation = std::atan2(current_heading[1], current_heading[0]) * (180.0f / 3.14159f) + 90.0f;
}

bool Missile::isExploded()
{
    // check if missile has exploded
    return exploded;
}

std::vector<float> Missile::getPosition()
{
    // retrieve current position of missile
    return position;
}

float Missile::getAngleOfRotation()
{
    // retrieve current angle of rotation for rendering
    return angle_of_rotation;
}

void Missile::updatePosition(float time_step)
{
    // Positional update based on current heading and speed, also handles final impact calculation
    angle_of_rotation = std::atan2(current_heading[1], current_heading[0]) * (180.0f / 3.14159f) + 90.0f;
    position[0] += current_heading[0] * missile_weapon.speed * time_step;
    position[1] += current_heading[1] * missile_weapon.speed * time_step;
    if (final_target_position.size() == 2) {
        float distance_to_target = magnitude2D({final_target_position[0] - position[0], final_target_position[1] - position[1]});
        if (distance_to_target < 14.0f && !exploded) {
            exploded = true;
            std::cout << "Missile exploded at target position (" << final_target_position[0] << ", " << final_target_position[1] << ")\n";
        }
    }
}

void Missile::adjustCourse(std::vector<float> target_pos)
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


int main(int argc, char *argv[])
{
    // JSON parsing and scenario creation
    std::ifstream f("example.json");
    json data = json::parse(f);
    Weapon example_weapon;
    Sensor example_sensor;
    std::vector<Platform> platforms;

    for (json::iterator it = data.begin(); it != data.end(); ++it) {
        // Check for weapon and sensor data first and assign to example objects, these are then used for platforms after creation
        if (it.key() == "example-missile") {
            example_weapon.name = (*it)["name"];
            example_weapon.range = (*it)["range"];
            example_weapon.speed = (*it)["speed"];
            example_weapon.damage = (*it)["damage"];
            example_weapon.turning_radius = (*it)["turning_radius"];
        } else if (it.key() == "example-sensor") {
            example_sensor.name = (*it)["name"];
            example_sensor.detection_range = (*it)["detection_range"];
            example_sensor.angle_of_view = (*it)["angle_of_view"];
        }
        else {
            // Iterate through platforms and create Platform objects, assinging based on JSON data
            std::string platform_side = "none";
            float platform_speed = 0.0f;
            float platform_turn_rate = 0.0f;
            std::vector<float> default_position = {0.0f, 0.0f};
            std::vector<float> default_heading = {1.0f, 0.0};
            std::string platform_icon_path;
            std::string platform_missile_icon_path;
            for (auto& element : *it) {
                for (auto& item : element.items()) {
                    if (item.key() == "side") {
                        platform_side = item.value();
                    } else if (item.key() == "platform") {
                        platform_speed = item.value()[0];
                        platform_turn_rate = item.value()[1];
                        default_position[0] = item.value()[2];
                        default_position[1] = item.value()[3];
                        default_heading[0] = item.value()[4];
                        default_heading[1] = item.value()[5];
                    } else if (item.key() == "icon") {
                        platform_icon_path = item.value()[0];
                        platform_missile_icon_path = item.value()[1];
                    }
                }
            }
            Platform p(default_position, default_heading, std::string(it.key()), platform_side, platform_icon_path, platform_missile_icon_path, platform_speed, platform_turn_rate);
            platforms.push_back(p);
        }
        
    }
    for (auto& p : platforms) {
        p.setWeapon(example_weapon);
        p.setSensor(example_sensor);
    }

    // Basic Test: have red platform fire weapon at blue platform, and vice versa
    platforms[1].fireWeapon();
    platforms[1].setTarget(&platforms[0]);

    platforms[0].setTarget(&platforms[1]);
    platforms[0].fireWeapon();

    // SDL3 Initialization and window creation
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }

    if (!SDL_CreateWindowAndRenderer("Combat Sim", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return 3;
    }

    //SDL loading textures from surfaces for platforms and missiles
    for (auto& p : platforms) {
            p.setTextures(renderer);
    }

    // Main loop
    while (1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_EVENT_QUIT) {
            break;
        }
        // Render simulation state here
        SDL_SetRenderDrawColor(renderer, 0x00, 0x10, 0x00, 0x00);
        SDL_RenderClear(renderer);
        draw_grid(renderer);
        for (auto& p : platforms) {
            for (auto& m : p.getActiveMissiles()) {
                if (m.isExploded()) {
                    continue;
                }
                SDL_RenderTextureRotated(renderer, p.getMissileTextures(), NULL, new SDL_FRect{m.getPosition()[0], m.getPosition()[1], 24.0f, 24.0f}, m.getAngleOfRotation(), NULL, SDL_FLIP_NONE);
            }
            if (!p.isDestroyed()) {
                p.updatePosition(0.000016f); // Rough sim time step simulator
                SDL_RenderTextureRotated(renderer, p.getPlatformTextures(), NULL, new SDL_FRect{p.getPosition()[0], p.getPosition()[1], 32.0f, 32.0f}, p.getAngleOfRotation(), NULL, SDL_FLIP_NONE);
            }
        }
        SDL_RenderPresent(renderer);
    }
    // Cleanup and shutdown
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}