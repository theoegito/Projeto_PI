#ifndef PHASE1_H
#define PHASE1_H

#include "raylib.h"
#include <stdbool.h>
#include "assets.h"
#include "player.h"

// estrutura obstaculo 
typedef struct {
    Rectangle rect;
    bool active;
} Obstacle;

#define MAX_OBS 64

typedef struct {
    // player e estado geral
    Player *player;
    float timeLeft;
    bool finished;
    bool failed;
    float groundY;

    // mundo do runner
    float scroll;
    float runSpeed;

    // obstaculos
    Obstacle obs[MAX_OBS];
    int obsCount;
    float nextSpawnX;

    // cutscene 
    bool  endingCutscene;   
    bool  weaponHit;        
    float cutTimer;         
    int   endStage; // 0 = forçar queda, 1 = andar pra arma, 2 = pega arma
    Rectangle weaponScreen; // posição arma
} Phase1;

// inicia fase 1 
void Phase1_Init(Phase1 *ph, Player *p, float groundY);
void Phase1_Update(Phase1 *ph, float dt, Assets *a);
void Phase1_Draw(Phase1 *ph, Assets *a);

#endif 
