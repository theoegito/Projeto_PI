#include "phase2.h"

// animação do boss
static const int   BOSS_FRAMES = 2;  
static const float BOSS_FPS    = 8.0f;

static int   s_bossFrame    = 0; 
static float s_bossAnimTime = 0.0f;   

// explosão 
static const float EXP_FPS   = 4.0f;   
static const float EXP_SCALE = 120.0f;  // para deixar a explusão no tamanho do boss

// tamanho da tela 
#define VIRTUAL_W 1024
#define VIRTUAL_H 576

// desenha o background 
static void DrawFullBG(Texture2D tex){
    Rectangle src = (Rectangle){0, 0, (float)tex.width, (float)tex.height};
    Rectangle dst = (Rectangle){0, 0, (float)VIRTUAL_W, (float)VIRTUAL_H};
    DrawTexturePro(tex, src, dst, (Vector2){0,0}, 0.0f, WHITE);
}


static void FirePlayer(Phase2 *ph, Assets *a){ // disparo da bala do player 

    // cria a bala
    for (int i = 0; i < MAX_PBULLETS; i++) if (!ph->pbul[i].active) {
        ph->pbul[i].rect = (Rectangle){
            ph->player->rect.x + ph->player->rect.width - 10,
            ph->player->rect.y + 20, 16, 16
        };
        ph->pbul[i].vel  = (Vector2){ 600, 0 };
        ph->pbul[i].active = true;
        PlaySound(a->sShoot);
        break;
    }
}

static void FireBoss(Phase2 *ph){ // disparo do boss 

    // cria a bala
    for (int i = 0; i < MAX_BOSS_BULLETS; i++) if (!ph->bbul[i].active) {
        const float bw = 16.0f, bh = 16.0f;
        float targetY = ph->player->rect.y + ph->player->rect.height * 0.55f - bh * 0.5f;
        ph->bbul[i].rect = (Rectangle){ ph->boss.rect.x, targetY, bw, bh };
        ph->bbul[i].vel  = (Vector2){ -350, 0 };
        ph->bbul[i].active = true;
        break;
    }
}

// inicia a fase 2 
void Phase2_Init(Phase2 *ph, Player *p, float groundY){
    ph->player = p;
    p->hasWeapon = true;
    p->lives = 3;

    // mantém tamanho e posição atuais do boss na tela
    ph->boss.rect = (Rectangle){ 1024 - 200, groundY - 120, 120, 120 };
    ph->boss.hp = 100;
    ph->boss.shootTimer = 0;

    for (int i = 0; i < MAX_PBULLETS; i++)       ph->pbul[i].active = false;
    for (int i = 0; i < MAX_BOSS_BULLETS; i++)   ph->bbul[i].active = false;

    ph->groundY = groundY; 
    ph->win = ph->lose = false; // estado inicial do jogo  

    // zera animação do boss
    s_bossFrame = 0;
    s_bossAnimTime = 0.0f;

    // explosão
    ph->exploding = false;
    ph->expCenter = (Vector2){0,0};
    ph->expFrame  = 0;
    ph->expTime   = 0.0f;
}

