#ifndef PLAYER_H
#define PLAYER_H
#include "raylib.h"
#include <stdbool.h>

// estrutura player
typedef struct {
    Rectangle rect; // posição e tamanho 
    Vector2   vel; 
    bool onGround;
    bool hasWeapon; // fase 2
    int  lives; // fase 2  

    // animação
    int   animFrame;
    float animTime;
} Player;

// parametros da animação 
#define PLAYER_COLS 2
#define PLAYER_ROWS 3
#define PLAYER_RUN_FRAMES 5     
#define PLAYER_ANIM_FPS 12.0f

// chama a funções 
void Player_Init(Player *p, float x, float y); 
void Player_UpdateRunner(Player *p, float dt, float groundY, Sound sJump);
void Player_Draw(Player *p, Texture2D texIdle, Texture2D texJump, bool drawWithWeapon);

#endif // PLAYER_H
