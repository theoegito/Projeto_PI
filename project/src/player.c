// src/player.c
#include "player.h"
#include <stdio.h>

#define GROUND_Y (420)
#define GRAVITY 2000.0f

void InitPlayer(Player *p) {
    p->pos = (Vector2){ 100, GROUND_Y };
    p->size = (Vector2){ 64, 64 };
    p->vel = (Vector2){ 0, 0 };
    p->onGround = true;
    p->hasWeapon = false;
    p->lives = 3;
    p->bounds = (Rectangle){ p->pos.x, p->pos.y, p->size.x, p->size.y };
}

// Função para obter bounds atual
Rectangle PlayerGetBounds(Player *p) {
    return (Rectangle){ p->pos.x, p->pos.y, p->size.x, p->size.y };
}

// Atualiza o player na fase 1 (runner).
// - Corre para frente automaticamente.
// - Pular: tecla SPACE.
// - Se colidir com obstáculo é tratado na lógica da fase1 (ela checa colisões).
void UpdatePlayerLevel1(Player *p, Assets *a, float dt) {
    const float runSpeed = 220.0f; // velocidade de corrida da fase 1

    // Movimento automático para frente (estilo runner)
    p->pos.x += runSpeed * dt;

    // Pulo
    if (IsKeyPressed(KEY_SPACE) && p->onGround) {
        p->vel.y = -700.0f; // força do pulo
        p->onGround = false;
        PlaySound(a->sfxJump); // som de pulo
    }

    // gravidade
    p->vel.y += GRAVITY * dt;
    p->pos.y += p->vel.y * dt;

    // chão
    if (p->pos.y >= GROUND_Y) {
        p->pos.y = GROUND_Y;
        p->vel.y = 0;
        p->onGround = true;
    }

    // atualizar bounds
    p->bounds = PlayerGetBounds(p);
}

// Desenha o jogador (usa sprite com arma se já coletou)
void DrawPlayer(Player *p, Assets *a) {
    if (p->hasWeapon) {
        DrawTexture(a->playerWeapon, (int)p->pos.x, (int)p->pos.y, WHITE);
    } else {
        DrawTexture(a->player, (int)p->pos.x, (int)p->pos.y, WHITE);
    }
}

// Atualiza o player na fase 2 (boss fight).
// - Movimentação livre horizontal com A/D.
// - Pulo com SPACE.
// - Atira com tecla F (ou SPACE duplicado se preferir).
void UpdatePlayerLevel2(Player *p, Assets *a, float dt) {
    const float moveSpeed = 260.0f;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) p->pos.x -= moveSpeed * dt;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) p->pos.x += moveSpeed * dt;

    // Limites na tela
    if (p->pos.x < 0) p->pos.x = 0;
    if (p->pos.x + p->size.x > 1024) p->pos.x = 1024 - p->size.x;

    // Pulo
    if (IsKeyPressed(KEY_SPACE) && p->onGround) {
        p->vel.y = -680.0f;
        p->onGround = false;
        PlaySound(a->sfxJump);
    }

    // gravidade
    p->vel.y += GRAVITY * dt;
    p->pos.y += p->vel.y * dt;
    if (p->pos.y >= GROUND_Y) {
        p->pos.y = GROUND_Y;
        p->vel.y = 0;
        p->onGround = true;
    }

    p->bounds = PlayerGetBounds(p);
}
