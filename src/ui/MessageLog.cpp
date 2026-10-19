#include "ui/MessageLog.h"

#include "raylib.h"

void MessageLog::add(std::string text, float duration)
{
    messages_.push_back(Message{std::move(text), duration, duration});
    while (messages_.size() > kMaxMessages) messages_.pop_front();
}

void MessageLog::update(float dt)
{
    for (Message& message : messages_) message.timer -= dt;
    while (!messages_.empty() && messages_.front().timer <= 0.0f) messages_.pop_front();
}

void MessageLog::draw(int x, int y) const
{
    int offset = y;
    for (const Message& message : messages_) {
        // Desbatace nos últimos segundos da mensagem.
        const float fade = message.timer < 1.0f ? message.timer : 1.0f;
        const unsigned char alpha = (unsigned char)(230.0f * fade);

        const int width = MeasureText(message.text.c_str(), 18) + 20;
        DrawRectangle(x, offset, width, 26, Color{20, 20, 30, (unsigned char)(alpha * 0.75f)});
        DrawRectangleLines(x, offset, width, 26, Color{240, 220, 150, alpha});
        DrawText(message.text.c_str(), x + 10, offset + 4, 18, Color{245, 240, 220, alpha});

        offset += 32;
    }
}
