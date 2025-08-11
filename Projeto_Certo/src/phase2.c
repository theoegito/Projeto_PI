#include "phase2.h"

// ----------------------
// Animação do BOSS (sem mexer no .h)
// ----------------------
static const int   BOSS_FRAMES = 2;   // dois quadros (vertical)
static const float BOSS_FPS    = 8.0f;

static int   s_bossFrame    = 0;      // quadro atual [0..BOSS_FRAMES-1]
static float s_bossAnimTime = 0.0f;   // acumulador
// ----------------------

#define VIRTUAL_W 1024
#define VIRTUAL_H 576

// desenha fundo preenchendo a tela lógica 1024x576 (mesmo estilo da fase 1)
static void DrawFullBG(Texture2D tex){
    Rectangle src = (Rectangle){0, 0, (float)tex.width, (float)tex.height};
    Rectangle dst = (Rectangle){0, 0, (float)VIRTUAL_W, (float)VIRTUAL_H};
    DrawTexturePro(tex, src, dst, (Vector2){0,0}, 0.0f, WHITE);
}

static void FirePlayer(Phase2 *ph, Assets *a){
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

// Boss mira na "cintura" do player
static void FireBoss(Phase2 *ph){
    for (int i = 0; i < MAX_BOSS_BULLETS; i++) if (!ph->bbul[i].active) {
        const float bw = 16.0f, bh = 16.0f;
        float targetY = ph->player->rect.y + ph->player->rect.height * 0.55f - bh * 0.5f;
        ph->bbul[i].rect = (Rectangle){ ph->boss.rect.x, targetY, bw, bh };
        ph->bbul[i].vel  = (Vector2){ -350, 0 };
        ph->bbul[i].active = true;
        break;
    }
}

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
    ph->win = ph->lose = false;

    // zera animação do boss
    s_bossFrame = 0;
    s_bossAnimTime = 0.0f;
}

void Phase2_Update(Phase2 *ph, float dt, Assets *a){
    if (ph->win || ph->lose) return;

    // movimento simples + pulo
    const float speed = 240.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  ph->player->rect.x -= speed * dt;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) ph->player->rect.x += speed * dt;

    // mantém dimensão do player
    float keepW = ph->player->rect.width;
    float keepH = ph->player->rect.height;
    Player_UpdateRunner(ph->player, dt, ph->groundY, a->sJump);
    ph->player->rect.width  = keepW;
    ph->player->rect.height = keepH;

    // LIMITES: dentro da tela e ANTES do boss
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

    // >>> SÓ ATIRA NO CHÃO (e com arma) <<<
    if (ph->player->hasWeapon && ph->player->onGround &&
        (IsKeyPressed(KEY_F) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) {
        FirePlayer(ph, a);
    }

    // balas do player
    for (int i = 0; i < MAX_PBULLETS; i++) if (ph->pbul[i].active) {
        ph->pbul[i].rect.x += ph->pbul[i].vel.x * dt;
        if (ph->pbul[i].rect.x > VIRTUAL_W) ph->pbul[i].active = false;
        if (CheckCollisionRecs(ph->pbul[i].rect, ph->boss.rect)) {
            ph->pbul[i].active = false;
            ph->boss.hp--;
            if (ph->boss.hp <= 0) { ph->win = true; PlaySound(a->sWin); }
        }
    }

    // tiro do boss
    ph->boss.shootTimer -= dt;
    if (ph->boss.shootTimer <= 0) {
        FireBoss(ph);
        ph->boss.shootTimer = 1.2f;
    }

    // balas do boss
    for (int i = 0; i < MAX_BOSS_BULLETS; i++) if (ph->bbul[i].active) {
        ph->bbul[i].rect.x += ph->bbul[i].vel.x * dt;
        if (ph->bbul[i].rect.x + ph->bbul[i].rect.width < 0) ph->bbul[i].active = false;
        if (CheckCollisionRecs(ph->bbul[i].rect, ph->player->rect)) {
            ph->bbul[i].active = false;
            ph->player->lives--;
            PlaySound(a->sLoseLife);
            if (ph->player->lives <= 0) { ph->lose = true; PlaySound(a->sLoseGame); }
        }
    }

    // animação do boss (8 fps)
    s_bossAnimTime += dt;
    const float frameTime = 1.0f / BOSS_FPS;
    while (s_bossAnimTime >= frameTime) {
        s_bossAnimTime -= frameTime;
        s_bossFrame = (s_bossFrame + 1) % BOSS_FRAMES;
    }
}

void Phase2_Draw(Phase2 *ph, Assets *a){
    // fundo no mesmo estilo da fase 1 (sem zoom aparente/corte)
    DrawFullBG(a->texBackground);

    // linha do chão
    DrawRectangle(0, (int)ph->groundY, VIRTUAL_W, 4, DARKGRAY);

    // Boss (recorta 1 quadro da sheet vertical)
    int fw = a->texBoss.width;
    int fh = a->texBoss.height / BOSS_FRAMES;
    Rectangle srcBoss = (Rectangle){ 0, s_bossFrame * fh, (float)fw, (float)fh };
    DrawTexturePro(a->texBoss, srcBoss, ph->boss.rect, (Vector2){0,0}, 0, WHITE);

    // Player (recorte correto)
    Player_Draw(ph->player, a->texPlayerWithWeapon, a->texJump, true);

    // Balas
    for (int i = 0; i < MAX_PBULLETS; i++) if (ph->pbul[i].active)
        DrawTexturePro(a->texBullet, (Rectangle){0,0,(float)a->texBullet.width,(float)a->texBullet.height}, ph->pbul[i].rect, (Vector2){0,0}, 0, WHITE);
    for (int i = 0; i < MAX_BOSS_BULLETS; i++) if (ph->bbul[i].active)
        DrawTexturePro(a->texBullet, (Rectangle){0,0,(float)a->texBullet.width,(float)a->texBullet.height}, ph->bbul[i].rect, (Vector2){0,0}, 0, WHITE);

    // UI
    DrawText(TextFormat("Vidas: %d", ph->player->lives), 20, 20, 28, WHITE);
    DrawText(TextFormat("Boss HP: %d", ph->boss.hp), 20, 56, 28, WHITE);
}
