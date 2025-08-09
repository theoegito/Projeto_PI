// src/player.h
#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "assets.h"

// Estrutura do jogador
typedef struct {
    Vector2 pos;
    Vector2 size;
    Vector2 vel;       // velocidade (x,y) usada pra pulo
    bool onGround;
    bool hasWeapon;    // se já coletou arma
    int lives;         // vidas na luta contra boss (3)
    Rectangle bounds;  // caixa de colisão
} Player;

void InitPlayer(Player *p);
void UpdatePlayerLevel1(Player *p, Assets *a, float dt); // runner
void DrawPlayer(Player *p, Assets *a);
void UpdatePlayerLevel2(Player *p, Assets *a, float dt); // boss fight
Rectangle PlayerGetBounds(Player *p);

#endif // PLAYER_H
