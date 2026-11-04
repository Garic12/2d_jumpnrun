#include "player.h"
#include "globals.h"
#include "raylib.h"

int SetupPlayer(Player *player, Level *level) {
    player->jump_height = 18.0f;
    player->jump_time_to_peak = 0.35f;
    player->jump_time_to_fall = 0.25f;

    player->jump_velocity =
        (2.0f * player->jump_height) / player->jump_time_to_peak;
    player->walljump_velocity =
        (1.0f * player->jump_height) / player->jump_time_to_peak;
    player->jump_gravity =
        (-2.0f * player->jump_height) /
        (player->jump_time_to_peak * player->jump_time_to_peak) * -1;
    player->fall_gravity =
        (-2.0f * player->jump_height) /
        (player->jump_time_to_fall * player->jump_time_to_fall) * -1;

    player->wall_coyote_time = 0.1f;
    player->wall_coyote_timer = 0.0f;

    player->wall_cooldown_time = 0.5f;
    player->wall_cooldown_timer = 0.0f;

    player->walk_speed = 45.0f;

    player->pos = level->player_start;
    player->last_entry = level->player_start;

    player->has_key = false;

    return 0;
}

Input GatherInput() {
    Input input = {};
    if (IsKeyDown(KEY_A))
        input.move_x -= 1.0f;
    if (IsKeyDown(KEY_D))
        input.move_x += 1.0f;
    input.jump_pressed = IsKeyPressed(KEY_SPACE);
    return input;
}

Rectangle GetPlayerHitbox(const Player &player) {
    return {player.pos.x + player.body.offset.x,
            player.pos.y + player.body.offset.y, player.body.size.x,
            player.body.size.y};
}

Rectangle GetPlayerHurtbox(const Player &player) {
    return {player.pos.x + player.hurtbox.offset.x,
            player.pos.y + player.hurtbox.offset.y, player.hurtbox.size.x,
            player.hurtbox.size.y};
}

void DrawPlayer(const Player &player, const Texture2D texture) {
    Rectangle src = {0.0f, 0.0f, TILESIZE, TILESIZE};

    switch (player.state) {
    case IDLING:
        src.x = anim_idle.x;
        src.y = anim_idle.y;
        if (player.facing == RIGHT) {
            src.width = -src.width;
        }
        break;
    case WALKING:
        src.x = anim_walk.x;
        src.y = anim_walk.y;
        if (player.facing == RIGHT) {
            src.width = -src.width;
        }
        break;
    case JUMPING:
        src.x = anim_jump.x;
        src.y = anim_jump.y;
        if (player.facing == RIGHT) {
            src.width = -src.width;
        }
        break;
    case FALLING:
        src.x = anim_fall.x;
        src.y = anim_fall.y;
        if (player.facing == RIGHT) {
            src.width = -src.width;
        }
        break;
    }

    Rectangle dest = {0.0f, 0.0f, TILESIZE, TILESIZE};
    dest.x = player.pos.x;
    dest.y = player.pos.y;
    DrawTexturePro(texture, src, dest, {0, 0}, 0.0f, WHITE);
}
