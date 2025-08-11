#include "phase1.h"
#include <stdlib.h>
// <math.h> não é necessário aqui

#define VIRTUAL_W 1024
#define VIRTUAL_H 576

// ---- parâmetros de espaçamento (pixels de MUNDO)
#define GAP_MIN_START 340
#define GAP_MAX_START 460
#define GAP_MIN_END   260
#define GAP_MAX_END   360

// tamanho dos obstáculos
#define SIZE_MIN 40
#define SIZE_MAX 56

// quanto à frente da borda direita devemos manter obstáculos pré-gerados
#define SPAWN_AHEAD 600

// cutscene (coleta no chão; player caminha até a arma)
#define WEAPON_W0     120.0f  // largura base da arma (na tela)
#define PICKUP_HOLD   0.65f   // tempo parado após coletar (s)
#define FALL_GRAVITY  1400.0f // gravidade reforçada para cair rápido
#define WALK_SPEED    160.0f  // velocidade do player caminhando (px/s)

// animação de corrida do player (spritesheet com 5 quadros)
#define RUN_FRAMES       5
#define RUN_FRAME_TIME   0.10f  // ~10 fps; ajuste se quiser

// utilitário
static inline int rr(int a, int b){ return GetRandomValue(a,b); }
static inline float clamp01(float x){ return x<0?0:(x>1?1:x); }
static inline float lerp(float a,float b,float t){ return a + (b-a)*t; }

static float fclamp(float x, float a, float b){ return x < a ? a : (x > b ? b : x); }

// gera um obstáculo novo em X de mundo = ph->nextSpawnX
static void SpawnOne(Phase1 *ph, float gapMin, float gapMax){
    if (ph->obsCount >= MAX_OBS) return;

    float size = (float)rr(SIZE_MIN, SIZE_MAX);
    float x    = ph->nextSpawnX;
    ph->obs[ph->obsCount].rect   = (Rectangle){ x, ph->groundY - size, size, size };
    ph->obs[ph->obsCount].active = true;
    ph->obsCount++;

    // define o próximo X com gap aleatório
    ph->nextSpawnX = x + (float)rr((int)gapMin, (int)gapMax);
}

// fallback se não houver weapon.png
static void DrawPickupWeaponShape(Rectangle r){
    Vector2 c = { r.x + r.width*0.5f, r.y + r.height*0.5f };
    DrawCircleGradient((int)c.x, (int)c.y, 42.0f, Fade(YELLOW, 0.55f), Fade(BLANK, 0.0f));
    Rectangle butt   = { r.x, r.y, r.width*0.36f, r.height };
    Rectangle barrel = { r.x + r.width*0.28f, r.y + r.height*0.35f, r.width*0.68f, r.height*0.30f };
    Rectangle trig   = { r.x + r.width*0.30f, r.y + r.height*0.55f, r.width*0.10f, r.height*0.18f };
    DrawRectangleRec(butt, BROWN);
    DrawRectangleRec(barrel, DARKGRAY);
    DrawRectangleRec(trig, BLACK);
    DrawRectangleLinesEx((Rectangle){r.x-1, r.y-1, r.width+2, r.height+2}, 2, Fade(BLACK, 0.55f));
}

// tick simples da animação de corrida (5 quadros)
static void TickRunAnim(Player *p, float dt){
    p->animTime += dt;
    while (p->animTime >= RUN_FRAME_TIME){
        p->animTime -= RUN_FRAME_TIME;
        p->animFrame = (p->animFrame + 1) % RUN_FRAMES;
    }
}

// inicia a cutscene de captura da arma (no CHÃO, com player caminhando)
static void StartPickupCutscene(Phase1 *ph, Assets *a){
    ph->endingCutscene = true;
    ph->weaponHit      = false;
    ph->cutTimer       = 0.0f;
    ph->endStage       = 0; // 0=forçar queda

    // reseta anima pra começar do 1º quadro da corrida
    ph->player->animTime  = 0.0f;
    ph->player->animFrame = 0;

    // define tamanho respeitando o aspect da weapon.png, se houver
    float w = WEAPON_W0;
    float h;
    if (a && a->texWeapon.id != 0 && a->texWeapon.width > 0 && a->texWeapon.height > 0){
        float aspect = (float)a->texWeapon.height / (float)a->texWeapon.width;
        h = w * aspect;
    } else {
        h = WEAPON_W0 * (24.0f/84.0f); // aproximação do fallback
    }

    // posiciona a arma um pouco à frente do player, já DENTRO da tela e no chão
    float minX = ph->player->rect.x + 160.0f;
    float maxX = (float)VIRTUAL_W - w - 40.0f;
    float x    = fclamp(ph->player->rect.x + 280.0f, minX, maxX);

    ph->weaponScreen = (Rectangle){
        x,                       // x fixo: arma parada
        ph->groundY - h,         // y no chão
        w, h
    };

    // garante estado do player para início da cutscene
    ph->player->vel.x   = 0.0f; // movemos o x manualmente
}

