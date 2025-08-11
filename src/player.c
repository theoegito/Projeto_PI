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
        // reset animação no salto
        p->animTime = 0.0f;
        p->animFrame = 0;
    }

    // gravidade
    p->vel.y += gravity * dt;
    p->rect.y += p->vel.y * dt;

    if (p->rect.y + p->rect.height >= groundY){
        p->rect.y = groundY - p->rect.height;
        p->vel.y = 0;
        p->onGround = true;
    }

    // animação de corrida (só no chão)
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
    // tamanhos base (cada quadro da sheet de corrida)
    const int fw = texIdle.width  / PLAYER_COLS;
    const int fh = texIdle.height / PLAYER_ROWS;

    if (!p->onGround){
        // -------- PULO: usa a MESMA escala de pixels dos frames de corrida --------
        // escala de pixels usada na corrida
        const float scale = p->rect.width / (float)fw;

        // tamanho que o sprite de pulo deve ter para manter a mesma escala
        float jumpW = texJump.width  * scale;
        float jumpH = texJump.height * scale;

        // desenha alinhando pela base do rect e centralizando na largura
        Rectangle dst = {
            p->rect.x + (p->rect.width  - jumpW) * 0.5f,
            p->rect.y + (p->rect.height - jumpH),   // encosta no chão
            jumpW, jumpH
        };

        DrawTexturePro(
            texJump,
            (Rectangle){0,0,(float)texJump.width,(float)texJump.height},
            dst, (Vector2){0,0}, 0, WHITE
        );
        return;
        // -------------------------------------------------------------------------
    }

    // recorta apenas o frame atual da spritesheet de idle/corrida
    int col = p->animFrame % PLAYER_COLS;
    int row = p->animFrame / PLAYER_COLS;

    Rectangle src = { (float)(col * fw), (float)(row * fh), (float)fw, (float)fh };
    DrawTexturePro(texIdle, src, p->rect, (Vector2){0,0}, 0, WHITE);
}
