#include "entities/Entity.h"

Vector2 directionVector(Direction dir)
{
    switch (dir) {
    case Direction::Up: return {0.0f, -1.0f};
    case Direction::Down: return {0.0f, 1.0f};
    case Direction::Left: return {-1.0f, 0.0f};
    case Direction::Right: return {1.0f, 0.0f};
    }
    return {0.0f, 0.0f};
}

Direction oppositeDirection(Direction dir)
{
    switch (dir) {
    case Direction::Up: return Direction::Down;
    case Direction::Down: return Direction::Up;
    case Direction::Left: return Direction::Right;
    case Direction::Right: return Direction::Left;
    }
    return Direction::Down;
}

Rectangle Entity::bounds() const
{
    return Rectangle{position.x, position.y, size.x, size.y};
}

Vector2 Entity::center() const
{
    return Vector2{position.x + size.x * 0.5f, position.y + size.y * 0.5f};
}
