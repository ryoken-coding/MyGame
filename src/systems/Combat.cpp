#include "systems/Combat.h"

#include "raylib.h"
#include "util/Strings.h"

namespace combat {

void resolvePlayerAttack(Player& player, World& world, QuestStatus& quest, MessageLog& messages,
                         SoundBank& sounds, int& enemiesDefeated)
{
    player.startAttack();
    if (!player.isAttacking()) return; // ainda em recarga

    sounds.play(Sfx::Attack);
    const Rectangle hitbox = player.attackHitbox();

    for (Enemy& enemy : world.enemies()) {
        if (!enemy.alive) continue;
        if (!CheckCollisionRecs(hitbox, enemy.bounds())) continue;

        enemy.takeDamage(Player::kAttackDamage, directionVector(player.facing()));
        sounds.play(Sfx::Hit);
        if (enemy.alive) continue;

        // Inimigo derrotado: registra a flag (persistente) e gera as recompensas.
        world.setFlag(enemy.id());
        enemiesDefeated++;
        sounds.play(Sfx::EnemyDie);

        const Vector2 pos = enemy.center();
        const std::string dropBase = enemy.id() + ":drop";

        switch (enemy.type()) {
        case EnemyType::Slime:
            if (GetRandomValue(0, 99) < 45) {
                world.addPickup(ItemPickup(ItemType::Coin, dropBase, pos, 3));
            }
            if (GetRandomValue(0, 99) < 15) {
                world.addPickup(ItemPickup(ItemType::Potion, dropBase + ":p", pos, 1));
            }
            messages.add("Slime derrotado!");
            break;
        case EnemyType::Goblin:
            if (GetRandomValue(0, 99) < 55) {
                world.addPickup(ItemPickup(ItemType::Coin, dropBase, pos, 6));
            }
            if (GetRandomValue(0, 99) < 20) {
                world.addPickup(ItemPickup(ItemType::Potion, dropBase + ":p", pos, 1));
            }
            messages.add("Goblin derrotado!");
            break;
        case EnemyType::GoblinLeader:
            world.addPickup(ItemPickup(ItemType::Coin, dropBase, pos, 10));
            world.addPickup(ItemPickup(ItemType::Key, dropBase + ":chave", pos, 1));
            messages.add("O Goblin Líder caiu! Ele carregava a Chave Antiga.");
            break;
        case EnemyType::Guardian:
            quest.bossDefeated = true;
            world.addPickup(ItemPickup(ItemType::Coin, dropBase, pos, 25));
            world.addPickup(ItemPickup(ItemType::Flower, "flor_lumina", pos, 1));
            messages.add("O Guardião caiu! A Flor de Lumina surgiu dos escombros.");
            sounds.play(Sfx::Quest);
            break;
        }
    }
}

bool resolveEnemyContact(Player& player, World& world, SoundBank& sounds)
{
    for (Enemy& enemy : world.enemies()) {
        if (!enemy.alive) continue;
        if (!CheckCollisionRecs(player.bounds(), enemy.bounds())) continue;
        if (!enemy.readyToAttack()) continue;

        enemy.markAttackDone();
        player.takeDamage(enemy.stats().contactDamage, enemy.center());
        sounds.play(Sfx::PlayerHurt);

        if (!player.alive) return true;
    }
    return false;
}

} // namespace combat
