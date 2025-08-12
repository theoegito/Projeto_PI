#include "player.h" 

void Player_Init(Player *p, float x, float y){ // inicia o player em x,y 
    p->rect = (Rectangle){x, y, 64, 64}; 
    p->vel  = (Vector2){0,0};
    p->onGround = true;
    p->hasWeapon = false;
    p->lives = 3;
    p->animFrame = 0;
    p->animTime  = 0.0f;
}

void Player_UpdateRunner(Player *p, float dt, float groundY, Sound sJump){ // atualiza as condições do jogo 
    const float gravity = 900.0f;
    const float jumpVel = -420.0f; // velocidade do salto 

    if (p->onGround && (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))){ // salto
        p->vel.y = jumpVel;
        p->onGround = false;
        PlaySound(sJump);
        // prepara para saltar dnv
        p->animTime = 0.0f;
        p->animFrame = 0;
    }

    // gravidade
    p->vel.y += gravity * dt;
    p->rect.y += p->vel.y * dt;

    if (p->rect.y + p->rect.height >= groundY){ // se terminou o salto 
        p->rect.y = groundY - p->rect.height; // coloca no chão 
        p->vel.y = 0;
        p->onGround = true;
    }

    // animação de corrida = ele no chão 
    if (p->onGround){
        p->animTime += dt; // aumenta o tempo para mudar os frames 
        float frameTime = 1.0f / PLAYER_ANIM_FPS; // tempo de cada frame 
        if (p->animTime >= frameTime){ // atualiza os frames em loop 
            p->animTime -= frameTime;
            p->animFrame = (p->animFrame + 1) % PLAYER_RUN_FRAMES;
        }
    }
}

void Player_Draw(Player *p, Texture2D texIdle, Texture2D texJump, bool drawWithWeapon){ // desenha o player
    // largura e altura de cd frame
    const int fw = texIdle.width  / PLAYER_COLS;
    const int fh = texIdle.height / PLAYER_ROWS;

    if (!p->onGround){ // caso esteja no ar = sprite de pulo 
        
        const float scale = p->rect.width / (float)fw;

        // ajusta a sprite para se manter na escala 
        float jumpW = texJump.width  * scale;
        float jumpH = texJump.height * scale;

        Rectangle dst = { // retangulo que recebe a sprite de pulo 
            p->rect.x + (p->rect.width  - jumpW) * 0.5f,
            p->rect.y + (p->rect.height - jumpH),   // encosta no chão
            jumpW, jumpH
        };

        DrawTexturePro( // desenha o pulo 
            texJump,
            (Rectangle){0,0,(float)texJump.width,(float)texJump.height},
            dst, (Vector2){0,0}, 0, WHITE
        );
        return; // não corre durante o salto 
    }

    // frames de corrida:
    // recorta apenas o frame atual da spritesheet
    int col = p->animFrame % PLAYER_COLS;
    int row = p->animFrame / PLAYER_COLS;

    Rectangle src = { (float)(col * fw), (float)(row * fh), (float)fw, (float)fh };
    DrawTexturePro(texIdle, src, p->rect, (Vector2){0,0}, 0, WHITE); // desenha o frame atual 
} 
