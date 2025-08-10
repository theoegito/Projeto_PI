#include "player.h"

void Player_Init(Player *p, float x, float y){
    p->rect = (Rectangle){x, y, 64, 64};
    p->vel  = (Vector2){0,0};
    p->onGround = true;
    p->hasWeapon = false;
    p->lives = 3;
    p->animFrame = 0;
    p->animTime  = 0.0f;
}

void Player_UpdateRunner(Player *p, float dt, float groundY, Sound sJump){
    const float gravity = 900.0f;
    const float jumpVel = -420.0f;

    if (p->onGround && (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))){
        p->vel.y = jumpVel;
        p->onGround = false;
        PlaySound(sJump);

        //reseta animação no salto
        p->animTime = 0.0f;
        p->animFrame = 0;

    }

    // apply gravity
    p->vel.y += gravity * dt;
    p->rect.y += p->vel.y * dt;

    if (p->rect.y + p->rect.height >= groundY){
        p->rect.y = groundY - p->rect.height;
        p->vel.y = 0;
        p->onGround = true;
    }

    // animação de corrida
    if (p->onGround){
        p->animTime += dt;
        float frameTime = 1.0f / PLAYER_ANIM_FPS;
        if (p->animTime >= frameTime){
            p->animTime -= frameTime;
            p->animFrame = (p->animFrame + 1) % PLAYER_RUN_FRAMES;
        }
    }
}

void Player_Draw(Player *p, Texture2D texIdle, Texture2D texJump, bool drawWithWeapon){
    if (!p->onGround){
        // salto é um frame único separado
        DrawTexturePro(texJump,
            (Rectangle){0,0,(float)texJump.width,(float)texJump.height},
            p->rect, (Vector2){0,0}, 0, WHITE);
        return;
    }

    // recorta apenas o frame atual da spritesheet de idle/corrida
    int fw = texIdle.width  / PLAYER_COLS;
    int fh = texIdle.height / PLAYER_ROWS;

    int col = p->animFrame % PLAYER_COLS;
    int row = p->animFrame / PLAYER_COLS; // 0..(PLAYER_ROWS-1)

    Rectangle src = { (float)(col * fw), (float)(row * fh), (float)fw, (float)fh };

    DrawTexturePro(texIdle, src, p->rect, (Vector2){0,0}, 0, WHITE);
}