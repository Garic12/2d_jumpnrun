#include "globals.h"
#include "level.h"
#include "player.h"
#include "resources.h"
#include <imgui.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <raylib.h>
#include <raymath.h>
#include <rlImGui.h>
#include <vector>

using json = nlohmann::json;

void DrawFinishScreen(Player player, int num_coins, float run_time) {
    int boxWidth = 300;
    int boxHeight = 205;

    int x = (GetScreenWidth() - boxWidth) / 2;
    int y = (GetScreenHeight() - boxHeight) / 2;

    DrawRectangle(x, y, boxWidth, boxHeight, BLACK);

    DrawRectangle(x, y, boxWidth, 5, GOLD);
    DrawRectangle(x, y + boxHeight - 5, boxWidth, 5, GOLD);
    DrawRectangle(x, y, 5, boxHeight, GOLD);
    DrawRectangle(x + boxWidth - 5, y, 5, boxHeight, GOLD);

    int minutes = (int)run_time / 60;
    float seconds = run_time - minutes * 60;

    DrawText("YOU WIN!", x + 75, y + 25, 35, GOLD);
    DrawText(TextFormat("Coins: %d/%d", player.coins, num_coins), x + 60,
             y + 85, 25, WHITE);
    DrawText(TextFormat("Deaths: %d", player.deaths), x + 60, y + 120, 25, RED);
    DrawText(TextFormat("Time: %02d:%05.2f", minutes, seconds), x + 60, y + 155,
             25, SKYBLUE);
}

Vector2 Perpendicular(Vector2 v) { return {-v.y, v.x}; }

Vector2 ProjectionMinMax(const Vector2 axis, const Vector2 *points, int size) {
    float min = FLT_MAX;
    float max = -FLT_MAX;

    for (int i = 0; i < size; i++) {
        float projection = Vector2DotProduct(points[i], axis);

        if (projection < min) {
            min = projection;
        }
        if (projection > max) {
            max = projection;
        }
    }
    return {min, max};
}

bool CollisionRectSpike(const Rectangle &rect, const Spike &spike) {
    // Calculate all axes we want to check the collision over
    const Vector2 axes[]{
        {1.0f, 0.0f},
        {0.0f, 1.0f},
        Perpendicular(Vector2Subtract(spike.a, spike.b)),
        Perpendicular(Vector2Subtract(spike.b, spike.c)),
        Perpendicular(Vector2Subtract(spike.c, spike.a)),
    };

    Vector2 corners_rect[4] = {{rect.x, rect.y},
                               {rect.x + rect.width, rect.y},
                               {rect.x, rect.y + rect.height},
                               {rect.x + rect.width, rect.y + rect.height}};
    Vector2 corners_spike[3] = {spike.a, spike.b, spike.c};

    for (int i = 0; i < 5; i++) {
        Vector2 minmax_rect = ProjectionMinMax(axes[i], corners_rect, 4);
        Vector2 minmax_spike = ProjectionMinMax(axes[i], corners_spike, 3);

        if (minmax_rect.y < minmax_spike.x || minmax_spike.y < minmax_rect.x) {
            return false;
        }
    }
    return true;
}

