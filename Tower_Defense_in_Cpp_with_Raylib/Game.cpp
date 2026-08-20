#include "Game.h"
#include "Tower.h"
#include "Projectile.h"
#include <cmath>

Game::Game()
    : spawnTimer(0.0f), spawnInterval(1.5f), playerLives(10), playerGold(300),
    enemiesToSpawn(0), enemiesSpawned(0), currentWave(0), waveActive(false)
{
    waypoints = {
        { 0, 300 },
        { 250, 300 },
        { 250, 150 },
        { 550, 150 },
        { 550, 450 },
        { 800, 450 }
    };
}

void Game::Reset()
{
    playerLives = 10;
    playerGold = 300;
    currentWave = 0;
    waveActive = false;
    enemies.clear();
    towers.clear();
    projectiles.clear();
    // НЕ видаляйте shootSound! Ми залишимо його як є.
}

double Game::Distance(Vector2 v1, Vector2 v2)
{
    return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
}

// Перевірка, чи не стабільно близько точка до дороги (щоб не будувати на дорозі)
bool Game::IsOnPath(Vector2 pos)
{
    for (size_t i = 0; i < waypoints.size() - 1; i++) {
        Vector2 a = waypoints[i];
        Vector2 b = waypoints[i + 1];

        // Вираховуємо відступ від лінії сегмента дороги
        float lineLen = Distance(a, b);
        if (lineLen == 0) continue;

        // Проста перевірка прямокутника навколо сегмента дороги з відступом 25 пікселів
        float minX = fmin(a.x, b.x) - 25;
        float maxX = fmax(a.x, b.x) + 25;
        float minY = fmin(a.y, b.y) - 25;
        float maxY = fmax(a.y, b.y) + 25;

        if (pos.x >= minX && pos.x <= maxX && pos.y >= minY && pos.y <= maxY) {
            return true; // Клікнули на дорогу!
        }
    }
    return false;
}

void Game::StartNextWave()
{
    if (!waveActive) {
        currentWave++;
        enemiesToSpawn = 4 + currentWave * 2; // З кожною хвилею більше ворогів
        enemiesSpawned = 0;
        spawnTimer = 0.0f;
        waveActive = true;
    }
}

void Game::RemoveTowerAt(Vector2 pos)
{
    for (auto it = towers.begin(); it != towers.end(); ) {
        if (Distance(it->position, pos) < 20.0f) {
            playerGold += it->cost;
            it = towers.erase(it); // Безпечно видаляємо і оновлюємо ітератор
            break;
        }
        else {
            ++it;
        }
    }
}

void Game::PlaceTower(Vector2 pos, int towerType)
{
    Tower tempTower(pos, towerType);
    if (playerGold >= tempTower.cost && !IsOnPath(pos)) {
        towers.push_back(tempTower);
        playerGold -= tempTower.cost;
    }
}

