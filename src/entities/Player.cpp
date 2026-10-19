#include "entities/Player.h"

#include <cmath>

#include "world/TileMap.h"

namespace {

constexpr float kPlayerSpeed = 155.0f;
constexpr float kAttackReach = 34.0f;

} // namespace

Player::Player()
{
    reset();
}

void Player::reset()
{
    position = Vector2{0.0f, 0.0f};
    size = Vector2{22.0f, 28.0f};
    maxHp = 100;
    hp = maxHp;
    speed = kPlayerSpeed;
    alive = true;
    facing_ = Direction::Down;
    attackTimer_ = 0.0f;
    attackCooldown_ = 0.0f;
    invulnTimer_ = 0.0f;
    inventory.clear();
}

void Player::applyMove(float dt, const TileMap& map, Vector2 moveDir)
{
    if (!alive) return;

    // Atualiza a direção para o lado em que o jogador está se movendo.
    if (moveDir.x != 0.0f || moveDir.y != 0.0f) {
        if (std::fabs(moveDir.x) >= std::fabs(moveDir.y)) {
            facing_ = (moveDir.x > 0.0f) ? Direction::Right : Direction::Left;
        } else {
            facing_ = (moveDir.y > 0.0f) ? Direction::Down : Direction::Up;
        }
    }

    // Sem deslize diagonal: a direção já vem normalizada pelo chamador.
    const Vector2 delta = Vector2{moveDir.x * speed * dt, moveDir.y * speed * dt};
    position = map.moveWithCollision(position, size, delta);
}

void Player::updateTimers(float dt)
{
    if (attackTimer_ > 0.0f) attackTimer_ -= dt;
    if (attackCooldown_ > 0.0f) attackCooldown_ -= dt;
    if (invulnTimer_ > 0.0f) invulnTimer_ -= dt;
}

void Player::startAttack()
{
    if (!alive || attackCooldown_ > 0.0f) return;
    attackTimer_ = kAttackDuration;
    attackCooldown_ = kAttackCooldown;
}

Rectangle Player::attackHitbox() const
{
    const Vector2 dir = directionVector(facing_);
    const Vector2 c = center();
    const float half = kAttackReach * 0.5f;
    // Retângulo centrado na ponta do golpe, à frente do jogador.
    const float cx = c.x + dir.x * (size.x * 0.5f + half);
    const float cy = c.y + dir.y * (size.y * 0.5f + half);
    const float w = (dir.x != 0.0f) ? kAttackReach : size.x + 12.0f;
    const float h = (dir.y != 0.0f) ? kAttackReach : size.y + 12.0f;
    return Rectangle{cx - w * 0.5f, cy - h * 0.5f, w, h};
}

void Player::takeDamage(int amount, Vector2 fromPos)
{
    if (!alive || invulnTimer_ > 0.0f) return;

    hp -= amount;
    invulnTimer_ = kInvulnerabilityTime;

    // Recuo para longe da origem do dano.
    const Vector2 c = center();
    Vector2 dir{c.x - fromPos.x, c.y - fromPos.y};
    const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.001f) {
        dir.x /= len;
        dir.y /= len;
    }
    position.x += dir.x * 12.0f;
    position.y += dir.y * 12.0f;

    if (hp <= 0) {
        hp = 0;
        alive = false;
    }
}

void Player::heal(int amount)
{
    if (!alive) return;
    hp += amount;
    if (hp > maxHp) hp = maxHp;
}

void Player::draw(float time) const
{
    if (!alive) return;

    // Pisca enquanto está invulnerável.
    if (invulnTimer_ > 0.0f && ((int)(invulnTimer_ * 12.0f) % 2 == 0)) return;

    const Rectangle r = bounds();
    const Vector2 c = center();

    // Sombra
    DrawEllipse((int)c.x, (int)(r.y + r.height) + 2, 13.0f, 5.0f, Color{0, 0, 0, 60});

    // Pernas
    const float stride = 2.0f * (float)sin(time * 10.0);
    DrawRectangle((int)(r.x + 4.0f), (int)(r.y + r.height - 8.0f + stride * 0.5f), 5, 8, Color{60, 50, 90, 255});
    DrawRectangle((int)(r.x + r.width - 9.0f), (int)(r.y + r.height - 8.0f - stride * 0.5f), 5, 8, Color{60, 50, 90, 255});

    // Corpo (túnica)
    DrawRectangleRounded(Rectangle{r.x, r.y + 8.0f, r.width, r.height - 12.0f}, 0.35f, 6,
                         Color{40, 110, 190, 255});
    DrawRectangle((int)(r.x + r.width * 0.5f - 2.0f), (int)(r.y + 12.0f), 4, 12, Color{220, 200, 120, 255});

    // Cabeça
    DrawCircle((int)c.x, (int)(r.y + 6.0f), 10.0f, Color{230, 190, 150, 255});
    // Cabelo
    DrawCircleSector(Vector2{c.x, r.y + 3.0f}, 10.0f, 180.0f, 360.0f, 8, Color{90, 60, 40, 255});

    // Olhos conforme a direção do olhar
    const Vector2 look = directionVector(facing_);
    const int eyeY = (int)(r.y + 6.0f + look.y * 2.0f);
    const int eyeDx = (int)(look.x * 3.0f);
    DrawCircle((int)(c.x - 4 + eyeDx), eyeY, 1.8f, Color{25, 25, 35, 255});
    DrawCircle((int)(c.x + 4 + eyeDx), eyeY, 1.8f, Color{25, 25, 35, 255});

    // Arco do golpe de espada
    if (attackTimer_ > 0.0f) {
        const float t = 1.0f - attackTimer_ / kAttackDuration; // 0 → 1 durante o golpe
        const Vector2 dir = directionVector(facing_);
        const float baseAngle = std::atan2(dir.y, dir.x) * 180.0f / 3.14159265f;
        const float sweep = -55.0f + t * 110.0f;
        const unsigned char alpha = (unsigned char)(140 + 90 * (1.0f - t));
        DrawCircleSector(c, kAttackReach + 6.0f, baseAngle - 22.0f + sweep * 0.2f,
                         baseAngle + 22.0f + sweep * 0.2f, 10, Color{255, 240, 180, alpha});

        // Lâmina
        const Vector2 tip{c.x + dir.x * (kAttackReach + 14.0f), c.y + dir.y * (kAttackReach + 14.0f)};
        DrawLineEx(c, tip, 4.0f, Color{220, 225, 235, 255});
        DrawLineEx(c, tip, 1.5f, Color{255, 255, 255, 255});
    }
}
