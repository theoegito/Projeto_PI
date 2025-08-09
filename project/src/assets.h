// src/assets.h
#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"

typedef struct {
    // Texturas
    Texture2D player;
    Texture2D playerWeapon;
    Texture2D obstacle;
    Texture2D weapon;
    Texture2D boss;
    // Sons
    Sound sfxJump;
    Sound sfxCollide;
    Sound sfxShoot;
    Sound sfxLoseGame;
    Sound sfxLoseLife;
    Sound sfxPickup;
    Sound sfxWin;
    // Músicas
    Music musicMenu;
    Music musicPhase1;
    Music musicPhase2;
} Assets;

void LoadAssets(Assets *a);
void UnloadAssets(Assets *a);

#endif // ASSETS_H
