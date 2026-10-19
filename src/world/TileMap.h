// TileMap.h - mapa em tiles: carregamento de arquivo ASCII, colisão e desenho.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "raylib.h"

// Tipos de tile do mapa. A legenda completa está no README e nos arquivos de mapa.
enum class Tile : uint8_t {
    Grass,  // grama (chão)
    Path,   // terra batida (chão)
    Water,  // água (sólido)
    Tree,   // árvore (sólido)
    Rock,   // rocha (sólido)
    Wall,   // alvenaria/parede (sólido)
    Flower, // flores/cristais (decoração no chão)
    Gate,   // portão de pedra (sólido até ser aberto com a Chave Antiga)
};

// Marcador de spawn extraído do arquivo do mapa (letra + posição em tiles).
struct SpawnMarker {
    char marker;
    int tx, ty;
};

// Paleta de cores de cada área (vila, floresta, caverna).
struct AreaTheme {
    const char* name;
    Color grass, grassAlt;
    Color path, pathAlt;
    Color water, waterAlt;
    Color treeTrunk, treeLeaf, treeLeafAlt;
    Color rock, rockAlt;
    Color wall, wallAlt;
    Color flowerA, flowerB;
    Color gate;
    float ambientDarkness; // 0 = normal; >0 aplica penumbra (caverna)
};

class TileMap {
public:
    static constexpr int kTileSize = 32;

    // Carrega o mapa de um arquivo ASCII. Linhas precisam ter o mesmo tamanho.
    bool loadFromFile(const std::string& path, std::string& error);

    int widthTiles() const { return width_; }
    int heightTiles() const { return height_; }
    float widthPixels() const { return (float)width_ * kTileSize; }
    float heightPixels() const { return (float)height_ * kTileSize; }

    Tile at(int tx, int ty) const;
    bool isSolid(int tx, int ty) const;
    bool rectCollides(Rectangle rect) const;

    // Move uma caixa (posição + tamanho) por delta, resolvendo colisões com tiles sólidos.
    Vector2 moveWithCollision(Vector2 pos, Vector2 size, Vector2 delta) const;

    void setGatesOpen(bool open) { gatesOpen_ = open; }
    bool gatesOpen() const { return gatesOpen_; }

    const std::vector<SpawnMarker>& spawnMarkers() const { return spawns_; }

    void draw(const AreaTheme& theme, float time) const;

private:
    std::vector<Tile> tiles_;
    std::vector<SpawnMarker> spawns_;
    int width_ = 0;
    int height_ = 0;
    bool gatesOpen_ = false;

    // Converte o caractere do arquivo em tile. terrainOnly = true para terreno puro;
    // false quando o caractere marca um spawn (NPC, inimigo, item, jogador).
    static Tile parseTile(char c, bool& terrainOnly);
    void drawTile(const AreaTheme& theme, Tile tile, int tx, int ty, float time) const;
};

// Variação determinística de cor por tile (evita visual chapado).
int tileHash(int tx, int ty);
