// src/phase2.h
#ifndef PHASE2_H
#define PHASE2_H

#include "raylib.h"
#include "assets.h"
#include "player.h"

#define MAX_PROJECTILES 32
#define MAX_BOSS_PROJECTILES 20

typedef struct {
    Vector2 pos;
    Vector2 vel;
    bool active;
    Rectangle rect;
} Projectile;

typedef struct {
    Rectangle rect;
    int hp;
    float attackTimer;
    Projectile bossProjectiles[MAX_BOSS_PROJECTILES];
    Projectile playerProjectiles[MAX_PROJECTILES];
} Phase2;

void InitPhase2(Phase2 *p);
void UpdatePhase2(Phase2 *p, Player *player, Assets *a, float dt);
void DrawPhase2(Phase2 *p, Player *player, Assets *a);
void ResetPhase2(Phase2 *p);

#endif // PHASE2_H
