// Areas.h - definição das áreas do mundo: mapa, paleta, pontos de spawn e transições.
#pragma once

#include <map>
#include <string>
#include <vector>

#include "world/TileMap.h"

// Uma zona de transição leva o jogador a outra área quando ele a atravessa.
struct TransitionZone {
    Rectangle rect;          // em pixels, no espaço do mundo
    std::string targetArea;  // id da área de destino
    std::string targetSpawn; // nome do spawn na área de destino
    std::string message;     // exibida ao chegar
};

struct AreaDef {
    std::string name;        // id estável (usado no salvamento)
    std::string displayName; // nome mostrado na interface
    std::string mapFile;     // caminho do arquivo ASCII do mapa
    AreaTheme theme;
    std::map<std::string, Vector2> spawns; // nome → centro do spawn, em pixels
    std::vector<TransitionZone> transitions;
};

// Constrói as três áreas do jogo (vila, floresta e caverna).
std::vector<AreaDef> buildAreaDefs();

// Procura uma área pelo id; devolve nullptr se não existir.
const AreaDef* findAreaDef(const std::vector<AreaDef>& areas, const std::string& name);

// Centro do tile (tx, ty) em pixels.
Vector2 tileCenter(int tx, int ty);
