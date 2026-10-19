// Inventory.h - itens coletáveis e inventário do jogador.
#pragma once

#include <array>
#include <cstddef>

enum class ItemType : int {
    Potion = 0, // Poção de Vida: restaura pontos de vida
    Coin,       // Moedas da vila
    Key,        // Chave Antiga: abre o portão da caverna
    Flower,     // Flor de Lumina: objeto da missão principal
    Count
};

struct ItemInfo {
    ItemType type;
    const char* name;
    const char* description;
    bool usable; // pode ser usado pelo jogador (ex.: poção)
};

const ItemInfo& itemInfo(ItemType type);

class Inventory {
public:
    int count(ItemType type) const;
    void add(ItemType type, int amount = 1);
    bool remove(ItemType type, int amount = 1); // devolve false se não houver itens suficientes
    bool has(ItemType type) const;
    void clear();

    const std::array<int, static_cast<size_t>(ItemType::Count)>& all() const { return counts_; }

private:
    std::array<int, static_cast<size_t>(ItemType::Count)> counts_{};
};
