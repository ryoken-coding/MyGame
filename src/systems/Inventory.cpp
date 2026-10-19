#include "systems/Inventory.h"

namespace {

const ItemInfo kItemInfo[] = {
    {ItemType::Potion, "Poção de Vida", "Restaura 35 pontos de vida.", true},
    {ItemType::Coin, "Moeda", "Moedas de ouro da vila de Pedra Clara.", false},
    {ItemType::Key, "Chave Antiga", "Abre o portão de pedra da caverna.", false},
    {ItemType::Flower, "Flor de Lumina", "O coração sagrado do bosque.", false},
};

static_assert(sizeof(kItemInfo) / sizeof(kItemInfo[0]) == static_cast<size_t>(ItemType::Count),
              "Tabela de itens desatualizada");

} // namespace

const ItemInfo& itemInfo(ItemType type)
{
    return kItemInfo[static_cast<size_t>(type)];
}

int Inventory::count(ItemType type) const
{
    return counts_[static_cast<size_t>(type)];
}

void Inventory::add(ItemType type, int amount)
{
    int& slot = counts_[static_cast<size_t>(type)];
    slot += amount;
    if (slot < 0) slot = 0;
}

bool Inventory::remove(ItemType type, int amount)
{
    int& slot = counts_[static_cast<size_t>(type)];
    if (slot < amount) return false;
    slot -= amount;
    return true;
}

bool Inventory::has(ItemType type) const
{
    return count(type) > 0;
}

void Inventory::clear()
{
    counts_.fill(0);
}
