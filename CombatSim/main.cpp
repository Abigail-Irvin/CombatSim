#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include "main.hpp"

using json = nlohmann::json;

void draw_grid(SDL_Renderer *renderer)
{
    // Draw grid lines
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
    SDL_Surface *loadedImage = SDL_LoadPNG(file);
    if (loadedImage == NULL) {
        SDL_Log("Unable to load image %s! SDL Error: %s\n", file, SDL_GetError());
        return NULL;
    }
    return loadedImage;
}

SDL_Texture *TextureLoadFromSurf(SDL_Renderer *renderer, SDL_Surface *surface)
{
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        SDL_Log("Unable to create texture from surface! SDL Error: %s\n", SDL_GetError());
        return NULL;
    }
    return texture;
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
                    }
                }
            }
            Platform p(default_position, default_heading, std::string(it.key()), platform_side, platform_speed, platform_turn_rate);
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
    SDL_Surface *blue_surface;
    SDL_Surface *red_surface;
    SDL_Surface *blue_missile_surface;
    SDL_Surface *red_missile_surface;
    SDL_Texture *blue_texture;
    SDL_Texture *red_texture;
    SDL_Texture *red_missile_texture;
    SDL_Texture *blue_missile_texture;
    SDL_Event event;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }

    if (!SDL_CreateWindowAndRenderer("Combat Sim", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return 3;
    }

    //SDL loading surfaces
    blue_surface = SurfLoadHelperPng("blue.png");
    red_surface = SurfLoadHelperPng("red.png");
    red_missile_surface = SurfLoadHelperPng("red-missile.png");
    blue_missile_surface = SurfLoadHelperPng("blue-missile.png");
    blue_texture = TextureLoadFromSurf(renderer, blue_surface);
    red_texture = TextureLoadFromSurf(renderer, red_surface);
    red_missile_texture = TextureLoadFromSurf(renderer, red_missile_surface);
    blue_missile_texture = TextureLoadFromSurf(renderer, blue_missile_surface);

    // Main loop
    while (1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_EVENT_QUIT) {
            break;
        }
        // Update simulation state here
        for (auto& p : platforms) {
            p.updatePosition(0.000016f); // assuming ~60 FPS, so ~16ms per frame
        }
        // Render simulation state here
        SDL_SetRenderDrawColor(renderer, 0x00, 0x10, 0x00, 0x00);
        SDL_RenderClear(renderer);
        draw_grid(renderer);
        if (platforms[1].hasMissile(0)) {
            // If red platform has an active missile, render it
            SDL_RenderTextureRotated(renderer, red_missile_texture, NULL, new SDL_FRect{platforms[1].getMissile(0).getPosition()[0], platforms[1].getMissile(0).getPosition()[1], 24.0f, 24.0f}, platforms[1].getMissile(0).getAngleOfRotation(), NULL, SDL_FLIP_NONE);
        }

        if (platforms[0].hasMissile(0)) {
            // If blue platform has an active missile, render it
            SDL_RenderTextureRotated(renderer, blue_missile_texture, NULL, new SDL_FRect{platforms[0].getMissile(0).getPosition()[0], platforms[0].getMissile(0).getPosition()[1], 24.0f, 24.0f}, platforms[0].getMissile(0).getAngleOfRotation(), NULL, SDL_FLIP_NONE);
        }

        if (!platforms[0].isDestroyed()) {
            // If blue platform is not destroyed, render it
            SDL_RenderTextureRotated(renderer, blue_texture, NULL, new SDL_FRect{platforms[0].getPosition()[0], platforms[0].getPosition()[1], 32.0f, 32.0f}, platforms[0].getAngleOfRotation(), NULL, SDL_FLIP_NONE);
        }
        if (!platforms[1].isDestroyed()) {
            // If red platform is not destroyed, render it
            SDL_RenderTextureRotated(renderer, red_texture, NULL, new SDL_FRect{platforms[1].getPosition()[0], platforms[1].getPosition()[1], 32.0f, 32.0f}, platforms[1].getAngleOfRotation(), NULL, SDL_FLIP_NONE);
        }
        
        SDL_RenderPresent(renderer);
    }
    // Cleanup and shutdown
    SDL_DestroySurface(blue_surface);
    SDL_DestroySurface(red_surface);
    SDL_DestroySurface(red_missile_surface); 
    SDL_DestroySurface(blue_missile_surface); 

    SDL_DestroyTexture(blue_texture);
    SDL_DestroyTexture(red_texture);
    SDL_DestroyTexture(red_missile_texture);
    SDL_DestroyTexture(blue_missile_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}