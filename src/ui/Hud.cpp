#include "ui/Hud.h"

#include <cmath>

#include "raylib.h"
#include "util/Strings.h"

namespace ui {

namespace {

void drawCentered(const std::string& text, int y, int fontSize, Color color)
{
    const int width = MeasureText(text.c_str(), fontSize);
    DrawText(text.c_str(), (GetScreenWidth() - width) / 2, y, fontSize, color);
}

void drawPanel(Rectangle rect, unsigned char alpha = 210)
{
    DrawRectangleRounded(rect, 0.12f, 6, Color{18, 18, 28, alpha});
    DrawRectangleRoundedLines(rect, 0.12f, 6, Color{220, 200, 140, (unsigned char)(alpha + 30)});
}

} // namespace

void drawHud(const Player& player, const QuestStatus& quest)
{
    const int pad = 16;

    // ---------------------------------------------------------- Painel de vida
    drawPanel(Rectangle{(float)pad, (float)pad, 260.0f, 96.0f});

    DrawText("Vida", pad + 14, pad + 10, 18, Color{230, 220, 200, 255});
    DrawRectangle(pad + 14, pad + 36, 232, 18, Color{40, 30, 35, 255});
    const float hpRatio = player.maxHp > 0 ? (float)player.hp / (float)player.maxHp : 0.0f;
    Color hpColor = hpRatio > 0.5f ? Color{80, 200, 90, 255}
                                   : (hpRatio > 0.25f ? Color{230, 180, 60, 255}
                                                      : Color{210, 70, 70, 255});
    DrawRectangle(pad + 14, pad + 36, (int)(232 * hpRatio), 18, hpColor);
    DrawRectangleLines(pad + 14, pad + 36, 232, 18, Color{15, 12, 16, 255});
    DrawText(util::formatText("%d / %d", player.hp, player.maxHp).c_str(), pad + 14, pad + 58, 18,
             Color{235, 230, 215, 255});

    // Inventário resumido: poções, moedas e chave.
    const std::string potions = util::formatText("Poções: %d [Q]", player.inventory.count(ItemType::Potion));
    DrawText(potions.c_str(), pad + 120, pad + 58, 18, Color{230, 160, 170, 255});

    const std::string coins = util::formatText("Moedas: %d", player.inventory.count(ItemType::Coin));
    DrawText(coins.c_str(), pad + 14, pad + 76, 18, Color{235, 210, 120, 255});

    if (player.inventory.has(ItemType::Key)) {
        DrawText("Chave Antiga", pad + 120, pad + 76, 18, Color{220, 200, 140, 255});
    }

    // ------------------------------------------------- Rastreador da missão
    const char* title = "Missão: A Flor de Lumina";
    const int questWidth = 360;
    const int questX = GetScreenWidth() - questWidth - pad;
    drawPanel(Rectangle{(float)questX, (float)pad, (float)questWidth, 78.0f});
    DrawText(title, questX + 14, pad + 10, 18, Color{240, 220, 150, 255});

    const auto lines = util::wrapText(quest.trackerLine(), questWidth - 28, 16);
    int lineY = pad + 36;
    for (const std::string& line : lines) {
        DrawText(line.c_str(), questX + 14, lineY, 16, Color{210, 205, 190, 255});
        lineY += 18;
    }
}

void drawDialogueBox(const DialogueSystem& dialogue)
{
    if (!dialogue.active()) return;

    const int width = GetScreenWidth() - 120;
    const int height = 150;
    const int x = (GetScreenWidth() - width) / 2;
    const int y = GetScreenHeight() - height - 24;

    drawPanel(Rectangle{(float)x, (float)y, (float)width, (float)height}, 230);

    // Nome de quem fala
    DrawText(dialogue.speaker().c_str(), x + 20, y + 12, 22, Color{240, 220, 150, 255});
    DrawText(util::formatText("(%d/%d)", dialogue.pageIndex(), dialogue.pageCount()).c_str(),
             x + width - 90, y + 14, 18, Color{180, 175, 160, 255});

    const auto lines = util::wrapText(dialogue.currentPage(), width - 40, 20);
    int lineY = y + 44;
    for (const std::string& line : lines) {
        DrawText(line.c_str(), x + 20, lineY, 20, Color{235, 232, 220, 255});
        lineY += 24;
    }

    DrawText("[E] continuar", x + width - 150, y + height - 26, 16, Color{180, 175, 160, 255});
}

void drawInteractionHint(const std::string& text)
{
    const int width = MeasureText(text.c_str(), 20) + 28;
    const int x = (GetScreenWidth() - width) / 2;
    const int y = GetScreenHeight() - 52;
    DrawRectangleRounded(Rectangle{(float)x, (float)y, (float)width, 34.0f}, 0.4f, 6,
                         Color{18, 18, 28, 190});
    DrawText(text.c_str(), x + 14, y + 7, 20, Color{240, 235, 215, 255});
}

void drawMenu(const std::vector<std::string>& items, int selected, int centerY,
              const std::vector<bool>* enabled)
{
    const int itemHeight = 42;
    const int totalHeight = (int)items.size() * itemHeight;
    int y = centerY - totalHeight / 2;

    for (size_t i = 0; i < items.size(); i++) {
        const bool isSelected = (int)i == selected;
        const bool isEnabled = (enabled == nullptr) || (*enabled)[i];

        const int width = MeasureText(items[i].c_str(), 26) + 48;
        const int x = (GetScreenWidth() - width) / 2;

        if (isSelected) {
            DrawRectangleRounded(Rectangle{(float)x, (float)y - 4, (float)width, 36.0f}, 0.35f, 6,
                                 Color{220, 190, 110, 60});
            DrawText(">", x + 10, y, 26, Color{240, 220, 140, 255});
        }

        const Color color = !isEnabled ? Color{110, 105, 100, 255}
                                       : (isSelected ? Color{250, 240, 200, 255}
                                                     : Color{200, 195, 180, 255});
        DrawText(items[i].c_str(), x + 34, y, 26, color);
        y += itemHeight;
    }
}

void drawTitleScreen(const std::vector<std::string>& items, int selected, const std::string& notice)
{
    ClearBackground(Color{18, 22, 38, 255});

    // Faixa decorativa de "montanhas" simples ao fundo.
    for (int i = 0; i < 6; i++) {
        const int h = 120 + (i * 37) % 90;
        DrawRectangle(i * (GetScreenWidth() / 5) - 60, GetScreenHeight() - h, GetScreenWidth() / 4, h,
                      Color{28, 34, 56, 255});
    }

    drawCentered("E L D A R A", 96, 64, Color{240, 225, 160, 255});
    drawCentered("A Flor de Lumina", 172, 30, Color{180, 200, 235, 255});
    drawCentered("Uma pequena aventura de fantasia em 2D", 212, 18, Color{150, 160, 190, 255});

    drawMenu(items, selected, GetScreenHeight() / 2 + 40);

    drawCentered("Mover: WASD ou setas   Atacar: Espaço   Interagir: E   Pausar: Esc",
                 GetScreenHeight() - 64, 18, Color{160, 165, 185, 255});

    if (!notice.empty()) {
        drawCentered(notice, GetScreenHeight() - 100, 18, Color{240, 160, 140, 255});
    }
}

void drawPauseOverlay(const std::vector<std::string>& items, int selected)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{10, 10, 20, 150});
    drawCentered("PAUSADO", 120, 42, Color{240, 235, 215, 255});
    drawMenu(items, selected, GetScreenHeight() / 2 + 20);
}

void drawGameOverScreen(const std::vector<std::string>& items, int selected)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{40, 8, 12, 190});
    drawCentered("VOCÊ FOI DERROTADO", 150, 44, Color{230, 90, 90, 255});
    drawCentered("O bosque ainda espera por um herói...", 210, 22, Color{220, 190, 180, 255});
    drawMenu(items, selected, GetScreenHeight() / 2 + 40);
}

void drawVictoryScreen(const std::vector<std::string>& items, int selected, int coins, int enemiesDefeated)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{20, 30, 24, 200});
    drawCentered("VITÓRIA!", 120, 52, Color{240, 225, 150, 255});
    drawCentered("A Flor de Lumina voltou ao bosque e a vila foi salva.", 190, 22,
                 Color{210, 225, 200, 255});

    const std::string stats = util::formatText("Moedas coletadas: %d    Inimigos derrotados: %d",
                                               coins, enemiesDefeated);
    drawCentered(stats, 236, 20, Color{190, 205, 185, 255});
    drawCentered("Obrigado por jogar Eldara: A Flor de Lumina!", 272, 20, Color{190, 205, 185, 255});

    drawMenu(items, selected, GetScreenHeight() / 2 + 60);
}

} // namespace ui
