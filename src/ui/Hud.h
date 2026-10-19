// Hud.h - interface: HUD, caixa de diálogo e telas de menu.
#pragma once

#include <string>
#include <vector>

#include "systems/Dialogue.h"
#include "systems/Quest.h"
#include "entities/Player.h"

namespace ui {

// HUD durante o jogo: vida, inventário e rastreador da missão.
void drawHud(const Player& player, const QuestStatus& quest);

// Caixa de diálogo na parte inferior da tela.
void drawDialogueBox(const DialogueSystem& dialogue);

// Dica de interação centralizada embaixo (ex.: "[E] Falar com Ancião").
void drawInteractionHint(const std::string& text);

// Lista de menu vertical centralizada. `enabled` (opcional) deixa itens indisponíveis em cinza.
void drawMenu(const std::vector<std::string>& items, int selected, int centerY,
              const std::vector<bool>* enabled = nullptr);

void drawTitleScreen(const std::vector<std::string>& items, int selected, const std::string& notice);
void drawPauseOverlay(const std::vector<std::string>& items, int selected);
void drawGameOverScreen(const std::vector<std::string>& items, int selected);
void drawVictoryScreen(const std::vector<std::string>& items, int selected, int coins, int enemiesDefeated);

} // namespace ui
