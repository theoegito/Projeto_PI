#include "assets.h"

void Assets_Load(Assets *a){
    a->texBackground      = LoadTexture("assets/images/background.png");
    a->texHome            = LoadTexture("assets/images/home_screen.png");
    a->texCredits         = LoadTexture("assets/images/credits.png");
    a->texWin             = LoadTexture("assets/images/win_game.png");
    a->texLose            = LoadTexture("assets/images/lose_game.png");

    // Deixe os traços do pixel art sem blur ao escalar
    SetTextureFilter(a->texBackground, TEXTURE_FILTER_POINT);
    SetTextureFilter(a->texHome,       TEXTURE_FILTER_POINT);
    SetTextureFilter(a->texCredits,    TEXTURE_FILTER_POINT);
    SetTextureFilter(a->texWin,        TEXTURE_FILTER_POINT);
    SetTextureFilter(a->texLose,       TEXTURE_FILTER_POINT);

    a->texPlayer          = LoadTexture("assets/images/player.png");
    a->texPlayerWithWeapon= LoadTexture("assets/images/player_with_weapon.png");
    a->texPlayerShooting  = LoadTexture("assets/images/player_shooting.png");
    a->texJump            = LoadTexture("assets/images/jump.png");
    a->texObstacle        = LoadTexture("assets/images/obstacle.png");
    a->texBoss            = LoadTexture("assets/images/boss.png");
    a->texBullet          = LoadTexture("assets/images/bullet.png");
    a->texWeapon          = LoadTexture("assets/images/weapon.png");
    a->texPickup          = LoadTexture("assets/images/pickup_weapon.png");

    a->sJump         = LoadSound("assets/sounds/jump.wav");
    a->sCollide      = LoadSound("assets/sounds/collide.wav");
    a->sShoot        = LoadSound("assets/sounds/shoot.ogg");
    a->sLoseGame     = LoadSound("assets/sounds/lose_game.wav");
    a->sLoseLife     = LoadSound("assets/sounds/lose_life.wav");
    a->sPickupWeapon = LoadSound("assets/sounds/pickup_weapon.wav");
    a->sWin          = LoadSound("assets/sounds/win.wav");

    a->mMenu   = LoadMusicStream("assets/music/menu.ogg");
    a->mPhase1 = LoadMusicStream("assets/music/phase1.ogg");
    a->mPhase2 = LoadMusicStream("assets/music/phase2.ogg");
}

void Assets_Unload(Assets *a){
    UnloadTexture(a->texBackground);
    UnloadTexture(a->texHome);
    UnloadTexture(a->texCredits);
    UnloadTexture(a->texWin);
    UnloadTexture(a->texLose);
    UnloadTexture(a->texPlayer);
    UnloadTexture(a->texPlayerWithWeapon);
    UnloadTexture(a->texPlayerShooting);
    UnloadTexture(a->texJump);
    UnloadTexture(a->texObstacle);
    UnloadTexture(a->texBoss);
    UnloadTexture(a->texBullet);
    UnloadTexture(a->texWeapon);
    UnloadTexture(a->texPickup);

    UnloadSound(a->sJump);
    UnloadSound(a->sCollide);
    UnloadSound(a->sShoot);
    UnloadSound(a->sLoseGame);
    UnloadSound(a->sLoseLife);
    UnloadSound(a->sPickupWeapon);
    UnloadSound(a->sWin);

    UnloadMusicStream(a->mMenu);
    UnloadMusicStream(a->mPhase1);
    UnloadMusicStream(a->mPhase2);
}
