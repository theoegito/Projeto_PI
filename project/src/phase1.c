// src/phase1.c
#include "phase1.h"
#include <stdlib.h>
#include <stdio.h>

#define PHASE1_DURATION 60.0f // 1 minuto

void InitPhase1(Phase1 *p) {
    ResetPhase1(p);
}

void ResetPhase1(Phase1 *p) {
    for (int i=0;i<MAX_OBS;i++) {
        p->obs[i].active = false;
    }
    p->spawnTimer = 0.0f;
    p->score = 0;
    p->elapsed = 0.0f;
    p->finished = false;
}

// Função para criar obstáculo em um slot livre
static void SpawnObstacleAt(Phase1 *p, float x) {
    for (int i=0;i<MAX_OBS;i++){
        if (!p->obs[i].active){
            p->obs[i].active = true;
            p->obs[i].rect = (Rectangle){ x, 420, 64, 64 };
            p->obs[i].speed = 280.0f + (float)(GetRandomValue(0,120));
            break;
        }
    }
}

void UpdatePhase1(Phase1 *p, Player *player, Assets *a, float dt) {
    p->elapsed += dt;

    // spawn periódico (intervalo reduzido conforme tempo)
    p->spawnTimer += dt;
    float interval = 1.0f; // 1s por padrão
    if (p->spawnTimer > interval) {
        p->spawnTimer = 0;
        // spawn no lado direito da tela
        SpawnObstacleAt(p, 1024 + GetRandomValue(0, 200));
    }

    // atualizar obstáculos
    for (int i=0;i<MAX_OBS;i++){
        if (!p->obs[i].active) continue;
        p->obs[i].rect.x -= p->obs[i].speed * dt;
        // se saiu da tela, desativa
        if (p->obs[i].rect.x + p->obs[i].rect.width < 0) p->obs[i].active = false;

        // checar colisão com jogador
        if (CheckCollisionRecs(p->obs[i].rect, PlayerGetBounds(player))) {
            // colisão -> game over fase1
            PlaySound(a->sfxCollide);
            player->lives = 0; // sinaliza derrota
            p->finished = false;
            return;
        }
    }

    // atualizar player
    UpdatePlayerLevel1(player, a, dt);

    // se passou 60s sem colisão -> fase concluída
    if (p->elapsed >= PHASE1_DURATION) {
        p->finished = true;
    }
}

void DrawPhase1(Phase1 *p, Player *player, Assets *a) {
    // chão
    DrawRectangle(0, 480, 1024, 96, LIGHTGRAY);

    // desenhar player
    DrawPlayer(player, a);

    // desenhar obstáculos
    for (int i=0;i<MAX_OBS;i++){
        if (!p->obs[i].active) continue;
        DrawTexture(a->obstacle, (int)p->obs[i].rect.x, (int)p->obs[i].rect.y, WHITE);
    }

    // HUD tempo
    char buf[64];
    int remaining = (int)(PHASE1_DURATION - p->elapsed);
    if (remaining < 0) remaining = 0;
    sprintf(buf, "Tempo restante: %d s", remaining);
    DrawText(buf, 10, 10, 20, BLACK);
}
