#pragma once
#include <level.h>
#include <raylib.h>

constexpr Vector2 anim_idle = {0.0f, 0.0f};
constexpr Vector2 anim_dead = {8.0f, 0.0f};
constexpr Vector2 anim_walk = {0.0f, 8.0f};
constexpr Vector2 anim_jump = {0.0f, 16.0f};
constexpr Vector2 anim_fall = {0.0f, 24.0f};
constexpr Vector2 anim_wall = {0.0f, 32.0f};

enum PlayerState {
    IDLING,
    WALKING,
    JUMPING,
    FALLING,
    DEAD,
};

enum Facing { LEFT, RIGHT };

struct Hitbox {
        Vector2 offset;
        Vector2 size;
};

struct Input {
        float move_x;
        bool jump_pressed;
};

struct Player {
        int coins = 0;
        int deaths = 0;

        float jump_height;
        float walk_speed;

        float jump_velocity;
        float walljump_velocity;

        float jump_gravity;
        float fall_gravity;
        float max_gravity = 140.0f;

        float jump_time_to_peak;
        float jump_time_to_fall;

        float wall_coyote_timer;
        float wall_coyote_time;

        float wall_cooldown_timer;
        float wall_cooldown_time;

        float respawn_timer;
        float respawn_time;

        int touches_wall;
        bool has_key;
        bool grounded = true;

        PlayerState state = IDLING;
        Facing facing;

        Vector2 pos = {0.0f, 0.0f};
        Vector2 vel = {0.0f, 0.0f};
        Vector2 last_entry = {0.0f, 0.0f};

        Hitbox body = {{0.0f, 0.0f}, {8.0f, 8.0f}};
        Hitbox hurtbox = {{1.0f, 2.0f}, {5.0f, 4.0f}};
};

int SetupPlayer(Player *player, Level *level);
Input GatherInput();

Rectangle GetPlayerHitbox(const Player &player);
Rectangle GetPlayerHurtbox(const Player &player);

void DrawPlayer(const Player &player, const Texture2D texture);
