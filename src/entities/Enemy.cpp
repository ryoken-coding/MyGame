#include "entities/Enemy.h"

#include <cmath>

#include "world/TileMap.h"

namespace {

const EnemyStats kEnemyStats[] = {
    // name          hp  speed  dano  aggro  cd    size
    {"Slime", 5, 55.0f, 8, 170.0f, 1.0f, Vector2{22.0f, 18.0f}},
    {"Goblin", 12, 85.0f, 12, 230.0f, 0.9f, Vector2{22.0f, 26.0f}},
    {"Goblin Líder", 18, 95.0f, 15, 260.0f, 0.8f, Vector2{24.0f, 28.0f}},
    {"Guardião", 54, 70.0f, 20, 320.0f, 0.7f, Vector2{36.0f, 40.0f}},
};

Vector2 normalizeOrZero(Vector2 v)
{
    const float len = std::sqrt(v.x * v.x + v.y * v.y);
    if (len < 0.0001f) return {0.0f, 0.0f};
    return {v.x / len, v.y / len};
}

} // namespace

const EnemyStats& enemyStats(EnemyType type)
{
    return kEnemyStats[static_cast<size_t>(type)];
}

Enemy::Enemy(EnemyType type, std::string id, Vector2 tileCenter)
    : type_(type), id_(std::move(id))
{
    const EnemyStats& s = enemyStats(type_);
    size = s.size;
    maxHp = s.maxHp;
    hp = maxHp;
    speed = s.speed;
    position = Vector2{tileCenter.x - size.x * 0.5f, tileCenter.y - size.y * 0.5f};
}

const EnemyStats& Enemy::stats() const
{
    return enemyStats(type_);
}

void Enemy::update(float dt, const TileMap& map, Vector2 playerCenter)
{
    if (!alive) return;

    if (hurtTimer_ > 0.0f) hurtTimer_ -= dt;
    if (attackCooldown_ > 0.0f) attackCooldown_ -= dt;
    if (knockbackTimer_ > 0.0f) knockbackTimer_ -= dt;

    Vector2 dir{0.0f, 0.0f};
    const Vector2 myCenter = center();
    const float dx = playerCenter.x - myCenter.x;
    const float dy = playerCenter.y - myCenter.y;
    const float dist = std::sqrt(dx * dx + dy * dy);

    float moveSpeed = speed;
    if (dist < stats().aggroRange) {
        dir = normalizeOrZero(Vector2{dx, dy});
        // O Guardião fica mais rápido quando está ferido (fase de fúria).
        if (type_ == EnemyType::Guardian && hp < maxHp / 2) moveSpeed *= 1.4f;
    } else {
        wanderTimer_ -= dt;
        if (wanderTimer_ <= 0.0f) {
            wanderTimer_ = 1.2f + (float)(GetRandomValue(0, 100)) / 100.0f;
            const int choice = GetRandomValue(0, 4);
            wanderDir_ = (choice == 4) ? Vector2{0.0f, 0.0f}
                                       : directionVector(static_cast<Direction>(choice));
        }
        dir = wanderDir_;
        moveSpeed *= 0.45f;
    }

    if (knockbackTimer_ > 0.0f) moveSpeed = 0.0f; // recuo após levar golpe

    if (dir.x != 0.0f || dir.y != 0.0f) {
        const Vector2 delta = Vector2{dir.x * moveSpeed * dt, dir.y * moveSpeed * dt};
        position = map.moveWithCollision(position, size, delta);
    }
}

void Enemy::takeDamage(int amount, Vector2 knockbackDir)
{
    if (!alive) return;
    hp -= amount;
    hurtTimer_ = 0.18f;

    // Pequeno recuo para dar sensação de impacto.
    const Vector2 n = normalizeOrZero(knockbackDir);
    position.x += n.x * 10.0f;
    position.y += n.y * 10.0f;
    knockbackTimer_ = 0.12f;

    if (hp <= 0) {
        hp = 0;
        alive = false;
    }
}

