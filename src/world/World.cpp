#include "world/World.h"

#include <cmath>

#include "util/Strings.h"

const char* World::kGateFlag = "portao_aberto";

std::string World::entityId(const std::string& areaName, int tx, int ty)
{
    return util::formatText("%s:%d:%d", areaName.c_str(), tx, ty);
}

bool World::loadArea(const std::string& areaName, std::string& error)
{
    error.clear();

    const AreaDef* def = findAreaDef(areas_, areaName);
    if (def == nullptr) {
        error = util::formatText("Área desconhecida: '%s'", areaName.c_str());
        return false;
    }

    TileMap map;
    if (!map.loadFromFile(def->mapFile, error)) return false;

    map_ = std::move(map);
    areaDef_ = def;
    areaName_ = areaName;
    enemies_.clear();
    npcs_.clear();
    pickups_.clear();

    // O portão permanece aberto depois de ser destrancado.
    map_.setGatesOpen(hasFlag(kGateFlag));

    spawnEntitiesFromMarkers();
    return true;
}

void World::spawnEntitiesFromMarkers()
{
    for (const SpawnMarker& marker : map_.spawnMarkers()) {
        const Vector2 center = tileCenter(marker.tx, marker.ty);
        const std::string id = entityId(areaName_, marker.tx, marker.ty);

        switch (marker.marker) {
        case 'P': // spawn inicial do jogador não gera entidade
        case 'S': case 'G': case 'K': case 'B': // variações não usadas
            break;
        case 's':
            if (!hasFlag(id)) enemies_.push_back(Enemy(EnemyType::Slime, id, center));
            break;
        case 'g':
            if (!hasFlag(id)) enemies_.push_back(Enemy(EnemyType::Goblin, id, center));
            break;
        case 'k':
            if (!hasFlag(id)) enemies_.push_back(Enemy(EnemyType::GoblinLeader, id, center));
            break;
        case 'b':
            if (!hasFlag(id)) enemies_.push_back(Enemy(EnemyType::Guardian, id, center));
            break;
        case 'n':
            npcs_.push_back(Npc(NpcKind::Elder, id, center));
            break;
        case 'm':
            npcs_.push_back(Npc(NpcKind::Villager, id, center));
            break;
        case 'h':
            if (!hasFlag(id)) pickups_.push_back(ItemPickup(ItemType::Potion, id, center, 1));
            break;
        case 'c':
            if (!hasFlag(id)) {
                const int amount = 4 + (tileHash(marker.tx, marker.ty) % 9);
                pickups_.push_back(ItemPickup(ItemType::Coin, id, center, amount));
            }
            break;
        default:
            break;
        }
    }
}

void World::spawnQuestDropIfNeeded(bool bossDefeated, bool flowerTaken)
{
    if (areaName_ != "caverna") return;
    if (!bossDefeated || flowerTaken) return;
    if (hasFlag("flor_lumina")) return; // já coletada em outra visita

    for (const ItemPickup& pickup : pickups_) {
        if (pickup.type() == ItemType::Flower) return; // já existe no chão
    }

    auto it = areaDef_->spawns.find("boss");
    const Vector2 pos = (it != areaDef_->spawns.end()) ? it->second : tileCenter(17, 4);
    pickups_.push_back(ItemPickup(ItemType::Flower, "flor_lumina", pos, 1));
}

void World::update(float dt, Vector2 playerCenter)
{
    for (Enemy& enemy : enemies_) {
        enemy.update(dt, map_, playerCenter);
    }
}

void World::draw(float time, Vector2 playerCenter) const
{
    map_.draw(areaDef_->theme, time);

    for (const ItemPickup& pickup : pickups_) pickup.draw(time);
    for (const Npc& npc : npcs_) npc.draw(time);
    for (const Enemy& enemy : enemies_) enemy.draw(time);

    // Penumbra da caverna, com a luz suave ao redor do jogador.
    if (areaDef_->theme.ambientDarkness > 0.0f) {
        const unsigned char dark = (unsigned char)(255.0f * areaDef_->theme.ambientDarkness);
        DrawCircleGradient(playerCenter, 320.0f,
                           Color{10, 8, 20, 10},
                           Color{10, 8, 20, (unsigned char)(dark + 60)});
    }
}

const TransitionZone* World::transitionAt(Rectangle playerBounds) const
{
    for (const TransitionZone& zone : areaDef_->transitions) {
        if (CheckCollisionRecs(zone.rect, playerBounds)) return &zone;
    }
    return nullptr;
}

Vector2 World::spawnPosition(const std::string& spawnName) const
{
    auto it = areaDef_->spawns.find(spawnName);
    if (it != areaDef_->spawns.end()) return it->second;

    // Fallback seguro: centro do mapa.
    return Vector2{map_.widthPixels() * 0.5f, map_.heightPixels() * 0.5f};
}
