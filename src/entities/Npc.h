// Npc.h - personagens não jogáveis (diálogos e missões).
#pragma once

#include <string>

#include "entities/Entity.h"

enum class NpcKind { Elder, Villager };

class Npc {
public:
    Npc(NpcKind kind, std::string id, Vector2 tileCenter);

    const std::string& id() const { return id_; }
    NpcKind kind() const { return kind_; }
    const char* displayName() const;

    Rectangle bounds() const { return entity_.bounds(); }
    Vector2 center() const { return entity_.center(); }
    const Entity& entity() const { return entity_; }

    void draw(float time) const;

private:
    NpcKind kind_;
    std::string id_;
    Entity entity_;
};
