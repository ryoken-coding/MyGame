// World.h - gerencia a área atual: mapa, inimigos, NPCs e itens coletáveis.
#pragma once

#include <set>
#include <string>
#include <vector>

#include "entities/Enemy.h"
#include "entities/ItemPickup.h"
#include "entities/Npc.h"
#include "world/Areas.h"
#include "world/TileMap.h"

class World {
public:
    void setAreaDefs(std::vector<AreaDef> areas) { areas_ = std::move(areas); }
    const std::vector<AreaDef>& areaDefs() const { return areas_; }

    // Carrega a área e gera as entidades dos marcadores do mapa.
    // Entidades removidas (marcadas em flags) não voltam a aparecer.
    bool loadArea(const std::string& areaName, std::string& error);

    const std::string& areaName() const { return areaName_; }
    const AreaDef& areaDef() const { return *areaDef_; }
    TileMap& map() { return map_; }
    const TileMap& map() const { return map_; }

    std::vector<Enemy>& enemies() { return enemies_; }
    std::vector<Npc>& npcs() { return npcs_; }
    std::vector<ItemPickup>& pickups() { return pickups_; }

    // Flags persistentes: inimigos derrotados, itens coletados, portão aberto, etc.
    std::set<std::string>& worldFlags() { return flags_; }
    const std::set<std::string>& worldFlags() const { return flags_; }
    bool hasFlag(const std::string& flag) const { return flags_.count(flag) > 0; }
    void setFlag(const std::string& flag) { flags_.insert(flag); }

    void addPickup(ItemPickup pickup) { pickups_.push_back(std::move(pickup)); }

    // Garante que a Flor de Lumina continue disponível na arena do chefe
    // enquanto não for coletada (o drop normal some ao sair da área).
    void spawnQuestDropIfNeeded(bool bossDefeated, bool flowerTaken);

    // Atualiza a IA dos inimigos da área.
    void update(float dt, Vector2 playerCenter);

    void draw(float time, Vector2 playerCenter) const;

    // Retorna a transição sob o jogador (ou nullptr).
    const TransitionZone* transitionAt(Rectangle playerBounds) const;

    // Posição de um spawn pelo nome; se não existir, o centro do mapa.
    Vector2 spawnPosition(const std::string& spawnName) const;

    // Flag estável de uma entidade gerada pelo mapa (ex.: "floresta:10:8").
    static std::string entityId(const std::string& areaName, int tx, int ty);
    static const char* kGateFlag; // flag do portão da caverna

private:
    std::vector<AreaDef> areas_;
    const AreaDef* areaDef_ = nullptr;
    std::string areaName_;
    TileMap map_;
    std::vector<Enemy> enemies_;
    std::vector<Npc> npcs_;
    std::vector<ItemPickup> pickups_;
    std::set<std::string> flags_;

    void spawnEntitiesFromMarkers();
};
