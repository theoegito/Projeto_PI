#include "phase1.h"
#include <stdlib.h>

static void SpawnObstacle(Phase1 *ph){
    for(int i=0;i<MAX_OBS;i++){
        if(!ph->obs[i].active){
            float size = 48 + GetRandomValue(0,16);
            ph->obs[i].rect = (Rectangle){1024 + GetRandomValue(0,200), ph->groundY - size, size, size};
            ph->obs[i].speed = 300;
            ph->obs[i].active = true;
            break;
        }
    }
}

void Phase1_Init(Phase1 *ph, Player *p, float groundY){
    ph->player = p;
    ph->spawnTimer = 0;
    ph->timeLeft = 60.0f;
    ph->finished = false;
    ph->failed = false;
    ph->groundY = groundY;
    for(int i=0;i<MAX_OBS;i++) ph->obs[i].active=false;
}

void Phase1_Update(Phase1 *ph, float dt, Assets *a){
    if (ph->finished || ph->failed) return;

    ph->timeLeft -= dt;
    if (ph->timeLeft <= 0){ ph->finished = true; return; }

    // player jump physics
    Player_UpdateRunner(ph->player, dt, ph->groundY, a->sJump);

    // spawn obstacles
    ph->spawnTimer -= dt;
    if (ph->spawnTimer <= 0){
        SpawnObstacle(ph);
        ph->spawnTimer = 1.2f + (float)GetRandomValue(0,80)/50.0f; // 0.8s - 1.6s
    }

    // move obstacles & check collision
    for(int i=0;i<MAX_OBS;i++) if(ph->obs[i].active){
        ph->obs[i].rect.x -= ph->obs[i].speed * dt;
        if (ph->obs[i].rect.x + ph->obs[i].rect.width < -10) ph->obs[i].active=false;
        if (CheckCollisionRecs(ph->obs[i].rect, ph->player->rect)){
            ph->failed = true;
            PlaySound(a->sCollide);
            PlaySound(a->sLoseGame);
        }
    }
}

void Phase1_Draw(Phase1 *ph, Assets *a){
    // background
    DrawTexture(a->texBackground,0,0,WHITE);

    // ground line
    DrawRectangle(0,(int)ph->groundY,1024,4, DARKGRAY);

    // player
    Player_Draw(ph->player, a->texPlayer, a->texJump, false);

    // obstacles
    for(int i=0;i<MAX_OBS;i++) if(ph->obs[i].active)
        DrawTexturePro(a->texObstacle,(Rectangle){0,0,(float)a->texObstacle.width,(float)a->texObstacle.height},ph->obs[i].rect,(Vector2){0,0},0,WHITE);

    // timer
    DrawText(TextFormat("Tempo: %.0f", ph->timeLeft), 20, 20, 28, WHITE);
}
