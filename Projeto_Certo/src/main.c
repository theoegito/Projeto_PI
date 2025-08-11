#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include "assets.h"
#include "player.h"
#include "phase1.h"
#include "phase2.h"

#define SCREEN_W 1024
#define SCREEN_H 576

// desenha uma textura preenchendo a tela lógica 1024x576
static void DrawFull(Texture2D tex){
    Rectangle src = (Rectangle){0, 0, (float)tex.width, (float)tex.height};
    Rectangle dst = (Rectangle){0, 0, SCREEN_W, SCREEN_H};
    DrawTexturePro(tex, src, dst, (Vector2){0,0}, 0.0f, WHITE);
}

// ---------------- Hotzones alinhadas com a arte (1024x576) ----------------
static Rectangle R_MENU_PLAY = { 536, 285, 243, 88 };
static Rectangle R_MENU_EXIT = { 534, 397, 254, 88 };
// GANHOU
static Rectangle R_PLAYAGAIN_WIN = {209, 384, 518, 70};
static Rectangle R_EXIT_WIN      = { 360, 471, 218, 67 };
// PERDEU
static Rectangle R_PLAYAGAIN_LOSE = {224, 374, 487, 81};
static Rectangle R_EXIT_LOSE      = { 347, 472, 243, 72 };
// Debug (F1 mostra/oculta os retângulos)
static bool gShowHotzones = false;
// --------------------------------------------------------------------------

