#include <raylib.h>
#include "Game.h"
#include "Settings.h"

int main() {
    InitWindow(window_width, window_height, title);

    InitAudioDevice();
    SetTargetFPS(fps);

    Game game;
    game.currentState = STATE_MENU; // Починаємо з головного меню

    int selectedTowerType = 0;
    Color BG_COLOR = { 15, 18, 25, 255 };
    Color GRID_COLOR = { 25, 30, 42, 255 };

    // Завантажуємо звук і передаємо в об'єкт гри
    Sound shootSound = LoadSound("assets/shoot.wav");
    game.shootSound = shootSound; // <-- Передаємо в клас Game

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        // --- ЛОГІКА СТАНІВ ГРИ ---
        if (game.currentState == STATE_MENU) {
            // Клік для старту гри
            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                game.LoadLevel(1); // Завантажуємо 1-й рівень
                game.currentState = STATE_PLAYING;
            }
        }
        else if (game.currentState == STATE_PLAYING) {
            if (IsKeyPressed(KEY_ONE)) selectedTowerType = 0;
            if (IsKeyPressed(KEY_ONE + 1)) selectedTowerType = 1;
            if (IsKeyPressed(KEY_ONE + 2)) selectedTowerType = 2;

            // Лівий клік — поставити вежу
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (mousePos.y > 80) {
                    game.PlaceTower(mousePos, selectedTowerType);
                }
            }

            // Правий клік — прибрати (продати) вежу
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                if (mousePos.y > 80) {
                    game.RemoveTowerAt(mousePos);
                }
            }

            if (IsKeyPressed(KEY_SPACE)) {
                game.StartNextWave();
            }

            game.Update();

            if (game.playerLives <= 0) {
                game.currentState = STATE_GAMEOVER;
            }
        }
        // ЗМІНА 1: Перезапуск на R працює і при поразці, і при перемозі
        else if (game.currentState == STATE_GAMEOVER || game.currentState == STATE_VICTORY) {
            if (IsKeyPressed(KEY_R)) {
                game.Reset(); // Перезапуск гри
                game.LoadLevel(1);
                game.currentState = STATE_PLAYING;
            }
        }

        // --- МАЛЮВАННЯ ---
        BeginDrawing();
        ClearBackground(BG_COLOR);

        if (game.currentState == STATE_MENU) {
            // Малювання Головного Меню
            DrawText("CYBER TOWER DEFENSE", window_width / 2 - 240, window_height / 2 - 100, 36, SKYBLUE);
            DrawText("Press [ENTER] or Click to Start", window_width / 2 - 160, window_height / 2 - 20, 20, RAYWHITE);
            DrawText("Protect your grid from incoming cyber threats.", window_width / 2 - 190, window_height / 2 + 30, 16, GRAY);
        }
        else if (game.currentState == STATE_PLAYING || game.currentState == STATE_GAMEOVER || game.currentState == STATE_VICTORY) {
            // Малювання сітки та ігрового поля
            for (int x = 0; x < window_width; x += 40) DrawLine(x, 80, x, window_height, GRID_COLOR);
            for (int y = 80; y < window_height; y += 40) DrawLine(0, y, window_width, y, GRID_COLOR);

            // Шлях
            for (size_t i = 0; i < game.waypoints.size() - 1; i++) {
                DrawLineEx(game.waypoints[i], game.waypoints[i + 1], 28, { 40, 48, 65, 255 });
                DrawCircleV(game.waypoints[i], 14, { 55, 65, 85, 255 });
            }
            if (!game.waypoints.empty()) DrawCircleV(game.waypoints.back(), 14, { 55, 65, 85, 255 });

            // Вежі
            for (const auto& tower : game.towers) {
                Color towerColor = (tower.type == 1) ? PURPLE : ((tower.type == 2) ? SKYBLUE : BLUE);
                DrawCircleV(tower.position, 18, towerColor);
                DrawCircleV(tower.position, 8, WHITE);

                // Відображення рівня вежі (кожні 5 вбивств = +1 рівень)
                if (tower.kills > 0) {
                    DrawText(TextFormat("Lv%d", (tower.kills / 5) + 1), tower.position.x - 10, tower.position.y - 30, 10, WHITE);
                }
            }

            // Вороги
            for (const auto& enemy : game.enemies) {
                if (enemy.active) {
                    Color enemyColor = RED;
                    if (enemy.type == 1) enemyColor = YELLOW;
                    else if (enemy.type == 2) enemyColor = ORANGE;
                    else if (enemy.type == 3) enemyColor = MAGENTA;

                    DrawCircleV(enemy.position, 10, enemyColor);
                    DrawRectangle(enemy.position.x - 15, enemy.position.y - 20, 30, 4, BLACK);
                    float hpPercent = (float)enemy.health / enemy.maxHealth;
                    DrawRectangle(enemy.position.x - 15, enemy.position.y - 20, 30 * hpPercent, 4, GREEN);
                }
            }

            // Снаряди
            for (auto& proj : game.projectiles) {
                if (proj.active) {
                    // Якщо снаряд тільки з'явився, малюємо спалах
                    if (proj.lifeTime > 0) {
                        DrawCircleV(proj.position, 12, ORANGE);
                        proj.lifeTime -= GetFrameTime();
                    }
                    DrawCircleV(proj.position, 5, YELLOW);
                }
            }

            // Інтерфейс (HUD)
            DrawRectangle(0, 0, window_width, 80, { 10, 12, 18, 250 });
            DrawLine(0, 80, window_width, 80, { 60, 75, 100, 255 });

            DrawText(TextFormat("LIVES: %d", game.playerLives), 20, 15, 18, { 255, 80, 80, 255 });
            DrawText(TextFormat("GOLD: %d G", game.playerGold), 140, 15, 18, GOLD);
            DrawText("[1] Std (100G)  [2] Sniper (175G)  [3] Cryo (150G)", 280, 42, 13, LIGHTGRAY);
            DrawText("[1-3] Select  [L-Click] Build  [R-Click] Sell  [SPACE] Wave", 260, 21, 12, LIGHTGRAY);
            DrawText("[SPACE] Next Wave", 650, 30, 14, GREEN);

            // Екран поразки
            if (game.currentState == STATE_GAMEOVER) {
                DrawRectangle(0, 0, window_width, window_height, Fade(BLACK, 0.85f));
                DrawText("MISSION FAILED", window_width / 2 - 130, window_height / 2 - 40, 40, RED);
                DrawText("Press [R] to Restart", window_width / 2 - 100, window_height / 2 + 15, 20, LIGHTGRAY);
            }
            else if (game.currentState == STATE_VICTORY) {
                DrawRectangle(0, 0, window_width, window_height, Fade(BLACK, 0.85f));
                DrawText("VICTORY! YOU SAVED THE GRID", window_width / 2 - 220, window_height / 2 - 40, 30, GREEN);
                DrawText("Press [R] to Restart", window_width / 2 - 100, window_height / 2 + 20, 20, LIGHTGRAY);
            }
        }

        if (game.bossSpawnFlash) {
            DrawRectangle(0, 0, window_width, window_height, Fade(WHITE, 0.4f));
            game.bossSpawnFlash = false; // Вимикаємо після першого кадру
        }

        EndDrawing();
    }

    UnloadSound(shootSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}