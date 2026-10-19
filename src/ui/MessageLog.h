// MessageLog.h - mensagens temporárias na tela (itens, missões, avisos).
#pragma once

#include <deque>
#include <string>

class MessageLog {
public:
    void add(std::string text, float duration = 4.5f);
    void update(float dt);
    void clear() { messages_.clear(); }
    void draw(int x, int y) const;

private:
    struct Message {
        std::string text;
        float timer;
        float duration;
    };
    std::deque<Message> messages_;
    static constexpr size_t kMaxMessages = 5;
};
