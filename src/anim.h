#pragma once
#include <raylib.h>
#include <vector>

struct TileAnimation {
        std::vector<Vector2> frames;
        int current_frame = 0;
        float frame_duration = 0.15;
        float animation_timer = 0.0f;
};

void UpdateAnimationFrame(TileAnimation &anim, float dt);
void DrawAnimationFrame(TileAnimation &anim, const Texture2D &texture, float dt,
                        const Rectangle &target);
