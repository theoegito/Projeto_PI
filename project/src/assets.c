// src/assets.c
#include "assets.h"
#include <stdio.h>

void LoadAssets(Assets *a) {
    // Altere os caminhos conforme seus arquivos em /assets/
    a->player = LoadTexture("assets/images/player.png");
    a->playerWeapon = LoadTexture("assets/images/player_with_weapon.png");
    a->obstacle = LoadTexture("assets/images/obstacle.png");
    a->weapon = LoadTexture("assets/images/weapon.png");
    a->boss = LoadTexture("assets/images/boss.png");

    a->sfxJump = LoadSound("assets/sounds/jump.wav");
    a->sfxCollide = LoadSound("assets/sounds/collide.wav");
    a->sfxShoot = LoadSound("assets/sounds/shoot.wav");
    a->sfxLoseGame = LoadSound("assets/sounds/lose_game.wav");
    a->sfxLoseLife = LoadSound("assets/sounds/lose_life.wav");
    a->sfxPickup = LoadSound("assets/sounds/pickup_weapon.wav");
    a->sfxWin = LoadSound("assets/sounds/win.wav");

    a->musicMenu = LoadMusicStream("assets/music/menu.ogg");
    a->musicPhase1 = LoadMusicStream("assets/music/phase1.ogg");
    a->musicPhase2 = LoadMusicStream("assets/music/phase2.ogg");

    // set volume default se quiser
    SetMusicVolume(a->musicMenu, 0.7f);
    SetMusicVolume(a->musicPhase1, 0.7f);
    SetMusicVolume(a->musicPhase2, 0.7f);
}

void UnloadAssets(Assets *a) {
    UnloadTexture(a->player);
    UnloadTexture(a->playerWeapon);
    UnloadTexture(a->obstacle);
    UnloadTexture(a->weapon);
    UnloadTexture(a->boss);

    UnloadSound(a->sfxJump);
    UnloadSound(a->sfxCollide);
    UnloadSound(a->sfxShoot);
    UnloadSound(a->sfxLoseGame);
    UnloadSound(a->sfxLoseLife);
    UnloadSound(a->sfxPickup);
    UnloadSound(a->sfxWin);

    UnloadMusicStream(a->musicMenu);
    UnloadMusicStream(a->musicPhase1);
    UnloadMusicStream(a->musicPhase2);
}
