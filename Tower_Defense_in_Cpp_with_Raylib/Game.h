#pragma once
#include <raylib.h>
#include <vector>
#include "Enemy.h"
#include "Tower.h"
#include "Projectile.h"

using namespace std;

enum GameState {
    STATE_MENU = 0,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_VICTORY // <-- Додайте сюди
};

class Game {
private:
    double spawnTimer;
    double spawnInterval;
    short enemiesToSpawn;
    short enemiesSpawned;
    short currentWave;
    bool waveActive;

public:
    bool bossSpawnFlash = false;
    GameState currentState;
    short playerLives;
    short playerGold;
    vector<Vector2> waypoints;
    vector<Enemy> enemies;
    vector<Tower> towers;
    vector<Projectile> projectiles;
    int currentLevel;
    Sound shootSound;

    Game();
    double Distance(Vector2 v1, Vector2 v2);
    void Reset();
    void Update();
    void PlaceTower(Vector2 pos, int towerType);
    void RemoveTowerAt(Vector2 pos);
    void StartNextWave();
    bool IsOnPath(Vector2 pos);
    void LoadLevel(int levelNum);
};