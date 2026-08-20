#include "Enemy.h"
#include "Game.h"

void Enemy::Update()
{
    Game game;
    for (auto& enemy : enemies) {
        if (!enemy.active) continue;
        double deltaTime = GetFrameTime();

        Vector2 target = game.waypoints[enemy.currentWaypoint + 1];
        float distToWaypoint = game.Distance(enemy.position, target);

        // Move towards the next waypoint
        if (distToWaypoint > 2.0f) {
            enemy.position.x += ((target.x - enemy.position.x) / distToWaypoint) * enemy.speed * deltaTime;
            enemy.position.y += ((target.y - enemy.position.y) / distToWaypoint) * enemy.speed * deltaTime;
        }
        else {
            // Advance to next waypoint segment
            enemy.currentWaypoint++;
            if (enemy.currentWaypoint >= (short)game.waypoints.size() - 1) {
                enemy.active = false;
                playerLives--; // Enemy reached the end!
            }
        }
    }
}
