#pragma once
#include <raylib.h>

struct Enemy {
    Vector2 position;
    float speed;
    int health;
    int maxHealth;
    int currentWaypoint;  // Назва має збігатися з тим, що в Game.cpp!
    bool active;
    float speedMultiplier = 1.0f;
    float slowTimer = 0.0f;
    int type;

    // Конструктор за замовчуванням
    Enemy() = default;

    // Конструктор із параметрами
    Enemy(Vector2 pos, float spd, int hp, int maxHp, int wpIdx, bool act)
        : position(pos), speed(spd), health(hp), maxHealth(maxHp), currentWaypoint(wpIdx), active(act) {}
};