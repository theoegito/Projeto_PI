#ifndef PLAYER_H
#define PLAYER_H
#include "raylib.h"
#include <stdbool.h>

typedef struct {
    Rectangle rect;   // position/size
    Vector2   vel;    // velocity (y used for jump)
    bool onGround;
    bool hasWeapon;
    int  lives;       // used in Phase 2

    //animação
    int   animFrame;
    float animTime;
} Player;

#define PLAYER_COLS 2
#define PLAYER_ROWS 3
#define PLAYER_RUN_FRAMES 5     // quantos frames usar para correr
#define PLAYER_ANIM_FPS 12.0f    // velocidade da animação (frames/seg)

void Player_Init(Player *p, float x, float y);
void Player_UpdateRunner(Player *p, float dt, float groundY, Sound sJump);
void Player_Draw(Player *p, Texture2D texIdle, Texture2D texJump, bool drawWithWeapon);

#endif // PLAYER_H
