#ifndef ASSETS_H
#define ASSETS_H
#include "raylib.h"

typedef struct {
    // imagens
    Texture2D texBackground;
    Texture2D texHome;
    Texture2D texCredits;
    Texture2D texWin;
    Texture2D texLose;
    Texture2D texPlayer;
    Texture2D texPlayerWithWeapon;
    Texture2D texPlayerShooting;
    Texture2D texJump;
    Texture2D texObstacle;
    Texture2D texBoss;
    Texture2D texBullet;
    Texture2D texBossBullet;
    Texture2D texWeapon;
    Texture2D texPickup;
    Texture2D texExplosion;
    Texture2D texEnredo;

    // sons 
    Sound sJump;
    Sound sCollide;
    Sound sShoot;
    Sound sLoseGame;
    Sound sLoseLife;
    Sound sPickupWeapon;
    Sound sWin;
    Sound sExplosion; 

    // musicas
    Music mMenu;
    Music mPhase1;
    Music mPhase2;
} Assets;

void Assets_Load(Assets *a);
void Assets_Unload(Assets *a);

#endif // ASSETS_H
