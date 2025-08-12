#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include "assets.h"
#include "player.h"
#include "phase1.h"
#include "phase2.h"

// tamanho da tela 
#define SCREEN_W 1024
#define SCREEN_H 576

// desenha textura q preenchendo a tela 
static void DrawFull(Texture2D tex){
    Rectangle src = (Rectangle){0, 0, (float)tex.width, (float)tex.height};
    Rectangle dst = (Rectangle){0, 0, SCREEN_W, SCREEN_H};
    DrawTexturePro(tex, src, dst, (Vector2){0,0}, 0.0f, WHITE);
}

// botões:
// MENU
static Rectangle R_MENU_PLAY = { 536, 285, 243, 88 }; // jogar
static Rectangle R_MENU_EXIT = { 534, 397, 254, 88 }; // sair

// GANHOU
static Rectangle R_PLAYAGAIN_WIN = {209, 384, 518, 70}; // jogar dnv
static Rectangle R_EXIT_WIN      = { 360, 471, 218, 67 }; // sair
// PERDEU
static Rectangle R_PLAYAGAIN_LOSE = {224, 374, 487, 81}; // jogar dnv 
static Rectangle R_EXIT_LOSE      = { 347, 472, 243, 72 }; // sair


static bool gShowHotzones = false; // conturno dos botões f1 

// mouse virtual
static Vector2 GetMouseVirtual(void){
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    float scale = fminf((float)sw / (float)SCREEN_W, (float)sh / (float)SCREEN_H);
    float offX  = (sw - SCREEN_W * scale) * 0.5f;
    float offY  = (sh - SCREEN_H * scale) * 0.5f;

    Vector2 m = GetMousePosition();

    // caso fora da area util 
    if (m.x < offX || m.x > offX + SCREEN_W*scale ||
        m.y < offY || m.y > offY + SCREEN_H*scale)
        return (Vector2){ -1e9f, -1e9f };

    return (Vector2){ (m.x - offX) / scale, (m.y - offY) / scale };
}

// marcador para depurar alinhamento do mouse virtual
static void DebugDrawMouseVirtual(void){
    Vector2 mv = GetMouseVirtual();
    if (mv.x > -1e8f) DrawCircleV(mv, 4.0f, YELLOW);
}

// clique com folga 
static bool ButtonPressedVirtualPad(Rectangle r, float pad){
    Rectangle rp = (Rectangle){ r.x - pad, r.y - pad, r.width + 2*pad, r.height + 2*pad };
    Vector2 mv = GetMouseVirtual();
    bool over = CheckCollisionPointRec(mv, rp);
    if (gShowHotzones){
        DrawRectangleLinesEx(rp, 2, over ? GREEN : WHITE);
    }
    return over && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}


// calibragem
typedef enum { CAL_NONE, CAL_PLAY_TL, CAL_PLAY_BR, CAL_EXIT_TL, CAL_EXIT_BR } CalibState;
static CalibState gCal = CAL_NONE;
static Vector2 gCalTL; 

static void StartCalibratePlay(void){ gCal = CAL_PLAY_TL;  TraceLog(LOG_INFO, "Calibrar PLAY: clique TOP-LEFT"); }
static void StartCalibrateExit(void){ gCal = CAL_EXIT_TL;  TraceLog(LOG_INFO, "Calibrar EXIT: clique TOP-LEFT"); }

static void HandleCalibration(void){
    if (gCal == CAL_NONE) return;
    Vector2 mv = GetMouseVirtual();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mv.x > -1e8f){
        if (gCal == CAL_PLAY_TL){ gCalTL = mv; gCal = CAL_PLAY_BR; TraceLog(LOG_INFO, "Agora clique BOTTOM-RIGHT (PLAY)"); }
        else if (gCal == CAL_PLAY_BR){
            Rectangle r = (Rectangle){ fminf(gCalTL.x, mv.x), fminf(gCalTL.y, mv.y),
                                       fabsf(mv.x - gCalTL.x), fabsf(mv.y - gCalTL.y) };
            R_MENU_PLAY = r; gCal = CAL_NONE;
            printf("\n// PLAY calibrado: { %.0f, %.0f, %.0f, %.0f }\n", r.x, r.y, r.width, r.height);
        }
        else if (gCal == CAL_EXIT_TL){ gCalTL = mv; gCal = CAL_EXIT_BR; TraceLog(LOG_INFO, "Agora clique BOTTOM-RIGHT (EXIT)"); }
        else if (gCal == CAL_EXIT_BR){
            Rectangle r = (Rectangle){ fminf(gCalTL.x, mv.x), fminf(gCalTL.y, mv.y),
                                       fabsf(mv.x - gCalTL.x), fabsf(mv.y - gCalTL.y) };
            R_MENU_EXIT = r; gCal = CAL_NONE;
            printf("\n// EXIT calibrado: { %.0f, %.0f, %.0f, %.0f }\n", r.x, r.y, r.width, r.height);
        }
    }
}

