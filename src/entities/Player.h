// Player.h - jogador: movimento em quatro direções, ataque, dano e invulnerabilidade breve.
#pragma once

#include "entities/Entity.h"
#include "systems/Inventory.h"

class TileMap;

class Player : public Entity {
public:
    Player();

    void reset();
    // moveDir deve estar normalizado ou ser o vetor zero; resolve colisão com o mapa.
    void applyMove(float dt, const TileMap& map, Vector2 moveDir);
    void updateTimers(float dt);

    void startAttack();
    bool isAttacking() const { return attackTimer_ > 0.0f; }
    Rectangle attackHitbox() const; // área do golpe à frente do jogador

    void takeDamage(int amount, Vector2 fromPos);
    void heal(int amount);

    Direction facing() const { return facing_; }
    bool isInvulnerable() const { return invulnTimer_ > 0.0f; }

    void draw(float time) const;

    Inventory inventory;

    static constexpr float kAttackDuration = 0.22f;
    static constexpr float kAttackCooldown = 0.38f;
    static constexpr float kInvulnerabilityTime = 0.9f;
    static constexpr int kAttackDamage = 6;

private:
    Direction facing_ = Direction::Down;
    float attackTimer_ = 0.0f;
    float attackCooldown_ = 0.0f;
    float invulnTimer_ = 0.0f;
};
