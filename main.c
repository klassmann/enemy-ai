#include <SDL2/SDL.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <math.h>
#include <stdbool.h>

// Utility function for linear interpolation
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// Game settings
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define PLAYER_SPEED 400
#define PLAYER_POS_X 200
#define PLAYER_POS_Y 300
#define ENEMY_POS_X 500
#define ENEMY_POS_Y 300

typedef struct {
    float x, y;
    float width, height;
} Rect;

typedef struct {
    Rect rect;
    float speed;
    float cooldown;
    float max_distance;
    float current_cooldown;
    float home_x, home_y;
    bool is_following;
} Enemy;

lua_State *L;

/*
 * Here is the function which is in charge of reload the Lua script dynamically.
 * Everytime we load the script, we update the current Lua state doing the following:
 * - Evaluate the script again with luaL_dofile
 * - Get the global variable enemy_config and read each field of the table
 */
void reload_lua_script(const char *script_path, Enemy *enemy) {
    if (luaL_dofile(L, script_path) != LUA_OK) {
        printf("Error loading script: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return;
    }

    lua_getglobal(L, "enemy_config");
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "speed");
        enemy->speed = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "cooldown");
        enemy->cooldown = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "max_distance");
        enemy->max_distance = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1); // Pop the table
}

/*
 * We use to call the update_enemy function inside the Lua script.
 * Everytime we call the function we send two tables (player and enemy)
 * and also enemy home position and delta time.
 * The return is the next enemy position.
 */
void update_enemy(Enemy *enemy, Rect *player, float delta_time) {
    lua_getglobal(L, "update_enemy");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    lua_newtable(L);
    lua_pushnumber(L, player->x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, player->y);
    lua_setfield(L, -2, "y");

    lua_newtable(L);
    lua_pushnumber(L, enemy->rect.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, enemy->rect.y);
    lua_setfield(L, -2, "y");

    lua_pushnumber(L, enemy->home_x);
    lua_pushnumber(L, enemy->home_y);
    lua_pushnumber(L, delta_time);

    if (lua_pcall(L, 5, 2, 0) != LUA_OK) {
        printf("Error calling Lua function: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return;
    }

    enemy->rect.x = lerp(enemy->rect.x, lua_tonumber(L, -2), delta_time * enemy->speed);
    enemy->rect.y = lerp(enemy->rect.y, lua_tonumber(L, -1), delta_time * enemy->speed);
    lua_pop(L, 2);
}


int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

    SDL_Window *window = SDL_CreateWindow(
            "Lua AI Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    L = luaL_newstate();
    luaL_openlibs(L);

    Rect player = {PLAYER_POS_X, PLAYER_POS_Y, 50, 50};
    Enemy enemy = {{ENEMY_POS_X, ENEMY_POS_Y, 50, 50}, 100, 1.0f, 300, 0.0f, 500, 300, false};

    reload_lua_script("enemy_ai.lua", &enemy);

    bool running = true;
    Uint32 last_time = SDL_GetTicks();
    const Uint8 *keyboard = SDL_GetKeyboardState(NULL);

    while (running) {
        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_r) {

                    player.x = PLAYER_POS_X;
                    player.y = PLAYER_POS_Y;
                    enemy.rect.x = ENEMY_POS_X;
                    enemy.rect.y = ENEMY_POS_Y;

                    reload_lua_script("enemy_ai.lua", &enemy);
                }
            }
        }

        // Player movement
        if (keyboard[SDL_SCANCODE_W]) player.y -= PLAYER_SPEED * delta_time;
        if (keyboard[SDL_SCANCODE_S]) player.y += PLAYER_SPEED * delta_time;
        if (keyboard[SDL_SCANCODE_A]) player.x -= PLAYER_SPEED * delta_time;
        if (keyboard[SDL_SCANCODE_D]) player.x += PLAYER_SPEED * delta_time;

        update_enemy(&enemy, &player, delta_time);

        // Rendering
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect sdl_player = {player.x, player.y, player.width, player.height};
        SDL_RenderFillRect(renderer, &sdl_player);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect sdl_enemy = {enemy.rect.x, enemy.rect.y, enemy.rect.width, enemy.rect.height};
        SDL_RenderFillRect(renderer, &sdl_enemy);

        SDL_RenderPresent(renderer);
    }

    lua_close(L);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
