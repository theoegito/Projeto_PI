// src/phase2.c
#include "phase2.h"
#include <stdio.h>

// inicializa projéteis
static void ResetProjectiles(Phase2 *p) {
    for (int i=0;i<MAX_PROJECTILES;i++){
        p->playerProjectiles[i].active = false;
    }
    for (int i=0;i<MAX_BOSS_PROJECTILES;i++){
        p->bossProjectiles[i].active = false;
    }
}

void InitPhase2(Phase2 *p) {
    p->rect = (Rectangle){ 700, 300, 200, 200 }; // posição do boss
    p->hp = 100;
    p->attackTimer = 0.0f;
    ResetProjectiles(p);
}

void ResetPhase2(Phase2 *p) {
    InitPhase2(p);
}

// atira projétil do jogador
static void FirePlayerProjectile(Phase2 *p, Vector2 pos) {
    for (int i=0;i<MAX_PROJECTILES;i++){
        if (!p->playerProjectiles[i].active){
            p->playerProjectiles[i].active = true;
            p->playerProjectiles[i].pos = pos;
            p->playerProjectiles[i].vel = (Vector2){ 600.0f, 0 };
            p->playerProjectiles[i].rect = (Rectangle){ pos.x, pos.y, 10, 6 };
            break;
        }
    }
}

// boss atira um projétil em direção ao jogador (simples: projétil direto à esquerda)
static void FireBossProjectile(Phase2 *p, Vector2 pos) {
    for (int i=0;i<MAX_BOSS_PROJECTILES;i++){
        if (!p->bossProjectiles[i].active){
            p->bossProjectiles[i].active = true;
            p->bossProjectiles[i].pos = pos;
            p->bossProjectiles[i].vel = (Vector2){ -300.0f, 0 };
            p->bossProjectiles[i].rect = (Rectangle){ pos.x, pos.y, 16, 16 };
            break;
        }
    }
}

void UpdatePhase2(Phase2 *p, Player *player, Assets *a, float dt) {
    // Atualizar ataque do boss
    p->attackTimer += dt;
    if (p->attackTimer >= 1.2f) {
        p->attackTimer = 0;
        // dispara da frente do boss
        Vector2 spawn = { p->rect.x - 20, p->rect.y + p->rect.height/2 };
        FireBossProjectile(p, spawn);
    }

    // Atualiza projéteis do jogador
    for (int i=0;i<MAX_PROJECTILES;i++){
        if (!p->playerProjectiles[i].active) continue;
        p->playerProjectiles[i].pos.x += p->playerProjectiles[i].vel.x * dt;
        p->playerProjectiles[i].rect.x = p->playerProjectiles[i].pos.x;
        if (p->playerProjectiles[i].pos.x > 1024) p->playerProjectiles[i].active = false;
        // colisão com boss
        if (CheckCollisionRecs(p->playerProjectiles[i].rect, p->rect)) {
            p->playerProjectiles[i].active = false;
            p->hp -= 10;
            PlaySound(a->sfxShoot); // som de dano (use outro se preferir)
            if (p->hp <= 0) {
                // boss morto -> win tratado no main
            }
        }
    }

    // Atualiza projéteis do boss
    for (int i=0;i<MAX_BOSS_PROJECTILES;i++){
        if (!p->bossProjectiles[i].active) continue;
        p->bossProjectiles[i].pos.x += p->bossProjectiles[i].vel.x * dt;
        p->bossProjectiles[i].rect.x = p->bossProjectiles[i].pos.x;
        // colisão com player
        if (CheckCollisionRecs(p->bossProjectiles[i].rect, PlayerGetBounds(player))) {
            p->bossProjectiles[i].active = false;
            player->lives -= 1;
            PlaySound(a->sfxLoseLife);
            if (player->lives <= 0) {
                // player perdeu todas as vidas -> tratado no main
            }
        }
        if (p->bossProjectiles[i].pos.x + p->bossProjectiles[i].rect.width < 0) p->bossProjectiles[i].active = false;
    }

    // disparo do jogador (input): tecla F para atirar
    if ((IsKeyPressed(KEY_F) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && player->hasWeapon) {
        Vector2 spawn = { player->pos.x + player->size.x, player->pos.y + player->size.y/2 };
        FirePlayerProjectile(p, spawn);
        PlaySound(a->sfxShoot);
    }
}

void DrawPhase2(Phase2 *p, Player *player, Assets *a) {
    // desenhar boss
    DrawTexture(a->boss, (int)p->rect.x, (int)p->rect.y, WHITE);

    // projéteis do jogador
    for (int i=0;i<MAX_PROJECTILES;i++){
        if (!p->playerProjectiles[i].active) continue;
        DrawRectangle((int)p->playerProjectiles[i].pos.x, (int)p->playerProjectiles[i].pos.y, 10, 6, BLACK);
    }
    // projéteis do boss
    for (int i=0;i<MAX_BOSS_PROJECTILES;i++){
        if (!p->bossProjectiles[i].active) continue;
        DrawCircle((int)p->bossProjectiles[i].pos.x, (int)p->bossProjectiles[i].pos.y, 8, RED);
    }

    // HUD boss hp
    char buf[64];
    sprintf(buf, "Boss HP: %d", p->hp);
    DrawText(buf, 800, 10, 20, MAROON);
}
