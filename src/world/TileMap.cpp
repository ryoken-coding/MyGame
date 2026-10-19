#include "world/TileMap.h"

#include <algorithm>
#include <cmath>

#include "util/Files.h"
#include "util/Strings.h"

namespace {

Color jitter(Color base, int amount)
{
    auto clamp255 = [](int v) { return v < 0 ? 0 : (v > 255 ? 255 : v); };
    return Color{(unsigned char)clamp255(base.r + amount),
                 (unsigned char)clamp255(base.g + amount),
                 (unsigned char)clamp255(base.b + amount),
                 base.a};
}

} // namespace

int tileHash(int tx, int ty)
{
    unsigned int h = static_cast<unsigned int>(tx) * 374761393u + static_cast<unsigned int>(ty) * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return static_cast<int>(h & 0xFFFF);
}

Tile TileMap::parseTile(char c, bool& terrainOnly)
{
    terrainOnly = true;
    switch (c) {
    case '.': return Tile::Grass;
    case ',': return Tile::Path;
    case '~': return Tile::Water;
    case 'T': return Tile::Tree;
    case 'R': return Tile::Rock;
    case '#': return Tile::Wall;
    case 'F': return Tile::Flower;
    case 'X': return Tile::Gate;
    default:
        // Letras reservadas para spawns: o personagem/item fica sobre grama.
        terrainOnly = false;
        return Tile::Grass;
    }
}

bool TileMap::loadFromFile(const std::string& path, std::string& error)
{
    error.clear();

    std::vector<std::string> searched;
    const std::string resolved = util::resolveResourcePath(path, &searched);
    if (resolved.empty()) {
        std::string list;
        for (const std::string& s : searched) {
            if (!list.empty()) list += ", ";
            list += "'" + s + "'";
        }
        error = util::formatText(
            "Arquivo de mapa não encontrado: '%s'. Caminhos testados: %s. "
            "Execute o jogo a partir da raiz do projeto.", path.c_str(), list.c_str());
        return false;
    }

    std::string content;
    if (!util::readTextFile(resolved, content, error)) return false;

    // Quebra em linhas (aceita \n e \r\n).
    std::vector<std::string> rows;
    std::string line;
    for (char c : content) {
        if (c == '\n') {
            rows.push_back(line);
            line.clear();
        } else if (c != '\r') {
            line += c;
        }
    }
    if (!line.empty()) rows.push_back(line);

    // Remove linhas vazias no fim do arquivo.
    while (!rows.empty() && rows.back().find_first_not_of(' ') == std::string::npos) {
        rows.pop_back();
    }

    if (rows.empty()) {
        error = util::formatText("Arquivo de mapa vazio: %s", resolved.c_str());
        return false;
    }

    const int w = static_cast<int>(rows.front().size());
    const int h = static_cast<int>(rows.size());
    if (w < 4 || h < 4) {
        error = util::formatText("Mapa muito pequeno em %s (mínimo 4x4 tiles)", resolved.c_str());
        return false;
    }

    tiles_.clear();
    spawns_.clear();
    tiles_.reserve(static_cast<size_t>(w) * h);

    for (int ty = 0; ty < h; ty++) {
        if (static_cast<int>(rows[static_cast<size_t>(ty)].size()) != w) {
            error = util::formatText(
                "Mapa %s: a linha %d tem %d colunas, mas a linha 1 tem %d. "
                "Todas as linhas precisam ter o mesmo tamanho.",
                resolved.c_str(), ty + 1, (int)rows[static_cast<size_t>(ty)].size(), w);
            tiles_.clear();
            spawns_.clear();
            return false;
        }
        for (int tx = 0; tx < w; tx++) {
            const char c = rows[static_cast<size_t>(ty)][static_cast<size_t>(tx)];
            bool terrainOnly = false;
            tiles_.push_back(parseTile(c, terrainOnly));

            if (!terrainOnly) {
                spawns_.push_back(SpawnMarker{c, tx, ty});
            }
        }
    }

    width_ = w;
    height_ = h;
    gatesOpen_ = false;
    return true;
}

Tile TileMap::at(int tx, int ty) const
{
    if (tx < 0 || ty < 0 || tx >= width_ || ty >= height_) return Tile::Wall;
    return tiles_[static_cast<size_t>(ty) * width_ + tx];
}

