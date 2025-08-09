// src/main.c
#include "raylib.h"
#include "assets.h"
#include "player.h"
#include "phase1.h"
#include "phase2.h"

#include <stdio.h>
#include <stdbool.h>

#define SCREEN_W 1024
#define SCREEN_H 576

typedef enum GameState { STATE_MENU, STATE_CREDITS, STATE_PHASE1, STATE_PICKUP_ANIM, STATE_PHASE2, STATE_GAMEOVER, STATE_VICTORY } GameState;

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "Runner com Boss - Exemplo");
    InitAudioDevice();
    SetTargetFPS(60);

    Assets assets;
    LoadAssets(&assets);

    // tocar música do menu
    PlayMusicStream(assets.musicMenu);

    Player player;
    InitPlayer(&player);

    Phase1 phase1;
    InitPhase1(&phase1);

    Phase2 phase2;
    InitPhase2(&phase2);

    GameState state = STATE_MENU;

    // variáveis pra animação da coleta da arma
    float pickupTimer = 0.0f;
    const float pickupDuration = 1.2f;
    Vector2 pickupPos = { 900, 420 };

    // loop principal
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // atualizar musicas (raylib exige UpdateMusicStream por frame)
        UpdateMusicStream(assets.musicMenu);
        UpdateMusicStream(assets.musicPhase1);
        UpdateMusicStream(assets.musicPhase2);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (state) {
            case STATE_MENU:
                // desenhar menu
                DrawText("Jogo Runner - MENU", 320, 120, 32, BLACK);
                DrawText("Enter para jogar   C para creditos   Esc para sair", 220, 220, 20, DARKGRAY);
                if (IsKeyPressed(KEY_ENTER)) {
                    // iniciar fase1
                    StopMusicStream(assets.musicMenu);
                    PlayMusicStream(assets.musicPhase1);
                    state = STATE_PHASE1;
                    ResetPhase1(&phase1);
                    InitPlayer(&player);
                }
                if (IsKeyPressed(KEY_C)) state = STATE_CREDITS;
                break;

            case STATE_CREDITS:
                DrawText("CREDITOS", 420, 60, 32, BLACK);
                DrawText("Equipe: Theo, Roni, Jefferson, Barros, Gabi, Leticia", 150, 140, 20, DARKGRAY);
                DrawText("Pressione ENTER para voltar", 360, 520, 18, DARKGRAY);
                if (IsKeyPressed(KEY_ENTER)) {
                    state = STATE_MENU;
                }
                break;

            case STATE_PHASE1:
                // atualizar e desenhar fase1
                UpdatePhase1(&phase1, &player, &assets, dt);
                DrawPhase1(&phase1, &player, &assets);

                // se player morreu (colidiu), ir pro gameover
                if (player.lives <= 0) {
                    StopMusicStream(assets.musicPhase1);
                    PlaySound(assets.sfxLoseGame);
                    state = STATE_GAMEOVER;
                }
                // se completou os 60s -> transição e animação de pickup
                if (phase1.finished) {
                    // posiciona animação perto do spawn da arma
                    pickupTimer = 0;
                    state = STATE_PICKUP_ANIM;
                    // tocar som de pickup depois da animação
                }
                break;

            case STATE_PICKUP_ANIM:
                // animação simples: reduz alpha / ou movimenta o sprite da arma até o player
                pickupTimer += dt;
                // desenha background da fase (pode reaproveitar)
                DrawPhase1(&phase1, &player, &assets);

                // calcula posição interpolada entre arma e player
                float t = pickupTimer / pickupDuration;
                if (t > 1.0f) t = 1.0f;
                Vector2 weaponDrawPos;
                weaponDrawPos.x = pickupPos.x + (player.pos.x - pickupPos.x) * t;
                weaponDrawPos.y = pickupPos.y + (player.pos.y - pickupPos.y) * t;

                // desenha arma movendo
                DrawTexture(assets.weapon, (int)weaponDrawPos.x, (int)weaponDrawPos.y, WHITE);

                if (pickupTimer >= pickupDuration) {
                    // animação acabou -> jogador recebe arma
                    player.hasWeapon = true;
                    PlaySound(assets.sfxPickup);
                    // trocar música para phase2 e iniciar fase2
                    StopMusicStream(assets.musicPhase1);
                    PlayMusicStream(assets.musicPhase2);
                    state = STATE_PHASE2;
                    // reset phase2 e player pos
                    ResetPhase2(&phase2);
                    InitPlayer(&player);
                    player.hasWeapon = true; // manter arma
                }
                break;

            case STATE_PHASE2:
                // atualizar jogador com lógica da fase2
                UpdatePlayerLevel2(&player, &assets, dt);
                UpdatePhase2(&phase2, &player, &assets, dt);

                // desenhar fase2
                // chão simples
                DrawRectangle(0, 480, 1024, 96, LIGHTGRAY);
                DrawPlayer(&player, &assets);
                DrawPhase2(&phase2, &player, &assets);

                // verificar vitória ou derrota
                if (player.lives <= 0) {
                    StopMusicStream(assets.musicPhase2);
                    PlaySound(assets.sfxLoseGame);
                    state = STATE_GAMEOVER;
                }
                if (phase2.hp <= 0) {
                    StopMusicStream(assets.musicPhase2);
                    PlaySound(assets.sfxWin);
                    state = STATE_VICTORY;
                }
                break;

            case STATE_GAMEOVER:
                DrawText("GAME OVER", 360, 200, 48, RED);
                DrawText("ENTER para voltar ao menu", 340, 280, 20, DARKGRAY);
                if (IsKeyPressed(KEY_ENTER)) {
                    state = STATE_MENU;
                    PlayMusicStream(assets.musicMenu);
                }
                break;

            case STATE_VICTORY:
                DrawText("VOCE VENCEU!", 340, 200, 48, GREEN);
                DrawText("ENTER para voltar ao menu", 340, 280, 20, DARKGRAY);
                if (IsKeyPressed(KEY_ENTER)) {
                    state = STATE_MENU;
                    PlayMusicStream(assets.musicMenu);
                }
                break;
        }

        EndDrawing();

        // importante: chamar UpdateMusicStream enquanto a música estiver tocando
        if (IsMusicStreamPlaying(assets.musicMenu)) UpdateMusicStream(assets.musicMenu);
        if (IsMusicStreamPlaying(assets.musicPhase1)) UpdateMusicStream(assets.musicPhase1);
        if (IsMusicStreamPlaying(assets.musicPhase2)) UpdateMusicStream(assets.musicPhase2);
    }

    // unload
    UnloadAssets(&assets);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
