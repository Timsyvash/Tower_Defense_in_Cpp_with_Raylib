#include "Projectile.h"
#include "Game.h"

void Projectile::Update()
{
    Game game;
    for (auto& proj : projectiles) {
        if (!proj.active) continue;
        double deltaTime = GetFrameTime();

        float dist = game.Distance(proj.position, proj.targetPos);
        if (dist > 5.0f) {
            proj.position.x += ((proj.targetPos.x - proj.position.x) / dist) * proj.speed * deltaTime;
            proj.position.y += ((proj.targetPos.y - proj.position.y) / dist) * proj.speed * deltaTime;
        }
        else {
            proj.active = false; // Projectile arrived
        }
    }
}
