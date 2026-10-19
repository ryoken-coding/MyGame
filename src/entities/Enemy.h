// Enemy.h - inimigos com comportamento básico de perseguição e ataque por contato.
#pragma once

#include <string>

#include "entities/Entity.h"

class TileMap;

enum class EnemyType { Slime, Goblin, GoblinLeader, Guardian };

struct EnemyStats {
    const char* name;
    int maxHp;
    float speed;
    int contactDamage;
    float aggroRange;       // distância para começar a perseguir o jogador
    float attackCooldown;   // intervalo entre danos por contato
    Vector2 size;
};

const EnemyStats& enemyStats(EnemyType type);

class Enemy : public Entity {
public:
    Enemy(EnemyType type, std::string id, Vector2 tileCenter);

    EnemyType type() const { return type_; }
    const std::string& id() const { return id_; }
    const EnemyStats& stats() const;

    // IA: perseguir o jogador quando próximo, senão vaguear.
    void update(float dt, const TileMap& map, Vector2 playerCenter);
    void takeDamage(int amount, Vector2 knockbackDir);

    bool readyToAttack() const { return attackCooldown_ <= 0.0f; }
    void markAttackDone() { attackCooldown_ = stats().attackCooldown; }

    void draw(float time) const;

private:
    EnemyType type_;
    std::string id_;
    float hurtTimer_ = 0.0f;      // branco ao levar dano
    float attackCooldown_ = 0.0f;
    float wanderTimer_ = 0.0f;
    Vector2 wanderDir_{0.0f, 0.0f};
    float knockbackTimer_ = 0.0f;
};
