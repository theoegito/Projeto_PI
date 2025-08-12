#ifndef PHASE2_H
#define PHASE2_H
#include "raylib.h"
#include <stdbool.h>
#include "player.h"
#include "assets.h"

// maximo de balas simultaneas 
#define MAX_PBULLETS 32
#define MAX_BOSS_BULLETS 32

// estrutura bala
typedef struct { 
    Rectangle rect; 
    Vector2 vel; 
    bool active; 
} Bullet;

// estrutura boss
typedef struct {
    Rectangle rect;
    int hp;
    float shootTimer;
} Boss;

// estrutura da fase 2 
typedef struct {
    Player *player;
    Boss boss;
    Bullet pbul[MAX_PBULLETS];
    Bullet bbul[MAX_BOSS_BULLETS];
    float groundY;
    bool win;
    bool lose;

    // explosão 
    bool   exploding;     
    Vector2 expCenter;  
    int    expFrame;
    float  expTime;
} Phase2;

// inicia e atualiza a fase 
void Phase2_Init(Phase2 *ph, Player *p, float groundY);
void Phase2_Update(Phase2 *ph, float dt, Assets *a);
void Phase2_Draw(Phase2 *ph, Assets *a);

#endif
