#ifndef PHASE1_H
#define PHASE1_H
#include "raylib.h"
#include <stdbool.h>
#include "player.h"
#include "assets.h"

#define MAX_OBS 20

typedef struct { Rectangle rect; float speed; bool active; } Obstacle;

typedef struct {
    Player *player;
    Obstacle obs[MAX_OBS];
    float spawnTimer;
    float timeLeft;     // counts down from 60s
    bool finished;      // true if survived 60s
    bool failed;        // true on collision
    float groundY;
} Phase1;

void Phase1_Init(Phase1 *ph, Player *p, float groundY);
void Phase1_Update(Phase1 *ph, float dt, Assets *a);
void Phase1_Draw(Phase1 *ph, Assets *a);

#endif // PHASE1_H