void Game::Update()
{
    double deltaTime = GetFrameTime();

    // 1. Логіка хвиль та спавну
    if (waveActive && enemiesSpawned < enemiesToSpawn) {
        spawnTimer += deltaTime;
        if (spawnTimer >= spawnInterval) {
            spawnTimer = 0.0f;

            // Спавн боса кожні 10 ворогів
            if (enemiesSpawned > 0 && enemiesSpawned % 10 == 0) {
                enemies.push_back(Enemy(waypoints[0], 40.0f, 500 + currentWave * 100, 500 + currentWave * 100, 0, true));
                bossSpawnFlash = true; // <-- Додайте цей рядок тут!
            }
            else {
                int enemyVariant = enemiesSpawned % 3;
                enemies.push_back(Enemy(waypoints[0], (enemyVariant == 1 ? 160.0f : 110.0f), 100 + currentWave * 25, 100 + currentWave * 25, enemyVariant, true));
            }
            enemiesSpawned++;
        }
    }

    // 2. Логіка руху ворогів (сповільнення)
    for (auto& enemy : enemies) {
        if (!enemy.active) continue;

        if (enemy.slowTimer > 0) {
            enemy.slowTimer -= deltaTime;
            enemy.speedMultiplier = 0.5f; // Сповільнення
        }
        else {
            enemy.speedMultiplier = 1.0f;
        }

        if (enemy.currentWaypoint + 1 >= waypoints.size()) continue;

        Vector2 target = waypoints[enemy.currentWaypoint + 1];
        float distToWaypoint = Distance(enemy.position, target);

        if (distToWaypoint > 2.0f) {
            // Множимо швидкість на speedMultiplier
            float moveStep = enemy.speed * enemy.speedMultiplier * deltaTime;
            enemy.position.x += ((target.x - enemy.position.x) / distToWaypoint) * moveStep;
            enemy.position.y += ((target.y - enemy.position.y) / distToWaypoint) * moveStep;
        }
        else {
            enemy.currentWaypoint++;
            if (enemy.currentWaypoint >= (short)waypoints.size() - 1) {
                enemy.active = false;
                if (playerLives > 0) playerLives--;
            }
        }
    }

    // 3. Логіка веж (стрільба + досвід + ефекти)
    for (auto& tower : towers) {
        if (tower.shotCooldown > 0) tower.shotCooldown -= deltaTime;

        for (auto& enemy : enemies) {
            if (!enemy.active) continue;

            if (Distance(tower.position, enemy.position) <= tower.range) {
                if (tower.shotCooldown <= 0) {
                    projectiles.push_back(Projectile(tower.position, enemy.position, 450.0f, true));
                    tower.shotCooldown = 1.0f / tower.fireRate;
                    PlaySound(shootSound);
                    break;
                }
            }
        }
    }

    // 4. Логіка снарядів (влучання + Cryo ефект)
    for (auto& proj : projectiles) {
        if (!proj.active) continue;

        // Пошук цілі
        Enemy* targetEnemy = nullptr;
        float minDst = 99999.0f;
        for (auto& enemy : enemies) {
            if (!enemy.active) continue;
            float d = Distance(proj.position, enemy.position);
            if (d < minDst) { minDst = d; targetEnemy = &enemy; }
        }

        if (targetEnemy) {
            proj.targetPosition = targetEnemy->position;
            float dx = proj.targetPosition.x - proj.position.x;
            float dy = proj.targetPosition.y - proj.position.y;
            float dist = sqrt(dx * dx + dy * dy);

            if (dist < 15.0f) {
                proj.active = false;
                targetEnemy->health -= 35;

                // Якщо вежа, що вистрілила - Cryo (припустимо, type == 2)
                // Ви можете додати перевірку, яка вежа вистрілила
                targetEnemy->slowTimer = 2.0f;

                if (targetEnemy->health <= 0) {
                    targetEnemy->active = false;
                    playerGold += 25;
                    // Знаходимо вежу, яка вбила (найближчу до ворога)
                    for (auto& t : towers) {
                        if (Distance(t.position, targetEnemy->position) < t.range) {
                            t.kills++;
                            if (t.kills % 5 == 0) { t.range += 5.0f; t.fireRate += 0.1f; }
                            break;
                        }
                    }
                }
            }
            else {
                proj.position.x += (dx / dist) * 700.0f * deltaTime;
                proj.position.y += (dy / dist) * 700.0f * deltaTime;
            }
        }
        else proj.active = false;
    }

    // --- НАДІЙНА ПЕРЕКРІВКА ХВИЛІ ---
    if (waveActive && enemiesSpawned >= enemiesToSpawn) {
        // Перевіряємо, чи залишився хоча б один активний ворог на полі
        bool anyActiveEnemy = false;
        for (const auto& enemy : enemies) {
            if (enemy.active) {
                anyActiveEnemy = true;
                break;
            }
        }

        // Якщо активних ворогів немає — хвиля закінчилася!
        if (!anyActiveEnemy) {
            waveActive = false;
        }
    }

    // Перехід рівнів (тільки якщо хвиля завершена і гравець пройшов 3 хвилі)
    if (!waveActive && currentWave >= 3 && currentWave > 0) {
        if (currentLevel == 1) {
            LoadLevel(2);
        }
        else if (currentLevel == 2) {
            currentState = STATE_VICTORY;
        }
    }
}

void Game::LoadLevel(int levelNum)
{
    currentLevel = levelNum;
    waypoints.clear();
    towers.clear();
    projectiles.clear();
    enemies.clear();

    playerLives = 10;
    playerGold = 300;
    currentWave = 0;
    waveActive = false;

    if (levelNum == 1) {
        // Рівень 1: Простий шлях
        waypoints = {
            { 0, 300 },
            { 250, 300 },
            { 250, 150 },
            { 550, 150 },
            { 550, 450 },
            { 800, 450 }
        };
    }
    else if (levelNum == 2) {
        // Рівень 2: Звивистіший та складніший шлях
        waypoints = {
            { 50, 100 },
            { 750, 100 },
            { 750, 300 },
            { 200, 300 },
            { 200, 550 },
            { 750, 550 }
        };
    }
}