void Phase1_Init(Phase1 *ph, Player *p, float groundY){
    ph->player   = p;
    ph->timeLeft = 60.0f;
    ph->finished = false;
    ph->failed   = false;
    ph->groundY  = groundY;

    ph->scroll   = 0.0f;
    ph->runSpeed = 300.0f;   // velocidade constante do runner (px/s)

    ph->obsCount = 0;
    for (int i=0;i<MAX_OBS;i++) ph->obs[i].active = false;

    // primeiro obstáculo nasce um pouco depois da borda direita do mundo
    ph->nextSpawnX = (float)VIRTUAL_W + 220.0f;

    // gera alguns iniciais para já ter coisa na tela
    for (int i=0;i<5;i++){
        float t      = 0.0f; // início da fase
        float gapMin = lerp(GAP_MIN_START, GAP_MIN_END, t);
        float gapMax = lerp(GAP_MAX_START, GAP_MAX_END, t);
        SpawnOne(ph, gapMin, gapMax);
    }

    // cutscene desligada no início
    ph->endingCutscene = false;
    ph->weaponHit      = false;
    ph->cutTimer       = 0.0f;
    ph->endStage       = 0;
    ph->weaponScreen   = (Rectangle){0,0,0,0};
}

void Phase1_Update(Phase1 *ph, float dt, Assets *a){
    if (ph->failed || ph->finished) return;

    // ---------------- CUTSCENE ----------------
    if (ph->endingCutscene){
        ph->cutTimer += dt;

        if (ph->endStage == 0){
            // 0) Força a queda até o chão (ignora input)
            ph->player->vel.y += FALL_GRAVITY * dt;
            ph->player->rect.y += ph->player->vel.y * dt;

            float yGroundTop = ph->groundY - ph->player->rect.height;
            if (ph->player->rect.y >= yGroundTop){
                ph->player->rect.y = yGroundTop;
                ph->player->vel.y  = 0.0f;
                ph->player->onGround = true;
                // respira um instantinho
                if (ph->cutTimer >= 0.08f){
                    ph->cutTimer = 0.0f;
                    ph->endStage = 1; // agora vai caminhar até a arma
                }
            }
            return;
        }

        if (ph->endStage == 1){
            // 1) Player CAMINHA até a arma (arma parada no chão)
            ph->player->onGround = true;
            ph->player->vel.y    = 0.0f;

            // anda para a direita (visual) e informa vel.x para a animação
            ph->player->rect.x += WALK_SPEED * dt;
            ph->player->vel.x   = WALK_SPEED;

            // avança a animação de corrida (5 quadros)
            TickRunAnim(ph->player, dt);

            // opcional: limita o player pra não sair da tela
            float xMax = VIRTUAL_W - ph->player->rect.width - 8.0f;
            if (ph->player->rect.x > xMax) ph->player->rect.x = xMax;

            // colisão com a arma (no chão)
            if (CheckCollisionRecs(ph->weaponScreen, ph->player->rect)){
                ph->weaponHit = true;
                ph->cutTimer  = 0.0f;
                PlaySound(a->sPickupWeapon);
                ph->endStage  = 2; // hold final
            }
            return;
        }

        if (ph->endStage == 2){
            // 2) pausa breve e finaliza fase
            ph->player->vel.x = 0.0f;
            if (ph->cutTimer >= PICKUP_HOLD){
                ph->endingCutscene = false;
                ph->finished = true;   // libera “Arma coletada!” no main
            }
            return;
        }

        return; // segurança
    }
    // -----------------------------------------

    // tempo da fase
    ph->timeLeft -= dt;

    // se o tempo acabou (e não falhou), inicia cutscene (força queda + caminha até a arma)
    if (ph->timeLeft <= 0.0f){
        StartPickupCutscene(ph, a);
        return; // não processa obstáculos após entrar na cutscene
    }

    // avanço do “mundo” (obstáculos não se movem, só o scroll anda)
    ph->scroll += ph->runSpeed * dt;

    // física/jump do player (x do player fica fixo no runner)
    // >>> mantém dimensão fixa do jogador <<<
    float keepW = ph->player->rect.width;
    float keepH = ph->player->rect.height;
    Player_UpdateRunner(ph->player, dt, ph->groundY, a->sJump);
    ph->player->rect.width  = keepW;
    ph->player->rect.height = keepH;

    // dificuldade progressiva -> gaps vão diminuindo
    float t      = clamp01((60.0f - ph->timeLeft) / 60.0f);
    float gapMin = lerp(GAP_MIN_START, GAP_MIN_END, t);
    float gapMax = lerp(GAP_MAX_START, GAP_MAX_END, t);

    // mantenha obstáculos pré-gerados à frente da câmera
    while (ph->nextSpawnX < ph->scroll + VIRTUAL_W + SPAWN_AHEAD) {
        SpawnOne(ph, gapMin, gapMax);
    }

    // colisões (mundo->tela)
    for (int i=0;i<ph->obsCount;i++) if (ph->obs[i].active){
        Rectangle screen = ph->obs[i].rect;
        screen.x -= ph->scroll;   // mundo -> tela

        if (screen.x + screen.width < -50.0f){
            ph->obs[i].active = false;
            continue;
        }

        if (CheckCollisionRecs(screen, ph->player->rect)){
            ph->failed = true;
            PlaySound(a->sCollide);
            PlaySound(a->sLoseGame);
        }
    }
}

