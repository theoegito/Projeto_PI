#include "phase2.h"

static void FirePlayer(Phase2 *ph, Assets *a){
    for(int i=0;i<MAX_PBULLETS;i++) if(!ph->pbul[i].active){
        ph->pbul[i].rect = (Rectangle){ph->player->rect.x + ph->player->rect.width-10, ph->player->rect.y+20, 16, 16};
        ph->pbul[i].vel = (Vector2){600,0};
        ph->pbul[i].active = true;
        PlaySound(a->sShoot);
        break;
    }
}

static void FireBoss(Phase2 *ph){
    for(int i=0;i<MAX_BOSS_BULLETS;i++) if(!ph->bbul[i].active){
        ph->bbul[i].rect = (Rectangle){ph->boss.rect.x, ph->boss.rect.y+30, 16, 16};
        ph->bbul[i].vel = (Vector2){-350,0};
        ph->bbul[i].active = true;
        break;
    }
}

void Phase2_Init(Phase2 *ph, Player *p, float groundY){
    ph->player = p;
    p->hasWeapon = true;
    p->lives = 3;
    ph->boss.rect = (Rectangle){1024-200, groundY-120, 120, 120};
    ph->boss.hp = 8;
    ph->boss.shootTimer = 0;
    for(int i=0;i<MAX_PBULLETS;i++){ ph->pbul[i].active=false; }
    for(int i=0;i<MAX_BOSS_BULLETS;i++){ ph->bbul[i].active=false; }
    ph->groundY = groundY;
    ph->win = ph->lose = false;
}

void Phase2_Update(Phase2 *ph, float dt, Assets *a){
    if (ph->win || ph->lose) return;

    // simple left-right move & jump
    float speed = 240.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) ph->player->rect.x -= speed*dt;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) ph->player->rect.x += speed*dt;

    Player_UpdateRunner(ph->player, dt, ph->groundY, a->sJump);

    if (IsKeyPressed(KEY_F) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        FirePlayer(ph, a);

    // update player bullets
    for(int i=0;i<MAX_PBULLETS;i++) if(ph->pbul[i].active){
        ph->pbul[i].rect.x += ph->pbul[i].vel.x * dt;
        if (ph->pbul[i].rect.x > 1024) ph->pbul[i].active=false;
        if (CheckCollisionRecs(ph->pbul[i].rect, ph->boss.rect)){
            ph->pbul[i].active=false;
            ph->boss.hp--;
            if (ph->boss.hp <= 0){ ph->win = true; PlaySound(a->sWin); }
        }
    }

    // boss shooting
    ph->boss.shootTimer -= dt;
    if (ph->boss.shootTimer <= 0){
        FireBoss(ph);
        ph->boss.shootTimer = 1.2f;
    }

    // update boss bullets and check hit
    for(int i=0;i<MAX_BOSS_BULLETS;i++) if(ph->bbul[i].active){
        ph->bbul[i].rect.x += ph->bbul[i].vel.x * dt;
        if (ph->bbul[i].rect.x + ph->bbul[i].rect.width < 0) ph->bbul[i].active=false;
        if (CheckCollisionRecs(ph->bbul[i].rect, ph->player->rect)){
            ph->bbul[i].active=false;
            ph->player->lives--;
            PlaySound(a->sLoseLife);
            if (ph->player->lives <= 0){ ph->lose = true; PlaySound(a->sLoseGame); }
        }
    }
}

void Phase2_Draw(Phase2 *ph, Assets *a){
    DrawTexture(a->texBackground,0,0,WHITE);
    DrawRectangle(0,(int)ph->groundY,1024,4,DARKGRAY);

    // boss
    DrawTexturePro(a->texBoss,(Rectangle){0,0,(float)a->texBoss.width,(float)a->texBoss.height},ph->boss.rect,(Vector2){0,0},0,WHITE);

    // player (with weapon)
    Texture2D t = a->texPlayerWithWeapon;
    if (!ph->player->onGround) t = a->texJump;
    DrawTexturePro(t,(Rectangle){0,0,(float)t.width,(float)t.height},ph->player->rect,(Vector2){0,0},0,WHITE);

    // bullets
    for(int i=0;i<MAX_PBULLETS;i++) if(ph->pbul[i].active)
        DrawTexturePro(a->texBullet,(Rectangle){0,0,(float)a->texBullet.width,(float)a->texBullet.height},ph->pbul[i].rect,(Vector2){0,0},0,WHITE);
    for(int i=0;i<MAX_BOSS_BULLETS;i++) if(ph->bbul[i].active)
        DrawTexturePro(a->texBullet,(Rectangle){0,0,(float)a->texBullet.width,(float)a->texBullet.height},ph->bbul[i].rect,(Vector2){0,0},0,WHITE);

    DrawText(TextFormat("Vidas: %d", ph->player->lives), 20, 20, 28, WHITE);
    DrawText(TextFormat("Boss HP: %d", ph->boss.hp), 20, 56, 28, WHITE);
}
