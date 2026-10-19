#include "systems/Quest.h"

std::string QuestStatus::trackerLine() const
{
    switch (state) {
    case QuestState::NotStarted:
        return "Fale com o Ancião na praça da vila.";
    case QuestState::Active:
        return "Derrote o Guardião na caverna e recupere a Flor de Lumina.";
    case QuestState::ReturnWithFlower:
        return "Devolva a Flor de Lumina ao Ancião.";
    case QuestState::Completed:
        return "Missão concluída! O bosque renasceu.";
    }
    return "";
}

const char* questStateName(QuestState state)
{
    switch (state) {
    case QuestState::NotStarted: return "not_started";
    case QuestState::Active: return "active";
    case QuestState::ReturnWithFlower: return "return_with_flower";
    case QuestState::Completed: return "completed";
    }
    return "not_started";
}

QuestState questStateFromName(const std::string& name)
{
    if (name == "active") return QuestState::Active;
    if (name == "return_with_flower") return QuestState::ReturnWithFlower;
    if (name == "completed") return QuestState::Completed;
    return QuestState::NotStarted;
}