int main() {
    int DEBUG = 0;
    bool fullscreen = false;
    float screenWidth = 800;
    float screenHeight = 600;

    int game_finished = false;
    float run_time = 0.0f;

    InitWindow(screenWidth, screenHeight, "Sidescroller");
    SetTargetFPS(60);
    rlImGuiSetup(true);

    // Load world with all sublevels
    Image image = LoadImageFromMemory(".png", res_tileset_png,
                                      static_cast<int>(res_tileset_png_size));
    if (image.data == nullptr) {
        std::cerr << "Failed to load tileset: " << "\n";
        return 1;
    }

    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    if (texture.id == 0) {
        std::cerr << "Failed to create texture\n";
        return 1;
    }

    json ldtk = json::parse(res_main_ldtk, res_main_ldtk + res_main_ldtk_size);
    const auto level_arr = ldtk["levels"];

    World world;
    int num_coins = 0;
    world.levels.reserve(level_arr.size());
    for (const auto &level : level_arr) {
        world.levels.push_back(LoadLevel(level, num_coins));
    }
    assert(num_coins != 0);
    world.current_level = &world.levels[4];

    Player player;
    if (SetupPlayer(&player, world.current_level) != 0) {
        CloseWindow();
        return 1;
    }

    Camera2D cam = {.offset = {screenWidth / 2.0f, screenHeight / 2.0f},
                    .target = player.pos,
                    .rotation = 0.0,
                    .zoom = 6.5f};

    // --- Game Loop ---
    while (!WindowShouldClose()) {
        if (game_finished) {
            BeginDrawing();
            ClearBackground({24, 26, 40, 255});
            DrawFinishScreen(player, num_coins, run_time);
            EndDrawing();
            continue;
        }

        float dt = GetFrameTime();
        if (!game_finished) {
            run_time += dt;
        }

        if (IsKeyPressed(KEY_B)) {
            fullscreen = !fullscreen;

            if (fullscreen) {
                int monitor = GetCurrentMonitor();
                screenWidth = GetMonitorWidth(monitor);
                screenHeight = GetMonitorHeight(monitor);
            } else {
                screenWidth = 800;
                screenHeight = 600;
            }
            SetWindowSize(screenWidth, screenHeight);
            cam.offset = {screenWidth / 2.0f, screenHeight / 2.0f};
            ToggleFullscreen();
        }

        Input input = {};
        if (!ImGui::GetIO().WantCaptureKeyboard)
            input = GatherInput();

        // Reset Variables
        player.touches_wall = 0;

        // Horizontal movement
        float dx = 0.0f;
        if (player.state != DEAD) {
            dx = input.move_x * player.walk_speed * dt + player.vel.x * dt;
            player.pos.x += dx;
        }

        // Horizontal drag
        if (input.move_x == 0) {
            player.vel.x *= 0.95f;
        } else {
            player.vel.x *= 0.85f;
        }

        // Horizontal collision check
        const Rectangle hitbox = GetPlayerHitbox(player);
        const TileBounds bounds = GetTileBounds(hitbox);
        for (int y = bounds.top; y <= bounds.bottom; ++y) {
            for (int x = bounds.left; x <= bounds.right; ++x) {
                if (IsColliding(world.current_level, x, y) == 1) {
                    if (dx > 0.0f) {
                        player.pos.x = x * TILESIZE - player.body.offset.x -
                                       player.body.size.x;
                    } else if (dx < 0.0f) {
                        player.pos.x =
                            (x + 1) * TILESIZE - player.body.offset.x;
                    }
                }
            }
        }

        // Check if player is standing next to wall
        {
            constexpr float probe = 0.5f; // pixels
            const Rectangle hitbox = GetPlayerHitbox(player);
            const TileBounds bounds = GetTileBounds(hitbox);
            const int probe_left = (int)floor((hitbox.x - probe) / TILESIZE);
            const int probe_right =
                (int)floor((hitbox.x + hitbox.width + probe) / TILESIZE);

            for (int y = bounds.top; y <= bounds.bottom; ++y) {
                if ((probe_left < bounds.left &&
                     IsColliding(world.current_level, probe_left, y) == 1)) {
                    player.touches_wall = -1;
                    break;

                } else if ((probe_right > bounds.right &&
                            IsColliding(world.current_level, probe_right, y) ==
                                1)) {
                    player.touches_wall = 1;
                    break;
                }
            }
        }

        if (player.touches_wall != 0) {
            player.wall_coyote_timer = player.wall_coyote_time;
        } else {
            if (player.wall_coyote_timer > 0.0f) {
                player.wall_coyote_timer -= dt;
            }
        }

        if (input.jump_pressed && player.state != DEAD) {
            if (player.grounded) {
                player.vel.y = -player.jump_velocity;
                player.grounded = false;
            } else if (!player.grounded && (player.touches_wall ||
                                            player.wall_coyote_timer > 0.0f)) {
                if (player.wall_cooldown_timer <= 0.0f) {
                    player.vel.y = -player.jump_velocity * 0.9f;
                    player.vel.x = -player.jump_velocity * player.touches_wall;
                    player.wall_cooldown_timer = player.wall_cooldown_time;
                }
            }
        }

        if (player.wall_cooldown_timer > 0.0f) {
            player.wall_cooldown_timer -= dt;
        }

        // Vertical movement and collision
        player.grounded = false;

        if (player.vel.y >= 0) {
            player.vel.y += player.fall_gravity * dt;
        } else {
            player.vel.y += player.jump_gravity * dt;
        }

        bool has_gravity = true;
        if (has_gravity && player.state != DEAD) {
            player.vel.y = std::min(player.vel.y, player.max_gravity);
            float dy = player.vel.y * dt;
            player.pos.y += dy;

            // Vertical collision check
            const Rectangle hitbox = GetPlayerHitbox(player);
            const TileBounds bounds = GetTileBounds(hitbox);
            for (int y = bounds.top; y <= bounds.bottom; ++y) {
                for (int x = bounds.left; x <= bounds.right; ++x) {
                    // Check grid bounds
                    const int collision_id =
                        IsColliding(world.current_level, x, y);
                    if (collision_id == 1) {
                        if (dy > 0.0f) {
                            player.pos.y = y * TILESIZE - player.body.offset.y -
                                           player.body.size.y;
                            player.vel = Vector2Zero();
                            player.grounded = true;
                        } else if (dy < 0.0f) {
                            player.pos.y =
                                (y + 1) * TILESIZE - player.body.offset.y;
                        }
                        player.vel.y = 0.0f;
                    } else if (collision_id == 2) {
                        if (dy > 0.0f) {
                            const float previous_bottom =
                                player.pos.y - dy + TILESIZE;

                            const float tile_top = y * TILESIZE;

                            if (previous_bottom <= tile_top) {
                                player.pos.y = y * TILESIZE -
                                               player.body.offset.y -
                                               player.body.size.y;
                                player.vel = Vector2Zero();
                                player.grounded = true;
                            }
                        }
                    }
                }
            }
        }

        // Update player state
        if (player.state == DEAD) {
            player.respawn_timer -= dt;

            if (player.respawn_timer <= 0) {
                // Respawn the player
                player.pos = player.last_entry;
                player.vel = Vector2Zero();
                player.state = IDLING;
            }
        } else if (player.grounded) {
            player.state = (input.move_x == 0) ? IDLING : WALKING;
        } else {
            player.state = (player.vel.y > 0) ? FALLING : JUMPING;
        }

        if (player.vel.x > 0 || dx > 0) {
            player.facing = RIGHT;
        } else if (player.vel.x < 0 || dx < 0) {
            player.facing = LEFT;
        }

        // Entity Collision
        if (player.state != DEAD) {
            if (!player.has_key) {
                Rectangle key_rect = {world.current_level->key.x,
                                      world.current_level->key.y + 4.0f, 8.0f,
                                      3.0f};
                if (CheckCollisionRecs(GetPlayerHitbox(player), key_rect)) {
                    player.has_key = true;
                }
            }

            for (const Spike &spike : world.current_level->spikes) {
                if (CollisionRectSpike(GetPlayerHurtbox(player), spike)) {
                    player.deaths += 1;
                    player.state = DEAD;
                    player.respawn_timer = player.respawn_time;
                }
            }

            auto &coins = world.current_level->coins; // vector type
            for (auto it = coins.begin(); it != coins.end();) {
                const Coin &coin = *it;

                Vector2 coin_center = {coin.pos.x + 4.0f, coin.pos.y + 4.0f};

                if (CheckCollisionCircleRec(coin_center, coin.r,
                                            GetPlayerHurtbox(player))) {
                    it = coins.erase(it); // Erase and get the next iterator
                    player.coins += 1;
                } else {
                    ++it;
                }
            }

            // Logic to teleport when stepping on door
            if (player.has_key) {
                Rectangle player_rect = {player.pos.x, player.pos.y, TILESIZE,
                                         TILESIZE};
                for (const Door &door : world.current_level->doors) {
                    if (CheckCollisionRecs(player_rect, door.hitbox)) {
                        if (door.target_level == "Finish") {
                            game_finished = true;
                            break;
                        }
                        for (Level &new_level : world.levels) {
                            if (door.target_level == new_level.id) {
                                world.current_level = &new_level;
                                player.pos = door.target_pos;
                                player.last_entry = door.target_pos;
                                player.has_key = false;
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        }

        // Move and Clamp Camera
        float half_view_x = screenWidth / (2.0f * cam.zoom);
        float half_view_y = screenHeight / (2.0f * cam.zoom);
        float camera_width = screenWidth / cam.zoom;
        float camera_height = screenHeight / cam.zoom;

        if (world.current_level->collision_width * TILESIZE < camera_width) {
            cam.target.x =
                world.current_level->world_pos.x +
                (world.current_level->collision_width * TILESIZE / 2.0f);
        } else {
            cam.target.x =
                Clamp(player.pos.x + TILESIZE / 2.0f,
                      world.current_level->world_pos.x + half_view_x,
                      world.current_level->world_pos.x +
                          world.current_level->world_size.x - half_view_x);
        }
        if (world.current_level->collision_height * TILESIZE < camera_height) {
            cam.target.y =
                world.current_level->world_pos.y +
                (world.current_level->collision_height * TILESIZE / 2.0f);
        } else {
            cam.target.y =
                Clamp(player.pos.y + TILESIZE / 2.0f,
                      world.current_level->world_pos.y + half_view_y,
                      world.current_level->world_pos.y +
                          world.current_level->world_size.y - half_view_y);
        }

        BeginDrawing();
        ClearBackground({24, 26, 40, 255});

        BeginMode2D(cam);
        // local Rectangles we use for drawing
        Rectangle src = {0.0f, 0.0f, TILESIZE, TILESIZE};
        Rectangle dest = {0.0f, 0.0f, TILESIZE, TILESIZE};

        for (int i = 0; i < world.current_level->tiles.size(); ++i) {
            Tile tile = world.current_level->tiles[i];

            src.x = tile.src_pos.x;
            src.y = tile.src_pos.y;
            dest.x = tile.world_pos.x;
            dest.y = tile.world_pos.y;
            DrawTexturePro(texture, src, dest, {0, 0}, 0.0f, WHITE);
        }

        if (player.has_key) {
            // Draw only door
            src.x = 88.0f;
            src.y = 56.0f;
            dest.x = world.current_level->doors[0].hitbox.x;
            dest.y = world.current_level->doors[0].hitbox.y;
            DrawTexturePro(texture, src, dest, {0, 0}, 0.0f, WHITE);
        } else {
            // Draw door and key
            src.x = 80.0f;
            src.y = 56.0f;
            dest.x = world.current_level->doors[0].hitbox.x;
            dest.y = world.current_level->doors[0].hitbox.y;
            DrawTexturePro(texture, src, dest, {0, 0}, 0.0f, WHITE);

            src.x = 48.0f;
            src.y = 40.0f;
            dest.x = world.current_level->key.x;
            dest.y = world.current_level->key.y;
            DrawTexturePro(texture, src, dest, {0, 0}, 0.0f, WHITE);
        }

        for (Coin &coin : world.current_level->coins) {
            dest.x = coin.pos.x;
            dest.y = coin.pos.y;
            anim::Draw(coin.anim, texture, dt, dest);
        }

        // Draw Player
        DrawPlayer(player, texture);
        EndMode2D();

        // draw stats
        // Background
        DrawRectangle(5, 5, 220, 80, BLACK);

        // Thick outer border
        DrawRectangleLines(5, 5, 220, 80, GOLD);
        DrawRectangleLines(6, 6, 218, 78, GOLD);
        DrawRectangleLines(7, 7, 216, 76, GOLD);

        DrawText(TextFormat("Coins: %d/%d", player.coins, num_coins), 10, 10,
                 30, GOLD);
        DrawText(TextFormat("Deaths: %d", player.deaths), 10, 45, 30, RED);

        if (DEBUG) {
            rlImGuiBegin();

            ImGui::Begin("Debug");
            ImGui::Text("Frametime: %.3f ms", GetFrameTime() * 1000.0f);
            ImGui::Separator();
            ImGui::Text("Current Level: %s", (world.current_level->id).c_str());
            ImGui::End();

            rlImGuiEnd();
        }
        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}
