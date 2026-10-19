// Quest.h - estado da missão principal "A Flor de Lumina".
#pragma once

#include <string>

enum class QuestState {
    NotStarted,       // o jogador ainda não falou com o Ancião
    Active,           // missão em andamento: derrotar o Guardião
    ReturnWithFlower, // flor recuperada: voltar ao Ancião
    Completed         // missão concluída
};

struct QuestStatus {
    QuestState state = QuestState::NotStarted;
    bool bossDefeated = false;
    bool flowerTaken = false;

    // Linha exibida no rastreador da interface.
    std::string trackerLine() const;
};

const char* questStateName(QuestState state);
QuestState questStateFromName(const std::string& name);