void Phase2_Update(Phase2 *ph, float dt, Assets *a){
    
    if (ph->exploding){ //explosão 
        ph->expTime += dt;
        float frameTime = 1.0f / EXP_FPS;

        // frames explosão
        int frameSize = a->texExplosion.height; 
        int totalFrames = a->texExplosion.width / frameSize; 

        while (ph->expTime >= frameTime){
            ph->expTime -= frameTime;
            ph->expFrame++;
            if (ph->expFrame >= totalFrames){
                // acabou a explosão = vitória e som
                ph->exploding = false;
                ph->win = true;
                PlaySound(a->sWin);
                break;
            }
        }
        return; // congela a tela durante a cutscne da explosão
    }
    
    if (ph->win || ph->lose) return; // acaba o jogo 

    // movimentação e pulo
    const float speed = 240.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  ph->player->rect.x -= speed * dt;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) ph->player->rect.x += speed * dt;

    // mantém dimensão do player
    float keepW = ph->player->rect.width;
    float keepH = ph->player->rect.height;

    Player_UpdateRunner(ph->player, dt, ph->groundY, a->sJump); // atualiza player
    // restaura a largura e a altura 
    ph->player->rect.width  = keepW;
    ph->player->rect.height = keepH;

    // limites dentro da tela e antes do boss
    {
        const float margin = 6.0f;
        float minX = 0.0f;
        float maxScreenX = (float)VIRTUAL_W - ph->player->rect.width;
        float maxVsBoss  = ph->boss.rect.x - ph->player->rect.width - margin;
        float maxX = maxScreenX;
        if (maxVsBoss < maxX) maxX = maxVsBoss;
        if (maxX < minX) maxX = minX;

        if (ph->player->rect.x < minX) ph->player->rect.x = minX;
        if (ph->player->rect.x > maxX) ph->player->rect.x = maxX;
    }

    // só atira no chão = não atira pulando 
    if (ph->player->hasWeapon && ph->player->onGround && // só atira no chão e com arma
        (IsKeyPressed(KEY_F) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) {
        FirePlayer(ph, a);
    }

    // balas do player
    for (int i = 0; i < MAX_PBULLETS; i++) if (ph->pbul[i].active) { // atualiza apenas balas ativas 
        ph->pbul[i].rect.x += ph->pbul[i].vel.x * dt;

        if (ph->pbul[i].rect.x > VIRTUAL_W) ph->pbul[i].active = false; // caso saia da tela desativa
        if (CheckCollisionRecs(ph->pbul[i].rect, ph->boss.rect)) { // colisão com o boss 
            ph->pbul[i].active = false;
            ph->boss.hp--;

            if (ph->boss.hp <= 0 && !ph->exploding){
                // inicia explosão no centro do boss
                ph->exploding = true;
                ph->expFrame  = 0;
                ph->expTime   = 0.0f;
                ph->expCenter = (Vector2){
                    ph->boss.rect.x + ph->boss.rect.width  * 0.5f,
                    ph->boss.rect.y + ph->boss.rect.height * 0.5f
                };

                PlaySound(a->sExplosion);
            }
        }
    }

    // tiro do boss
    ph->boss.shootTimer -= dt;
    if (ph->boss.shootTimer <= 0) {
        FireBoss(ph); // tiro do boss 
        ph->boss.shootTimer = 1.2f; // tempo entre cada tiro 
    }

    // balas do boss
    for (int i = 0; i < MAX_BOSS_BULLETS; i++) if (ph->bbul[i].active) {
        ph->bbul[i].rect.x += ph->bbul[i].vel.x * dt;
        if (ph->bbul[i].rect.x + ph->bbul[i].rect.width < 0) ph->bbul[i].active = false;
        if (CheckCollisionRecs(ph->bbul[i].rect, ph->player->rect)) { // caso colida com o player
            ph->bbul[i].active = false;
            ph->player->lives--;
            PlaySound(a->sLoseLife);
            if (ph->player->lives <= 0) { ph->lose = true; PlaySound(a->sLoseGame); }
        }
    }

    // animação do boss 
    s_bossAnimTime += dt;
    const float frameTime = 1.0f / BOSS_FPS;
    while (s_bossAnimTime >= frameTime) {
        s_bossAnimTime -= frameTime;
        s_bossFrame = (s_bossFrame + 1) % BOSS_FRAMES; // muda e faz loop dos frames 
    }
}

void Phase2_Draw(Phase2 *ph, Assets *a){ //  desenha fase 2
    
    DrawFullBG(a->texBackground); 

    // linha do chão
    DrawRectangle(0, (int)ph->groundY, VIRTUAL_W, 4, DARKGRAY);

   // boss durante o jogo - antes a explosão 
    if (!ph->exploding && ph->boss.hp > 0){
        int fw = a->texBoss.width;
        int fh = a->texBoss.height / BOSS_FRAMES;
        Rectangle srcBoss = (Rectangle){ 0, s_bossFrame * fh, (float)fw, (float)fh };
        DrawTexturePro(a->texBoss, srcBoss, ph->boss.rect, (Vector2){0,0}, 0, WHITE);
    }

    // player 
    Player_Draw(ph->player, a->texPlayerWithWeapon, a->texJump, true);

    // balas player
    for (int i = 0; i < MAX_PBULLETS; i++) if (ph->pbul[i].active)
        DrawTexturePro(a->texBullet,
        (Rectangle){0,0,(float)a->texBullet.width,(float)a->texBullet.height},
        ph->pbul[i].rect, (Vector2){0,0}, 0, WHITE);

    // balas boss
    for (int i = 0; i < MAX_BOSS_BULLETS; i++) if (ph->bbul[i].active)
        DrawTexturePro(a->texBossBullet,
        (Rectangle){0,0,(float)a->texBossBullet.width,(float)a->texBossBullet.height},
        ph->bbul[i].rect, (Vector2){0,0}, 0, WHITE);


    // explosão 
    if (ph->exploding){
        int frameSize = a->texExplosion.height;  
        Rectangle src = (Rectangle){
            ph->expFrame * frameSize, 0,
            (float)frameSize, (float)frameSize
        };

        
        float s = EXP_SCALE; 
        Rectangle dst = (Rectangle){
            ph->expCenter.x - s*0.5f,
            ph->expCenter.y - s*0.5f,
            s, s
        };
        DrawTexturePro(a->texExplosion, src, dst, (Vector2){0,0}, 0, WHITE);
    }

    DrawText(TextFormat("Vidas: %d", ph->player->lives), 20, 20, 28, WHITE);
    DrawText(TextFormat("Boss HP: %d", ph->boss.hp), 20, 56, 28, WHITE);
}