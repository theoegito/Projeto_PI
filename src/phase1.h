#ifndef PHASE1_H
#define PHASE1_H

#include "raylib.h"
#include <stdbool.h>
#include "assets.h"
#include "player.h"

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

    // obstáculos
    Obstacle obs[MAX_OBS];
    int obsCount;
    float nextSpawnX;

    // ---- cutscene de captura da arma (no chão, player caminha) ----
    bool  endingCutscene;   // estamos na cutscene?
    bool  weaponHit;        // já colidiu (arma coletada)?
    float cutTimer;         // cronômetro interno da cutscene
    int   endStage;         // 0=forçar queda, 1=andar até a arma, 2=hold e finalizar
    Rectangle weaponScreen; // arma (coordenadas de TELA)
} Phase1;

void Phase1_Init(Phase1 *ph, Player *p, float groundY);
void Phase1_Update(Phase1 *ph, float dt, Assets *a);
void Phase1_Draw(Phase1 *ph, Assets *a);

#endif // PHASE1_H
