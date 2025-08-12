#include "phase1.h"
#include <stdlib.h>

// tamanho da tela 
#define VIRTUAL_W 1024
#define VIRTUAL_H 576

// parâmetros de espaçamento
#define GAP_MIN_START 340
#define GAP_MAX_START 460
#define GAP_MIN_END   260
#define GAP_MAX_END   360

// tamanho dos obstáculos
#define SIZE_MIN 40
#define SIZE_MAX 56

// distância da geração dos obstáculos no canto da tela
#define SPAWN_AHEAD 600

// parâmetros da cutscene 
#define WEAPON_W0     120.0f  // largura da arma
#define PICKUP_HOLD   0.65f   // tempo parado após coletar a arma
#define FALL_GRAVITY  1400.0f // gravidade reforçada para cair rápido
#define WALK_SPEED    160.0f  // velocidade do player caminhando

// animação de corrida 
#define RUN_FRAMES       5
#define RUN_FRAME_TIME   0.10f 

static inline int rr(int a, int b){ return GetRandomValue(a,b); } // gera um valor aleatório para o tamanho e as distância entre os objetos
static inline float clamp01(float x){ return x<0?0:(x>1?1:x); } // limita o valor entre 0 e 1
static inline float lerp(float a,float b,float t){ return a + (b-a)*t; } // reduz aos poucos a distância entre os objetos

static float fclamp(float x, float a, float b){ return x < a ? a : (x > b ? b : x); } // limita o valor entre a e b

// gera um obstáculo novo em x
static void SpawnOne(Phase1 *ph, float gapMin, float gapMax){
    if (ph->obsCount >= MAX_OBS) return;

    float size = (float)rr(SIZE_MIN, SIZE_MAX); //define o tamanho de forma aleatória
    float x    = ph->nextSpawnX; //define o local do próximo objerto gerado
    ph->obs[ph->obsCount].rect   = (Rectangle){ x, ph->groundY - size, size, size }; // cria e registra o objeto
    ph->obs[ph->obsCount].active = true; // define o obstáculo como ativo no jogo
    ph->obsCount++; // aumenta o contador de obstáculos
    ph->nextSpawnX = x + (float)rr((int)gapMin, (int)gapMax); // define o próximo x com valor aleatório
}

// caso não carregue a imagem da arma
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

// avança a animação da corrida do player
static void TickRunAnim(Player *p, float dt){
    p->animTime += dt;
    while (p->animTime >= RUN_FRAME_TIME){ //verifica se já pode passar para o próximo frame    
        p->animTime -= RUN_FRAME_TIME;
        p->animFrame = (p->animFrame + 1) % RUN_FRAMES;
    }
}

// inicia a cutscene de captura da arma
static void StartPickupCutscene(Phase1 *ph, Assets *a){
    ph->endingCutscene = true;
    ph->weaponHit      = false;
    ph->cutTimer       = 0.0f;
    ph->endStage       = 0; // força a queda do player

    // reseta animação pra começar do primeiro quadro de corrida
    ph->player->animTime  = 0.0f;
    ph->player->animFrame = 0;

    // define tamanho da arma mantendo a proporção original da imagem
    float w = WEAPON_W0;
    float h;
    if (a && a->texWeapon.id != 0 && a->texWeapon.width > 0 && a->texWeapon.height > 0){ //verifica se uma imagem pra arma foi carregada
        float aspect = (float)a->texWeapon.height / (float)a->texWeapon.width; //calcula a proporção
        h = w * aspect; 
    } else {
        h = WEAPON_W0 * (24.0f/84.0f); // tamanho caso não haja imagem carregada
    }

    // posiciona a arma um pouco à frente do player e no chão
    float minX = ph->player->rect.x + 160.0f;
    float maxX = (float)VIRTUAL_W - w - 40.0f;
    float x    = fclamp(ph->player->rect.x + 280.0f, minX, maxX);

    ph->weaponScreen = (Rectangle){
        x,                       // x fixo para deixar arma parada
        ph->groundY - h,         // y no chão para deixar a arma chão
        w, h
    };

    ph->player->vel.x   = 0.0f; // zera a velocidade da física do player para não atrapalhar a cutscene
}

