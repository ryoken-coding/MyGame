// SaveSystem.h - salvamento e carregamento do progresso em JSON.
#pragma once

#include <string>
#include <vector>

#include "systems/Quest.h"

// Todos os dados persistidos de uma partida.
struct SaveData {
    std::string area = "vila";
    float playerX = 0.0f;
    float playerY = 0.0f;
    int hp = 100;
    int maxHp = 100;

    int potions = 0;
    int coins = 0;
    bool key = false;
    bool flower = false;

    QuestStatus quest;
    std::vector<std::string> worldFlags; // inimigos derrotados, itens coletados, portão...
};

class SaveSystem {
public:
    static const char* defaultPath(); // "saves/savegame.json"

    bool save(const SaveData& data, std::string& error, const std::string& path = defaultPath());
    bool load(SaveData& out, std::string& error, const std::string& path = defaultPath());
    bool exists(const std::string& path = defaultPath());
};
