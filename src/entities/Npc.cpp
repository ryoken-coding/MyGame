#include "entities/Npc.h"

#include <cmath>

Npc::Npc(NpcKind kind, std::string id, Vector2 tileCenter)
    : kind_(kind), id_(std::move(id))
{
    entity_.size = Vector2{24.0f, 30.0f};
    entity_.position = Vector2{tileCenter.x - entity_.size.x * 0.5f,
                               tileCenter.y - entity_.size.y * 0.5f};
}

const char* Npc::displayName() const
{
    return kind_ == NpcKind::Elder ? "Ancião" : "Aldeão";
}

void Npc::draw(float time) const
{
    const Rectangle r = entity_.bounds();
    const float bob = (kind_ == NpcKind::Elder) ? 0.0f : 1.5f * ((float)std::sin(time * 2.0 + 1.0));

    // Sombra
    DrawEllipse((int)entity_.center().x, (int)(r.y + r.height) + 2, 14.0f, 5.0f, Color{0, 0, 0, 60});

    const Color robe = (kind_ == NpcKind::Elder) ? Color{90, 70, 140, 255} : Color{170, 110, 60, 255};
    const Color trim = (kind_ == NpcKind::Elder) ? Color{180, 160, 230, 255} : Color{220, 180, 120, 255};

    // Corpo
    DrawRectangleRounded(Rectangle{r.x, r.y + 8.0f + bob, r.width, r.height - 8.0f},
                         0.35f, 6, robe);
    // Cabeça
    DrawCircle((int)(r.x + r.width * 0.5f), (int)(r.y + 6.0f + bob), 10.0f, Color{226, 188, 148, 255});

    if (kind_ == NpcKind::Elder) {
        // Barba e manto do Ancião
        DrawCircle((int)(r.x + r.width * 0.5f), (int)(r.y + 12.0f + bob), 6.0f, Color{230, 230, 235, 255});
        DrawRectangle((int)(r.x + 2.0f), (int)(r.y + 10.0f + bob), 4, 14, trim);
        DrawRectangle((int)(r.x + r.width - 6.0f), (int)(r.y + 10.0f + bob), 4, 14, trim);
    } else {
        // Olhos do Aldeão
        DrawCircle((int)(r.x + r.width * 0.5f - 4), (int)(r.y + 5.0f + bob), 1.6f, Color{30, 30, 40, 255});
        DrawCircle((int)(r.x + r.width * 0.5f + 4), (int)(r.y + 5.0f + bob), 1.6f, Color{30, 30, 40, 255});
    }
}
