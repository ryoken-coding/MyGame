// Combat.h - resolução dos golpes do jogador e do dano por contato dos inimigos.
#pragma once

#include "audio/SoundBank.h"
#include "entities/Player.h"
#include "systems/Quest.h"
#include "ui/MessageLog.h"
#include "world/World.h"

namespace combat {

// Aplica o golpe do jogador a todos os inimigos na área de ataque.
// Cuida de mortes, drops de itens e progresso da missão.
void resolvePlayerAttack(Player& player, World& world, QuestStatus& quest, MessageLog& messages,
                         SoundBank& sounds, int& enemiesDefeated);

// Dano por contato quando um inimigo encosta no jogador.
// Devolve true se o jogador morreu.
bool resolveEnemyContact(Player& player, World& world, SoundBank& sounds);

} // namespace combat