bool TileMap::isSolid(int tx, int ty) const
{
    const Tile tile = at(tx, ty);
    switch (tile) {
    case Tile::Water:
    case Tile::Tree:
    case Tile::Rock:
    case Tile::Wall:
        return true;
    case Tile::Gate:
        return !gatesOpen_;
    default:
        return false;
    }
}

bool TileMap::rectCollides(Rectangle rect) const
{
    const int x0 = (int)std::floor(rect.x / kTileSize);
    const int y0 = (int)std::floor(rect.y / kTileSize);
    const int x1 = (int)std::floor((rect.x + rect.width - 0.001f) / kTileSize);
    const int y1 = (int)std::floor((rect.y + rect.height - 0.001f) / kTileSize);

    for (int ty = y0; ty <= y1; ty++) {
        for (int tx = x0; tx <= x1; tx++) {
            if (isSolid(tx, ty)) return true;
        }
    }
    return false;
}

Vector2 TileMap::moveWithCollision(Vector2 pos, Vector2 size, Vector2 delta) const
{
    const float eps = 0.01f;
    // O movimento é feito em passos limitados a meio tile: garante que a
    // resolução de colisão sempre encontre a borda correta, mesmo com
    // deltas grandes (sem "atravessar" paredes).
    const float maxStep = kTileSize * 0.5f;

    auto moveAxis = [&](Vector2 current, float step, bool horizontal) {
        Vector2 next = current;
        if (horizontal) next.x += step;
        else next.y += step;

        if (!rectCollides(Rectangle{next.x, next.y, size.x, size.y})) return next;

        if (horizontal) {
            if (step > 0.0f) {
                const int tx = (int)std::floor((next.x + size.x) / kTileSize);
                next.x = tx * kTileSize - size.x - eps;
            } else {
                const int tx = (int)std::floor(next.x / kTileSize);
                next.x = (tx + 1) * kTileSize + eps;
            }
            if (rectCollides(Rectangle{next.x, next.y, size.x, size.y})) next.x = current.x;
        } else {
            if (step > 0.0f) {
                const int ty = (int)std::floor((next.y + size.y) / kTileSize);
                next.y = ty * kTileSize - size.y - eps;
            } else {
                const int ty = (int)std::floor(next.y / kTileSize);
                next.y = (ty + 1) * kTileSize + eps;
            }
            if (rectCollides(Rectangle{next.x, next.y, size.x, size.y})) next.y = current.y;
        }
        return next;
    };

    // Eixo X
    float remaining = delta.x;
    while (std::fabs(remaining) > 0.0001f) {
        const float step = (remaining > 0.0f) ? std::min(remaining, maxStep)
                                              : std::max(remaining, -maxStep);
        const Vector2 before = pos;
        pos = moveAxis(pos, step, true);
        remaining -= step;
        if (std::fabs(pos.x - before.x) < 0.0001f && std::fabs(step) > 0.0001f) break; // parado
    }

    // Eixo Y
    remaining = delta.y;
    while (std::fabs(remaining) > 0.0001f) {
        const float step = (remaining > 0.0f) ? std::min(remaining, maxStep)
                                              : std::max(remaining, -maxStep);
        const Vector2 before = pos;
        pos = moveAxis(pos, step, false);
        remaining -= step;
        if (std::fabs(pos.y - before.y) < 0.0001f && std::fabs(step) > 0.0001f) break;
    }

    return pos;
}

void TileMap::draw(const AreaTheme& theme, float time) const
{
    for (int ty = 0; ty < height_; ty++) {
        for (int tx = 0; tx < width_; tx++) {
            drawTile(theme, at(tx, ty), tx, ty, time);
        }
    }
}

