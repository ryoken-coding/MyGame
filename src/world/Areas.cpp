#include "world/Areas.h"

namespace {

AreaTheme makeVillageTheme()
{
    return AreaTheme{
        "vila",
        Color{106, 170, 90, 255},  // grass
        Color{88, 150, 74, 255},   // grassAlt
        Color{196, 168, 122, 255}, // path
        Color{170, 142, 100, 255}, // pathAlt
        Color{70, 130, 190, 255},  // water
        Color{110, 170, 220, 255}, // waterAlt
        Color{110, 75, 45, 255},   // treeTrunk
        Color{52, 120, 55, 255},   // treeLeaf
        Color{70, 145, 65, 255},   // treeLeafAlt
        Color{130, 130, 135, 255}, // rock
        Color{165, 165, 175, 255}, // rockAlt
        Color{168, 130, 104, 255}, // wall (casas de madeira)
        Color{128, 96, 76, 255},   // wallAlt
        Color{230, 120, 140, 255}, // flowerA
        Color{240, 210, 90, 255},  // flowerB
        Color{100, 70, 45, 255},   // gate
        0.0f,
    };
}

AreaTheme makeForestTheme()
{
    return AreaTheme{
        "floresta",
        Color{70, 130, 62, 255},
        Color{58, 110, 52, 255},
        Color{156, 130, 92, 255},
        Color{132, 108, 76, 255},
        Color{50, 100, 150, 255},
        Color{85, 140, 190, 255},
        Color{95, 62, 38, 255},
        Color{36, 92, 42, 255},
        Color{52, 112, 50, 255},
        Color{115, 115, 122, 255},
        Color{150, 150, 160, 255},
        Color{120, 92, 70, 255},
        Color{90, 68, 52, 255},
        Color{210, 110, 130, 255},
        Color{225, 195, 85, 255},
        Color{85, 60, 40, 255},
        0.0f,
    };
}

AreaTheme makeCaveTheme()
{
    return AreaTheme{
        "caverna",
        Color{92, 86, 98, 255},
        Color{78, 72, 86, 255},
        Color{110, 100, 92, 255},
        Color{92, 84, 78, 255},
        Color{40, 70, 110, 255},
        Color{60, 100, 150, 255},
        Color{60, 50, 48, 255},
        Color{70, 62, 80, 255},
        Color{88, 78, 100, 255},
        Color{105, 100, 112, 255},
        Color{135, 130, 150, 255},
        Color{52, 46, 62, 255},
        Color{38, 34, 48, 255},
        Color{140, 210, 255, 255}, // cristais
        Color{180, 150, 255, 255},
        Color{70, 55, 45, 255},
        0.30f,
    };
}

} // namespace

Vector2 tileCenter(int tx, int ty)
{
    return Vector2{tx * (float)TileMap::kTileSize + TileMap::kTileSize * 0.5f,
                   ty * (float)TileMap::kTileSize + TileMap::kTileSize * 0.5f};
}

std::vector<AreaDef> buildAreaDefs()
{
    std::vector<AreaDef> areas;

    // ------------------------------------------------------------------ Vila
    {
        AreaDef vila;
        vila.name = "vila";
        vila.displayName = "Vila de Pedra Clara";
        vila.mapFile = "assets/maps/vila.txt";
        vila.theme = makeVillageTheme();
        vila.spawns["start"] = tileCenter(18, 18);
        vila.spawns["from_floresta"] = tileCenter(35, 12);
        vila.transitions.push_back(TransitionZone{
            Rectangle{38.0f * TileMap::kTileSize, 10.0f * TileMap::kTileSize,
                      2.5f * TileMap::kTileSize, 4.0f * TileMap::kTileSize},
            "floresta", "from_vila", "Você entrou na Floresta Sombria."});
        areas.push_back(std::move(vila));
    }

    // --------------------------------------------------------------- Floresta
    {
        AreaDef floresta;
        floresta.name = "floresta";
        floresta.displayName = "Floresta Sombria";
        floresta.mapFile = "assets/maps/floresta.txt";
        floresta.theme = makeForestTheme();
        floresta.spawns["from_vila"] = tileCenter(3, 12);
        floresta.spawns["from_caverna"] = tileCenter(19, 3);
        floresta.transitions.push_back(TransitionZone{
            Rectangle{-1.5f * TileMap::kTileSize, 10.0f * TileMap::kTileSize,
                      3.0f * TileMap::kTileSize, 4.0f * TileMap::kTileSize},
            "vila", "from_floresta", "Você voltou para a Vila de Pedra Clara."});
        floresta.transitions.push_back(TransitionZone{
            Rectangle{17.0f * TileMap::kTileSize, -0.5f * TileMap::kTileSize,
                      5.0f * TileMap::kTileSize, 1.5f * TileMap::kTileSize},
            "caverna", "from_floresta", "Você entrou na Caverna do Guardião."});
        areas.push_back(std::move(floresta));
    }

    // --------------------------------------------------------------- Caverna
    {
        AreaDef caverna;
        caverna.name = "caverna";
        caverna.displayName = "Caverna do Guardião";
        caverna.mapFile = "assets/maps/caverna.txt";
        caverna.theme = makeCaveTheme();
        caverna.spawns["from_floresta"] = tileCenter(17, 16);
        caverna.spawns["boss"] = tileCenter(17, 4);
        caverna.transitions.push_back(TransitionZone{
            Rectangle{14.0f * TileMap::kTileSize, 18.0f * TileMap::kTileSize,
                      5.0f * TileMap::kTileSize, 2.0f * TileMap::kTileSize},
            "floresta", "from_caverna", "Você saiu da caverna."});
        areas.push_back(std::move(caverna));
    }

    return areas;
}

const AreaDef* findAreaDef(const std::vector<AreaDef>& areas, const std::string& name)
{
    for (const AreaDef& area : areas) {
        if (area.name == name) return &area;
    }
    return nullptr;
}
