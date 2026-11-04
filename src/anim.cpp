#include "anim.h"
#include "globals.h"

void UpdateAnimationFrame(TileAnimation &anim, float dt) {
    anim.animation_timer += dt;
    if (anim.animation_timer >= anim.frame_duration) {
        anim.animation_timer -= anim.frame_duration;

        anim.current_frame++;

        if (anim.current_frame >= anim.frames.size()) {
            anim.current_frame = 0;
        }
    }
}

void DrawAnimationFrame(TileAnimation &anim, const Texture2D &texture, float dt,
                        const Rectangle &target) {
    UpdateAnimationFrame(anim, dt);
    Rectangle src = {0.0f, 0.0f, TILESIZE, TILESIZE};
    src.x = anim.frames[anim.current_frame].x;
    src.y = anim.frames[anim.current_frame].y;

    DrawTexturePro(texture, src, target, {0, 0}, 0.0f, WHITE);
}
