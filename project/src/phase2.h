#ifndef PHASE2_H
#define PHASE2_H
#include "raylib.h"
#include <stdbool.h>
#include "player.h"
#include "assets.h"

#define MAX_PBULLETS 32
#define MAX_BOSS_BULLETS 32

typedef struct { Rectangle rect; Vector2 vel; bool active; } Bullet;

typedef struct { Rectangle rect; int hp; float shootTimer; } Boss;

typedef struct {
    Player *player;
    Boss boss;
    Bullet pbul[MAX_PBULLETS];
    Bullet bbul[MAX_BOSS_BULLETS];
    float groundY;
    bool win;
    bool lose;
} Phase2;

void Phase2_Init(Phase2 *ph, Player *p, float groundY);
void Phase2_Update(Phase2 *ph, float dt, Assets *a);
void Phase2_Draw(Phase2 *ph, Assets *a);

#endif // PHASE2_H
