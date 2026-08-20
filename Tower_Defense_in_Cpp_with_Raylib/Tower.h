#pragma once
#include <raylib.h>

enum TowerType {
    TOWER_STANDARD = 0,
    TOWER_SNIPER,
    TOWER_CRYO
};

struct Tower {
    Vector2 position;
    float range;
    float fireRate;
    float shotCooldown;
    int type; // 0 - Стандарт, 1 - Снайпер, 2 - Сповільнювач
    int cost;
    int kills = 0;

    Tower() = default;

    Tower(Vector2 pos, int tType) : position(pos), shotCooldown(0.0f) {
        type = tType;
        if (type == TOWER_STANDARD) {
            range = 140.0f;
            fireRate = 2.0f;
            cost = 100;
        }
        else if (type == TOWER_SNIPER) {
            range = 250.0f;
            fireRate = 0.8f;
            cost = 175;
        }
        else if (type == TOWER_CRYO) {
            range = 100.0f;
            fireRate = 1.5f;
            cost = 150;
        }
    }
};