typedef enum { // telas do jogo 
    SCREEN_MENU,
    SCREEN_ENREDO,  
    SCREEN_PHASE1,
    SCREEN_PICKUP_ANIM,
    SCREEN_PHASE2,
    SCREEN_CREDITS,
    SCREEN_WIN,
    SCREEN_LOSE
} GameScreen;

int main(void){
    InitWindow(SCREEN_W, SCREEN_H, "RevoluCIn – Runner & Boss");
    InitAudioDevice(); 
    SetTargetFPS(60); // trava fpd em 60 para deixar mais estavel 

    Assets A; Assets_Load(&A); // carrega os assets

    // musicas
    PlayMusicStream(A.mMenu);

    // inica player e fase 1 
    Player player; Player_Init(&player, 120, SCREEN_H-110);
    Phase1 p1; Phase1_Init(&p1, &player, SCREEN_H-60);
    Phase2 p2; // se passar fase 1

    GameScreen screen = SCREEN_MENU; // primeira tela  
    float pickupTimer = 0; // tempo cutscene arma

    while (!WindowShouldClose()){ // enquanto a janela aberta

        float dt = GetFrameTime(); // delta do timer do frame
        UpdateMusicStream(A.mMenu);
        UpdateMusicStream(A.mPhase1);
        UpdateMusicStream(A.mPhase2);

        if (IsKeyPressed(KEY_F1)) gShowHotzones = !gShowHotzones;
        if (IsKeyPressed(KEY_F2)) StartCalibratePlay(); // calibrar botão jogar 
        if (IsKeyPressed(KEY_F3)) StartCalibrateExit(); // calibrar botão sair 
        HandleCalibration();


        switch (screen){ // tela atual

        case SCREEN_MENU: {

            if (!IsMusicStreamPlaying(A.mMenu)) PlayMusicStream(A.mMenu);
            if (IsMusicStreamPlaying(A.mPhase1)) StopMusicStream(A.mPhase1);
            if (IsMusicStreamPlaying(A.mPhase2)) StopMusicStream(A.mPhase2);

            if (ButtonPressedVirtualPad(R_MENU_PLAY, 12.0f)){

                // tela com explicação enredo 
                screen = SCREEN_ENREDO;

                // reseta player e fase 1  
                Player_Init(&player, 120, SCREEN_H-110);
                Phase1_Init(&p1, &player, SCREEN_H-60);
            }
            if (ButtonPressedVirtualPad(R_MENU_EXIT, 12.0f)){
                CloseWindow(); return 0;
            }
        } break;

        case SCREEN_ENREDO: {

            // vai para fase 1 com enter ou clique
            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){

                // muda musica 
                if (IsMusicStreamPlaying(A.mMenu)) StopMusicStream(A.mMenu);
                PlayMusicStream(A.mPhase1);

                screen = SCREEN_PHASE1;
            }
        } break;

        case SCREEN_PHASE1: {

            Phase1_Update(&p1, dt, &A); // atualiza a fase 1 

            if (p1.failed){ // perdeu 
                StopMusicStream(A.mPhase1);
                screen = SCREEN_LOSE;

            } else if (p1.finished){ // passou para fase 2 
                StopMusicStream(A.mPhase1);

                // cutscene arma
                PlaySound(A.sPickupWeapon);
                pickupTimer = 2.0f; 
                screen = SCREEN_PICKUP_ANIM;
            }
        } break;

        case SCREEN_PICKUP_ANIM: {

            pickupTimer -= dt; // contagem regressiva da cena

            if (pickupTimer <= 0){
                // vai para fase 2
                PlayMusicStream(A.mPhase2);
                Player_Init(&player, 120, SCREEN_H-110);
                player.hasWeapon = true;
                Phase2_Init(&p2, &player, SCREEN_H-60);
                screen = SCREEN_PHASE2;
            }
        } break;

        case SCREEN_PHASE2: {

            Phase2_Update(&p2, dt, &A); // atualiza fase 2 

            if (p2.lose){ // perdeu 
                StopMusicStream(A.mPhase2);
                screen = SCREEN_LOSE;
            } else if (p2.win){ // ganhou 
                StopMusicStream(A.mPhase2);
                screen = SCREEN_CREDITS; // creditos ao finalizar o jogo 
            }
        } break;

        case SCREEN_CREDITS: {
            // enter ou clica para sair dos credidos 
            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                screen = SCREEN_WIN;
            }
        } break;

        case SCREEN_WIN: { // imagem de vitoria + jogar dnv ou sair 
            if (ButtonPressedVirtualPad(R_PLAYAGAIN_WIN, 12.0f)){
                StopMusicStream(A.mMenu);
                PlayMusicStream(A.mMenu);
                screen = SCREEN_MENU;
            }
            if (ButtonPressedVirtualPad(R_EXIT_WIN, 12.0f)){
                CloseWindow(); return 0;
            }
        } break;

        case SCREEN_LOSE: { // imagem de vitoria + jogar dnv ou sair 
            if (ButtonPressedVirtualPad(R_PLAYAGAIN_LOSE, 12.0f)){
                
                StopMusicStream(A.mMenu);
                PlayMusicStream(A.mMenu);
                screen = SCREEN_MENU;
            }
            if (ButtonPressedVirtualPad(R_EXIT_LOSE, 12.0f)){
                CloseWindow(); return 0;
            }
        } break;
        }

        // desenho das telas
        BeginDrawing();
        ClearBackground(BLACK);

        switch (screen){
        case SCREEN_MENU:
            DrawFull(A.texHome);
            if (gShowHotzones){
                DrawRectangleLinesEx(R_MENU_PLAY, 2, WHITE);
                DrawRectangleLinesEx(R_MENU_EXIT, 2, WHITE);
            }
            break;

        case SCREEN_ENREDO:
            DrawFull(A.texEnredo); 
            break;

        case SCREEN_PHASE1:
            Phase1_Draw(&p1, &A);
            break;

        case SCREEN_PICKUP_ANIM: {

            // desenha cutscene 
            float sw = (float)SCREEN_W, sh = (float)SCREEN_H;
            float tw = (float)A.texPickup.width, th = (float)A.texPickup.height;

            float screenAR = sw / sh;
            float texAR    = tw / th;

            Rectangle srcCover;
            if (texAR > screenAR){
                float newW = th * screenAR;
                float x = (tw - newW) * 0.5f;
                srcCover = (Rectangle){ x, 0, newW, th };
            } else {
                float newH = tw / screenAR;
                float y = (th - newH) * 0.5f;
                srcCover = (Rectangle){ 0, y, tw, newH };
            }

            // margem 
            float marginTop = 44.0f;

            // 1
            float sampleH = fminf(40.0f, th * 0.08f); 
            Rectangle srcBlue = (Rectangle){ 0, 0, tw, sampleH };
            Rectangle dstBlue = (Rectangle){ 0, 0, sw, marginTop };
            DrawTexturePro(A.texPickup, srcBlue, dstBlue, (Vector2){0,0}, 0, WHITE);

            // 2
            Rectangle dstCover = (Rectangle){ 0, marginTop, sw, sh };
            DrawTexturePro(A.texPickup, srcCover, dstCover, (Vector2){0,0}, 0, WHITE);
        } break;

        case SCREEN_PHASE2:
            Phase2_Draw(&p2, &A);
            break;

        case SCREEN_CREDITS:
            DrawFull(A.texCredits);
            break;

        case SCREEN_WIN:
            DrawFull(A.texWin);
            if (gShowHotzones){
                DrawRectangleLinesEx(R_PLAYAGAIN_WIN, 2, WHITE);
                DrawRectangleLinesEx(R_EXIT_WIN,      2, WHITE);
            }
            break;

        case SCREEN_LOSE:
            DrawFull(A.texLose);
            if (gShowHotzones){
                DrawRectangleLinesEx(R_PLAYAGAIN_LOSE, 2, WHITE);
                DrawRectangleLinesEx(R_EXIT_LOSE,      2, WHITE);
            }
            break;
        }

        // ponto - mouse virtual
        DebugDrawMouseVirtual();

        EndDrawing();
    }

    Assets_Unload(&A);
    CloseAudioDevice();
    CloseWindow(); // fecha a janela 
    return 0;
}