void Phase1_Draw(Phase1 *ph, Assets *a){
    // fundo
    DrawTexturePro(a->texBackground,
        (Rectangle){0,0,(float)a->texBackground.width,(float)a->texBackground.height},
        (Rectangle){0,0,(float)VIRTUAL_W,(float)VIRTUAL_H},
        (Vector2){0,0}, 0, WHITE);

    // chão
    DrawRectangle(0,(int)ph->groundY, VIRTUAL_W, 4, DARKGRAY);

    // player
    Player_Draw(ph->player, a->texPlayer, a->texJump, false);

    // obstáculos (omitidos durante cutscene para foco)
    if (!ph->endingCutscene){
        for (int i=0;i<ph->obsCount;i++) if (ph->obs[i].active){
            Rectangle screen = ph->obs[i].rect;
            screen.x -= ph->scroll;
            if (screen.x > VIRTUAL_W + 50.0f) continue;

            DrawTexturePro(a->texObstacle,
                (Rectangle){0,0,(float)a->texObstacle.width,(float)a->texObstacle.height},
                screen, (Vector2){0,0}, 0, WHITE);
        }
    }

    // CUTSCENE: arma (weapon.png se disponível), com leve foco
    if (ph->endingCutscene){
        DrawRectangle(0, 0, VIRTUAL_W, VIRTUAL_H, Fade(BLACK, 0.18f));

        if (a && a->texWeapon.id != 0 && a->texWeapon.width > 0 && a->texWeapon.height > 0){
            Rectangle dst = ph->weaponScreen; // fixa no chão
            float aspect = (float)a->texWeapon.height / (float)a->texWeapon.width;
            dst.height = dst.width * aspect;
            dst.y = ph->groundY - dst.height;
            // brilho
            Vector2 c = { dst.x + dst.width*0.5f, dst.y + dst.height*0.6f };
            DrawCircleGradient((int)c.x, (int)c.y, 42.0f, Fade(YELLOW, 0.55f), Fade(BLANK, 0.0f));
            DrawTexturePro(a->texWeapon, (Rectangle){0,0,(float)a->texWeapon.width,(float)a->texWeapon.height},
                           dst, (Vector2){0,0}, 0, WHITE);
        } else {
            DrawPickupWeaponShape(ph->weaponScreen);
        }
    } else {
        // HUD normal
        DrawText(TextFormat("Tempo: %.0f", ph->timeLeft), 20, 20, 28, WHITE);
    }
}