void Phase1_Init(Phase1 *ph, Player *p, float groundY){ //inicializa a fase 1
    ph->player   = p;
    ph->timeLeft = 30.0f;
    ph->finished = false;
    ph->failed   = false;
    ph->groundY  = groundY;

    ph->scroll   = 0.0f;
    ph->runSpeed = 300.0f;   // velocidade constante do player

    //zera o contador de obstáculos e define todos como não ativos antes de começar
    ph->obsCount = 0;
    for (int i=0;i<MAX_OBS;i++) ph->obs[i].active = false;

    ph->nextSpawnX = (float)VIRTUAL_W + 220.0f; // primeiro obstáculo nasce um pouco depois da borda direita da tela

    // gera alguns obstáculos iniciais
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

void Phase1_Update(Phase1 *ph, float dt, Assets *a){ // atualiza a fase 1
    if (ph->failed || ph->finished) return;

    // cutscene
    if (ph->endingCutscene){
        ph->cutTimer += dt;

        if (ph->endStage == 0){
            // força a queda do player
            ph->player->vel.y += FALL_GRAVITY * dt;
            ph->player->rect.y += ph->player->vel.y * dt;

            float yGroundTop = ph->groundY - ph->player->rect.height;
            if (ph->player->rect.y >= yGroundTop){
                ph->player->rect.y = yGroundTop;
                ph->player->vel.y  = 0.0f;
                ph->player->onGround = true;
                
                // leve pausa
                if (ph->cutTimer >= 0.08f){
                    ph->cutTimer = 0.0f;
                    ph->endStage = 1; // agora vai caminhar até a arma
                }
            }
            return;
        }

        if (ph->endStage == 1){
            
            // player caminha até a arma
            ph->player->onGround = true;
            ph->player->vel.y    = 0.0f;

            // anda para a direita e informa velocidade para a animação
            ph->player->rect.x += WALK_SPEED * dt;
            ph->player->vel.x   = WALK_SPEED;

            // avança a animação de corrida
            TickRunAnim(ph->player, dt);

            // limita pra o player não sair da tela
            float xMax = VIRTUAL_W - ph->player->rect.width - 8.0f;
            if (ph->player->rect.x > xMax) ph->player->rect.x = xMax;

            // colisão com a arma
            if (CheckCollisionRecs(ph->weaponScreen, ph->player->rect)){
                ph->weaponHit = true;
                ph->cutTimer  = 0.0f;
                PlaySound(a->sPickupWeapon);
                ph->endStage  = 2; // hold final
            }
            return;
        }

        if (ph->endStage == 2){
            // pausa breve e finaliza fase
            ph->player->vel.x = 0.0f;
            if (ph->cutTimer >= PICKUP_HOLD){
                ph->endingCutscene = false;
                ph->finished = true;   // libera a imagem de “Arma coletada!” no main
            }
            return;
        }

        return;
    }

    ph->timeLeft -= dt; // tempo da fase

    // inicia cutscene se a primeira fase for vencida
    if (ph->timeLeft <= 0.0f){
        StartPickupCutscene(ph, a);
        return; // para de processar obstáculos após entrar na cutscene
    }

    ph->scroll += ph->runSpeed * dt; // cenário se move dando a impressão de movimento do player

    // física de pulo do player
    float keepW = ph->player->rect.width;
    float keepH = ph->player->rect.height;
    Player_UpdateRunner(ph->player, dt, ph->groundY, a->sJump);

    //mantém o tamanho do player
    ph->player->rect.width  = keepW;
    ph->player->rect.height = keepH;

    // intervalo entre os obstáculos vão dimiuindo
    float t      = clamp01((30.0f - ph->timeLeft) / 30.0f);
    float gapMin = lerp(GAP_MIN_START, GAP_MIN_END, t);
    float gapMax = lerp(GAP_MAX_START, GAP_MAX_END, t);

    // garante que sempre haverá obstáculos na tela
    while (ph->nextSpawnX < ph->scroll + VIRTUAL_W + SPAWN_AHEAD) {
        SpawnOne(ph, gapMin, gapMax);
    }

    // colisões
    for (int i=0;i<ph->obsCount;i++) if (ph->obs[i].active){
        Rectangle screen = ph->obs[i].rect;
        screen.x -= ph->scroll;

        if (screen.x + screen.width < -50.0f){ // remove obstáculos que sairam da tela 
            ph->obs[i].active = false;
            continue;
        }

        if (CheckCollisionRecs(screen, ph->player->rect)){ // detecta colisão
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

    // obstaculos durante a fase 1 
    if (!ph->endingCutscene){ // sem obstaculas na cutscene

        for (int i=0;i<ph->obsCount;i++) if (ph->obs[i].active){
            Rectangle screen = ph->obs[i].rect;
            screen.x -= ph->scroll;
            if (screen.x > VIRTUAL_W + 50.0f) continue;

            DrawTexturePro(a->texObstacle,
                (Rectangle){0,0,(float)a->texObstacle.width,(float)a->texObstacle.height},
                screen, (Vector2){0,0}, 0, WHITE);
        }
    }

    // cutscene 
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
        // cronometro 
        DrawText(TextFormat("Tempo: %.0f", ph->timeLeft), 20, 20, 28, WHITE);
    }
}
