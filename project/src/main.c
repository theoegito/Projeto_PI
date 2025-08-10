#include "raylib.h"
#include <stdio.h>
#include "assets.h"
#include "player.h"
#include "phase1.h"
#include "phase2.h"

#define SCREEN_W 1024
#define SCREEN_H 576

typedef enum {
    SCREEN_MENU,
    SCREEN_PHASE1,
    SCREEN_PICKUP_ANIM,
    SCREEN_PHASE2,
    SCREEN_CREDITS,
    SCREEN_WIN,
    SCREEN_LOSE
} GameScreen;

static bool ButtonPressed(Rectangle r){
    Vector2 m = GetMousePosition();
    bool over = CheckCollisionPointRec(m, r);
    if (over) DrawRectangleRec(r, Fade(WHITE, 0.15f));
    DrawRectangleLinesEx(r, 2, RAYWHITE);
    return over && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

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

        // Screen logic
        switch (screen){
        case SCREEN_MENU: {
            if (!IsMusicStreamPlaying(A.mMenu)) PlayMusicStream(A.mMenu);
            if (IsMusicStreamPlaying(A.mPhase1)) StopMusicStream(A.mPhase1);
            if (IsMusicStreamPlaying(A.mPhase2)) StopMusicStream(A.mPhase2);

            if (ButtonPressed((Rectangle){SCREEN_W/2-120, SCREEN_H-220, 240, 48})){
                // start phase 1
                StopMusicStream(A.mMenu);
                PlayMusicStream(A.mPhase1);
                Player_Init(&player, 120, SCREEN_H-110);
                Phase1_Init(&p1, &player, SCREEN_H-60);
                screen = SCREEN_PHASE1;
            }
            if (ButtonPressed((Rectangle){SCREEN_W/2-120, SCREEN_H-160, 240, 48})){
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
            if (ButtonPressed((Rectangle){SCREEN_W/2-120, SCREEN_H-140, 240, 48})){
                StopMusicStream(A.mMenu);
                PlayMusicStream(A.mMenu);
                screen = SCREEN_MENU;
            }
            if (ButtonPressed((Rectangle){SCREEN_W/2-120, SCREEN_H-80, 240, 48})){
                CloseWindow(); return 0;
            }
        } break;

        case SCREEN_LOSE: {
            if (ButtonPressed((Rectangle){SCREEN_W/2-120, SCREEN_H-140, 240, 48})){
                // restart from menu
                StopMusicStream(A.mMenu);
                PlayMusicStream(A.mMenu);
                screen = SCREEN_MENU;
            }
            if (ButtonPressed((Rectangle){SCREEN_W/2-120, SCREEN_H-80, 240, 48})){
                CloseWindow(); return 0;
            }
        } break;
        }

        // DRAW
        BeginDrawing();
        ClearBackground(BLACK);

        switch (screen){
        case SCREEN_MENU:
            DrawTexture(A.texHome,0,0,WHITE);
            DrawText("Jogar", SCREEN_W/2-40, SCREEN_H-210, 28, WHITE);
            DrawRectangleLines(SCREEN_W/2-120, SCREEN_H-220, 240, 48, WHITE);
            DrawText("Sair", SCREEN_W/2-24, SCREEN_H-150, 28, WHITE);
            DrawRectangleLines(SCREEN_W/2-120, SCREEN_H-160, 240, 48, WHITE);
            break;

        case SCREEN_PHASE1:
            Phase1_Draw(&p1, &A);
            break;

        case SCREEN_PICKUP_ANIM:
            DrawTexture(A.texBackground,0,0,WHITE);
            DrawTexturePro(A.texPickup, (Rectangle){0,0,(float)A.texPickup.width,(float)A.texPickup.height},
                           (Rectangle){SCREEN_W/2-128, SCREEN_H/2-128, 256,256}, (Vector2){0,0}, 0, WHITE);
            DrawText("Arma coletada!", SCREEN_W/2-110, SCREEN_H/2+150, 24, WHITE);
            break;

        case SCREEN_PHASE2:
            Phase2_Draw(&p2, &A);
            break;

        case SCREEN_CREDITS:
            DrawTexture(A.texCredits,0,0,WHITE);
            DrawText("Pressione ENTER ou clique para continuar", SCREEN_W/2-260, SCREEN_H-40, 18, WHITE);
            break;

        case SCREEN_WIN:
            DrawTexture(A.texWin,0,0,WHITE);
            DrawText("Jogar novamente", SCREEN_W/2-110, SCREEN_H-130, 24, WHITE);
            DrawRectangleLines(SCREEN_W/2-120, SCREEN_H-140, 240, 48, WHITE);
            DrawText("Sair", SCREEN_W/2-24, SCREEN_H-70, 24, WHITE);
            DrawRectangleLines(SCREEN_W/2-120, SCREEN_H-80, 240, 48, WHITE);
            break;

        case SCREEN_LOSE:
            DrawTexture(A.texLose,0,0,WHITE);
            DrawText("Jogar novamente", SCREEN_W/2-110, SCREEN_H-130, 24, WHITE);
            DrawRectangleLines(SCREEN_W/2-120, SCREEN_H-140, 240, 48, WHITE);
            DrawText("Sair", SCREEN_W/2-24, SCREEN_H-70, 24, WHITE);
            DrawRectangleLines(SCREEN_W/2-120, SCREEN_H-80, 240, 48, WHITE);
            break;
        }

        EndDrawing();
    }

    Assets_Unload(&A);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
