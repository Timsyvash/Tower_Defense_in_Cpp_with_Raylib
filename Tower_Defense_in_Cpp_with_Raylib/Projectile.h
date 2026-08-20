#pragma once
#include <raylib.h>

struct Projectile {
    Vector2 position;
    Vector2 targetPosition;
    float speed;
    bool active;
    float lifeTime = 0.1f;

    // Конструктор за замовчуванням
    Projectile() = default;

    // Конструктор із параметрами
    Projectile(Vector2 pos, Vector2 target, float spd, bool act)
        : position(pos), targetPosition(target), speed(spd), active(act) {}
};