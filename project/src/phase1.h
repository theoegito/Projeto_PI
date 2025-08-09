// src/phase1.h
#ifndef PHASE1_H
#define PHASE1_H

#include "raylib.h"
#include "assets.h"
#include "player.h"

typedef struct {
    Rectangle rect;
    bool active;
    float speed;
} Obstacle;

#define MAX_OBS 20

typedef struct {
    Obstacle obs[MAX_OBS];
    float spawnTimer;
    int score;      // opcional
    float elapsed;  // tempo decorrido na fase
    bool finished;  // se completou 1 minuto
} Phase1;

void InitPhase1(Phase1 *p);
void UpdatePhase1(Phase1 *p, Player *player, Assets *a, float dt);
void DrawPhase1(Phase1 *p, Player *player, Assets *a);
void ResetPhase1(Phase1 *p);

#endif // PHASE1_H
