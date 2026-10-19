// Entity.h - tipos e utilitários compartilhados pelas entidades do jogo.
#pragma once

#include "raylib.h"

// Direções de movimento/ataque em quatro eixos.
enum class Direction { Up, Down, Left, Right };

Vector2 directionVector(Direction dir);
Direction oppositeDirection(Direction dir);

// Entidade básica: posição, tamanho e pontos de vida.
struct Entity {
    Vector2 position{0.0f, 0.0f}; // canto superior esquerdo, em pixels
    Vector2 size{24.0f, 28.0f};
    int hp = 10;
    int maxHp = 10;
    float speed = 90.0f;
    bool alive = true;

    Rectangle bounds() const;
    Vector2 center() const;
};