void Enemy::draw(float time) const
{
    if (!alive) return;

    const Rectangle r = bounds();
    const float flash = hurtTimer_ > 0.0f ? 1.0f : 0.0f;
    const float squash = 1.0f + 0.06f * (float)sin(time * 6.0);

    // Sombra
    DrawEllipse((int)center().x, (int)(r.y + r.height) + 2, r.width * 0.55f, 5.0f, Color{0, 0, 0, 55});

    switch (type_) {
    case EnemyType::Slime: {
        const Color body = flash ? WHITE : Color{80, 190, 90, 255};
        DrawEllipse((int)center().x, (int)(r.y + r.height * 0.62f),
                    r.width * 0.5f, r.height * 0.42f * squash, body);
        DrawCircle((int)(center().x - 5), (int)(r.y + r.height * 0.45f), 2.5f, Color{20, 60, 30, 255});
        DrawCircle((int)(center().x + 5), (int)(r.y + r.height * 0.45f), 2.5f, Color{20, 60, 30, 255});
        DrawEllipse((int)(center().x - 4), (int)(r.y + r.height * 0.35f), 5.0f, 3.0f,
                    Color{255, 255, 255, 90});
        break;
    }
    case EnemyType::Goblin:
    case EnemyType::GoblinLeader: {
        const bool leader = (type_ == EnemyType::GoblinLeader);
        const Color skin = flash ? WHITE
                                 : (leader ? Color{180, 110, 50, 255} : Color{110, 160, 70, 255});
        const Color tunic = leader ? Color{120, 40, 40, 255} : Color{70, 50, 40, 255};
        // Corpo
        DrawRectangleRounded(r, 0.3f, 5, tunic);
        // Cabeça
        DrawCircle((int)center().x, (int)(r.y + 9.0f), 10.0f, skin);
        // Orelhas pontudas
        DrawTriangle(Vector2{r.x + 2.0f, r.y + 4.0f}, Vector2{r.x - 5.0f, r.y - 2.0f},
                     Vector2{r.x + 4.0f, r.y + 10.0f}, skin);
        DrawTriangle(Vector2{r.x + r.width - 2.0f, r.y + 4.0f},
                     Vector2{r.x + r.width + 5.0f, r.y - 2.0f},
                     Vector2{r.x + r.width - 4.0f, r.y + 10.0f}, skin);
        // Olhos
        DrawCircle((int)(center().x - 4), (int)(r.y + 8.0f), 2.0f, Color{20, 20, 20, 255});
        DrawCircle((int)(center().x + 4), (int)(r.y + 8.0f), 2.0f, Color{20, 20, 20, 255});
        if (leader) {
            // Coroa simples do líder
            DrawRectangle((int)(r.x + 3.0f), (int)(r.y - 3.0f), (int)(r.width - 6.0f), 5, GOLD);
            DrawCircle((int)center().x, (int)(r.y - 4.0f), 3.0f, GOLD);
        }
        break;
    }
    case EnemyType::Guardian: {
        const Color body = flash ? WHITE : Color{70, 40, 110, 255};
        const Color crystal = Color{160, 120, 255, 255};
        // Corpo rochoso
        DrawRectangleRounded(r, 0.25f, 6, body);
        // Cristais nas costas
        DrawTriangle(Vector2{r.x + 4.0f, r.y + 10.0f}, Vector2{r.x + 10.0f, r.y - 8.0f},
                     Vector2{r.x + 14.0f, r.y + 12.0f}, crystal);
        DrawTriangle(Vector2{r.x + r.width - 14.0f, r.y + 12.0f},
                     Vector2{r.x + r.width - 10.0f, r.y - 8.0f},
                     Vector2{r.x + r.width - 4.0f, r.y + 10.0f}, crystal);
        // Olhos brilhantes
        const int eyeY = (int)(r.y + 14.0f);
        DrawCircle((int)(center().x - 8), eyeY, 4.0f, Color{255, 230, 120, 255});
        DrawCircle((int)(center().x + 8), eyeY, 4.0f, Color{255, 230, 120, 255});
        DrawCircle((int)(center().x - 8), eyeY, 1.8f, Color{40, 20, 10, 255});
        DrawCircle((int)(center().x + 8), eyeY, 1.8f, Color{40, 20, 10, 255});
        break;
    }
    }

    // Barra de vida (sempre para o chefe, ao levar dano para os demais).
    if (hp < maxHp || type_ == EnemyType::Guardian) {
        const int barW = (int)r.width + 6;
        const int barX = (int)center().x - barW / 2;
        const int barY = (int)r.y - 10;
        DrawRectangle(barX, barY, barW, 5, Color{20, 20, 20, 200});
        DrawRectangle(barX, barY, (int)(barW * (float)hp / (float)maxHp), 5, Color{200, 60, 60, 255});
    }
}