void TileMap::drawTile(const AreaTheme& theme, Tile tile, int tx, int ty, float time) const
{
    const int px = tx * kTileSize;
    const int py = ty * kTileSize;
    const int hash = tileHash(tx, ty);
    const int jitterAmt = (hash % 3) * 6 - 6; // -6, 0, +6

    switch (tile) {
    case Tile::Grass: {
        const Color base = jitter(theme.grass, jitterAmt);
        DrawRectangle(px, py, kTileSize, kTileSize, base);
        // Tufos de grama
        if (hash % 5 == 0) {
            DrawRectangle(px + 6 + hash % 14, py + 8 + (hash / 7) % 12, 3, 6, theme.grassAlt);
        }
        break;
    }
    case Tile::Path: {
        DrawRectangle(px, py, kTileSize, kTileSize, jitter(theme.path, jitterAmt));
        if (hash % 4 == 0) {
            DrawCircle(px + 8 + hash % 16, py + 10 + (hash / 5) % 14, 2.5f, theme.pathAlt);
        }
        break;
    }
    case Tile::Water: {
        DrawRectangle(px, py, kTileSize, kTileSize, theme.water);
        // Ondas animadas
        const float phase = (float)sin(time * 1.5 + (tx + ty) * 0.7);
        const int waveY = py + 10 + (int)(phase * 3.0f) + (hash % 3) * 6;
        DrawRectangle(px + 2, waveY, kTileSize - 10, 3, theme.waterAlt);
        DrawRectangle(px + 12, waveY + 10, kTileSize - 18, 3, theme.waterAlt);
        break;
    }
    case Tile::Tree: {
        // Grama embaixo da árvore
        DrawRectangle(px, py, kTileSize, kTileSize, jitter(theme.grass, jitterAmt - 4));
        const int cx = px + kTileSize / 2;
        const int cy = py + kTileSize / 2;
        DrawRectangle(cx - 4, cy, 8, kTileSize / 2, theme.treeTrunk);
        DrawCircle(cx, cy - 6, 15.0f, theme.treeLeaf);
        DrawCircle(cx - 9, cy + 2, 11.0f, theme.treeLeafAlt);
        DrawCircle(cx + 9, cy + 2, 11.0f, theme.treeLeafAlt);
        DrawCircle(cx - 4, cy - 10, 6.0f, jitter(theme.treeLeafAlt, 18));
        break;
    }
    case Tile::Rock: {
        DrawRectangle(px, py, kTileSize, kTileSize, jitter(theme.grass, jitterAmt - 6));
        DrawRectangleRounded(Rectangle{(float)px + 3, (float)py + 5, 26.0f, 22.0f}, 0.35f, 5,
                             jitter(theme.rock, jitterAmt));
        DrawRectangleRounded(Rectangle{(float)px + 7, (float)py + 8, 10.0f, 7.0f}, 0.5f, 4,
                             theme.rockAlt);
        break;
    }
    case Tile::Wall: {
        DrawRectangle(px, py, kTileSize, kTileSize, jitter(theme.wall, jitterAmt));
        // Padrão de tijolos
        DrawRectangle(px, py, kTileSize, 2, theme.wallAlt);
        DrawRectangle(px, py + kTileSize / 2, kTileSize, 2, theme.wallAlt);
        DrawRectangle(px + ((ty % 2 == 0) ? 8 : 20), py + 2, 2, kTileSize / 2 - 2, theme.wallAlt);
        DrawRectangle(px + ((ty % 2 == 0) ? 22 : 14), py + kTileSize / 2 + 2, 2, kTileSize / 2 - 2,
                      theme.wallAlt);
        break;
    }
    case Tile::Flower: {
        DrawRectangle(px, py, kTileSize, kTileSize, jitter(theme.grass, jitterAmt));
        const Color petal = (hash % 2 == 0) ? theme.flowerA : theme.flowerB;
        for (int i = 0; i < 3; i++) {
            const int fx = px + 6 + (hash >> (i * 2)) % 20;
            const int fy = py + 6 + (hash >> (i * 2 + 1)) % 20;
            DrawCircle(fx, fy, 3.5f, petal);
            DrawCircle(fx, fy, 1.5f, Color{255, 230, 120, 255});
        }
        break;
    }
    case Tile::Gate: {
        // Base de pedra com barras de madeira; o desenho muda quando aberto.
        DrawRectangle(px, py, kTileSize, kTileSize, jitter(theme.path, jitterAmt));
        if (!gatesOpen_) {
            DrawRectangle(px + 2, py, kTileSize - 4, kTileSize, theme.gate);
            for (int i = 0; i < 3; i++) {
                DrawRectangle(px + 6 + i * 8, py + 2, 4, kTileSize - 4, Color{40, 30, 25, 255});
            }
            DrawRectangle(px + 2, py + 12, kTileSize - 4, 5, Color{50, 38, 30, 255});
        } else {
            // Barras caídas nas laterais
            DrawRectangle(px, py, 5, kTileSize, theme.gate);
            DrawRectangle(px + kTileSize - 5, py, 5, kTileSize, theme.gate);
        }
        break;
    }
    }
}
