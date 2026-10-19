// ItemPickup.h - itens coletáveis espalhados pelo mapa.
#pragma once

#include <string>

#include "entities/Entity.h"
#include "systems/Inventory.h"

class ItemPickup {
public:
    ItemPickup(ItemType type, std::string id, Vector2 tileCenter, int amount = 1);

    ItemType type() const { return type_; }
    const std::string& id() const { return id_; }
    int amount() const { return amount_; }

    Rectangle bounds() const;
    Vector2 center() const;

    void draw(float time) const;

private:
    ItemType type_;
    std::string id_;
    Vector2 position_;
    int amount_;
    float bobPhase_;
};