// ---------- Mouse virtual (mapa clique janela -> 1024x576) ----------
static Vector2 GetMouseVirtual(void){
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    float scale = fminf((float)sw / (float)SCREEN_W, (float)sh / (float)SCREEN_H);
    float offX  = (sw - SCREEN_W * scale) * 0.5f;
    float offY  = (sh - SCREEN_H * scale) * 0.5f;

    Vector2 m = GetMousePosition();

    // fora da área útil (letterbox)? devolve fora do virtual
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

// clique com folga (padding) — mais fácil de acertar
static bool ButtonPressedVirtualPad(Rectangle r, float pad){
    Rectangle rp = (Rectangle){ r.x - pad, r.y - pad, r.width + 2*pad, r.height + 2*pad };
    Vector2 mv = GetMouseVirtual();
    bool over = CheckCollisionPointRec(mv, rp);
    if (gShowHotzones){
        DrawRectangleLinesEx(rp, 2, over ? GREEN : WHITE);
    }
    return over && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}
// --------------------------------------------------------------------

// --------- Calibração interativa (F2/F3) ---------
typedef enum { CAL_NONE, CAL_PLAY_TL, CAL_PLAY_BR, CAL_EXIT_TL, CAL_EXIT_BR } CalibState;
static CalibState gCal = CAL_NONE;
static Vector2 gCalTL; // top-left

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
// -------------------------------------------------

typedef enum {
    SCREEN_MENU,
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
    SetTargetFPS(60);

    Assets A; Assets_Load(&A);

    // music
    PlayMusicStream(A.mMenu);

    Player player; Player_Init(&player, 120, SCREEN_H-110);
    Phase1 p1; Phase1_Init(&p1, &player, SCREEN_H-60);
    Phase2 p2; // will init later

    GameScreen screen = SCREEN_MENU;
    float pickupTimer = 0; // controls pickup animation time

    while (!WindowShouldClose()){
        float dt = GetFrameTime();
        UpdateMusicStream(A.mMenu);
        UpdateMusicStream(A.mPhase1);
        UpdateMusicStream(A.mPhase2);

        if (IsKeyPressed(KEY_F1)) gShowHotzones = !gShowHotzones;
        if (IsKeyPressed(KEY_F2)) StartCalibratePlay(); // calibrar JOGAR
        if (IsKeyPressed(KEY_F3)) StartCalibrateExit(); // calibrar SAIR
        HandleCalibration();

        // Screen logic
        switch (screen){
        case SCREEN_MENU: {
            if (!IsMusicStreamPlaying(A.mMenu)) PlayMusicStream(A.mMenu);
            if (IsMusicStreamPlaying(A.mPhase1)) StopMusicStream(A.mPhase1);
            if (IsMusicStreamPlaying(A.mPhase2)) StopMusicStream(A.mPhase2);

            if (ButtonPressedVirtualPad(R_MENU_PLAY, 12.0f)){
                // start phase 1
                StopMusicStream(A.mMenu);
                PlayMusicStream(A.mPhase1);
                Player_Init(&player, 120, SCREEN_H-110);
                Phase1_Init(&p1, &player, SCREEN_H-60);
                screen = SCREEN_PHASE1;
            }
            if (ButtonPressedVirtualPad(R_MENU_EXIT, 12.0f)){
                CloseWindow(); return 0;
            }
        } break;

        case SCREEN_PHASE1: {
            Phase1_Update(&p1, dt, &A);
            if (p1.failed){
                StopMusicStream(A.mPhase1);
                screen = SCREEN_LOSE;
            } else if (p1.finished){
                StopMusicStream(A.mPhase1);
                // pickup weapon animation
                PlaySound(A.sPickupWeapon);
                pickupTimer = 2.0f; // 2 seconds animation
                screen = SCREEN_PICKUP_ANIM;
            }
        } break;

        case SCREEN_PICKUP_ANIM: {
            pickupTimer -= dt;
            if (pickupTimer <= 0){
                // go to phase 2
                PlayMusicStream(A.mPhase2);
                Player_Init(&player, 120, SCREEN_H-110);
                player.hasWeapon = true;
                Phase2_Init(&p2, &player, SCREEN_H-60);
                screen = SCREEN_PHASE2;
            }
        } break;

        case SCREEN_PHASE2: {
            Phase2_Update(&p2, dt, &A);
            if (p2.lose){
                StopMusicStream(A.mPhase2);
                screen = SCREEN_LOSE;
            } else if (p2.win){
                StopMusicStream(A.mPhase2);
                // show credits first
                screen = SCREEN_CREDITS;
            }
        } break;

        case SCREEN_CREDITS: {
            // wait for click or any key, then win screen
            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                screen = SCREEN_WIN;
            }
        } break;

        case SCREEN_WIN: {
            if (ButtonPressedVirtualPad(R_PLAYAGAIN_WIN, 12.0f)){
                StopMusicStream(A.mMenu);
                PlayMusicStream(A.mMenu);
                screen = SCREEN_MENU;
            }
            if (ButtonPressedVirtualPad(R_EXIT_WIN, 12.0f)){
                CloseWindow(); return 0;
            }
        } break;

        case SCREEN_LOSE: {
            if (ButtonPressedVirtualPad(R_PLAYAGAIN_LOSE, 12.0f)){
                // restart from menu
                StopMusicStream(A.mMenu);
                PlayMusicStream(A.mMenu);
                screen = SCREEN_MENU;
            }
            if (ButtonPressedVirtualPad(R_EXIT_LOSE, 12.0f)){
                CloseWindow(); return 0;
            }
        } break;
        }

        // DRAW
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

        case SCREEN_PHASE1:
            Phase1_Draw(&p1, &A);
            break;

        case SCREEN_PICKUP_ANIM: {
            // ====== COVER com "respiro azul" no topo ======
            float sw = (float)SCREEN_W, sh = (float)SCREEN_H;
            float tw = (float)A.texPickup.width, th = (float)A.texPickup.height;

            float screenAR = sw / sh;
            float texAR    = tw / th;

            // Cover: calcula o recorte proporcional
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

            // margem desejada no topo (em pixels da tela)
            float marginTop = 44.0f;

            // 1) pinta a faixa azul usando um pedacinho do topo da própria textura
            float sampleH = fminf(40.0f, th * 0.08f); // tira ~8% do topo (puro azul)
            Rectangle srcBlue = (Rectangle){ 0, 0, tw, sampleH };
            Rectangle dstBlue = (Rectangle){ 0, 0, sw, marginTop };
            DrawTexturePro(A.texPickup, srcBlue, dstBlue, (Vector2){0,0}, 0, WHITE);

            // 2) desenha a arte principal em cover, deslocada para baixo
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

        // ponto do mouse virtual pra ajudar a alinhar
        DebugDrawMouseVirtual();

        EndDrawing();
    }

    Assets_Unload(&A);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
