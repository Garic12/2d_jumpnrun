#pragma once
#include <raylib.h>
#include <vector>

namespace anim {

struct TileAnimation {
        std::vector<Vector2> frames; // top-left source position
        int current_frame = 0;
        float frame_duration = 0.15; // seconds per frame
        float animation_timer = 0.0f;
};

void Update(TileAnimation &anim, float dt);
void Draw(TileAnimation &anim, const Texture2D &texture, float dt,
          const Rectangle &target);
} // namespace anim
