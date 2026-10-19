#include "entities/ItemPickup.h"

#include <cmath>

ItemPickup::ItemPickup(ItemType type, std::string id, Vector2 tileCenter, int amount)
    : type_(type), id_(std::move(id)), amount_(amount)
{
    position_ = tileCenter;
    // Fase aleatória faz os itens não "pulsarem" em uníssono.
    bobPhase_ = (float)GetRandomValue(0, 100) / 100.0f * 6.28f;
}

Rectangle ItemPickup::bounds() const
{
    return Rectangle{position_.x - 12.0f, position_.y - 12.0f, 24.0f, 24.0f};
}

Vector2 ItemPickup::center() const
{
    return position_;
}

void ItemPickup::draw(float time) const
{
    const float bob = 3.0f * std::sin(time * 3.0f + bobPhase_);
    const float y = position_.y + bob;

    // Sombra
    DrawEllipse((int)position_.x, (int)(position_.y + 14.0f), 10.0f, 3.5f, Color{0, 0, 0, 50});

    switch (type_) {
    case ItemType::Potion: {
        DrawCircle((int)position_.x, (int)(y + 2.0f), 8.0f, Color{200, 60, 70, 255});
        DrawRectangle((int)(position_.x - 3.0f), (int)(y - 12.0f), 6, 7, Color{180, 180, 200, 255});
        DrawCircle((int)(position_.x - 2), (int)y, 2.5f, Color{255, 160, 160, 180});
        break;
    }
    case ItemType::Coin: {
        DrawCircle((int)position_.x, (int)y, 8.0f, GOLD);
        DrawCircle((int)position_.x, (int)y, 5.0f, Color{255, 220, 100, 255});
        break;
    }
    case ItemType::Key: {
        // Anel e haste da chave
        DrawCircleLines((int)(position_.x - 5.0f), (int)y, 5.0f, GOLD);
        DrawCircle((int)(position_.x - 5.0f), (int)y, 2.0f, Color{255, 220, 100, 255});
        DrawRectangle((int)(position_.x - 2.0f), (int)(y - 1.5f), 12, 3, GOLD);
        DrawRectangle((int)(position_.x + 7.0f), (int)(y + 1.0f), 3, 5, GOLD);
        DrawRectangle((int)(position_.x + 3.0f), (int)(y + 1.0f), 3, 4, GOLD);
        break;
    }
    case ItemType::Flower: {
        // Pétalas luminosas ao redor de um centro dourado
        for (int i = 0; i < 6; i++) {
            const float a = (float)i / 6.0f * 6.2831853f + bobPhase_;
            const float px = position_.x + std::cos(a) * 9.0f;
            const float py = y + std::sin(a) * 9.0f;
            DrawCircle((int)px, (int)py, 5.0f, Color{170, 220, 255, 255});
        }
        DrawCircle((int)position_.x, (int)y, 5.5f, Color{255, 235, 150, 255});
        // Brilho suave
        DrawCircle((int)position_.x, (int)y, 14.0f, Color{170, 220, 255, 40});
        break;
    }
    default:
        DrawRectangle((int)(position_.x - 6.0f), (int)(y - 6.0f), 12, 12, GRAY);
        break;
    }
}
