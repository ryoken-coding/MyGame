#include "systems/Dialogue.h"

void DialogueSystem::start(DialogueSequence sequence)
{
    sequence_ = std::move(sequence);
    index_ = 0;
    active_ = !sequence_.pages.empty();
}

void DialogueSystem::advance()
{
    if (!active_) return;
    index_++;
    if (index_ >= static_cast<int>(sequence_.pages.size())) {
        cancel();
    }
}

void DialogueSystem::cancel()
{
    active_ = false;
    index_ = 0;
    sequence_ = DialogueSequence{};
}

const std::string& DialogueSystem::currentPage() const
{
    static const std::string empty;
    if (!active_ || index_ < 0 || index_ >= static_cast<int>(sequence_.pages.size())) return empty;
    return sequence_.pages[static_cast<size_t>(index_)];
}

DialogueSequence buildNpcDialogue(NpcKind kind, const QuestStatus& quest)
{
    DialogueSequence seq;

    if (kind == NpcKind::Elder) {
        seq.speaker = "Ancião";
        switch (quest.state) {
        case QuestState::NotStarted:
            seq.pages = {
                "Ah, jovem aventureiro... Nossa vila está em perigo! Uma criatura sombria "
                "roubou a Flor de Lumina, o coração sagrado do bosque.",
                "O Guardião a levou para a caverna ao norte da floresta. Enquanto a flor "
                "estiver longe, a luz do bosque se apaga e os monstros ficam corajosos.",
                "Atravesse a floresta, derrote o Guardião e traga a Flor de Lumina de volta. "
                "A vila conta com você!",
            };
            break;
        case QuestState::Active:
            seq.pages = {
                "A Flor ainda está com o Guardião, na caverna ao norte da floresta.",
                "Tome cuidado pelo caminho. Se precisar, beba uma poção com [Q] para "
                "recuperar suas forças.",
            };
            break;
        case QuestState::ReturnWithFlower:
            seq.pages = {
                "Incrível... você conseguiu! A Flor de Lumina brilha novamente em suas mãos!",
                "Graças a você, o bosque renascerá e nossa vila está salva.",
                "Aceite esta recompensa: moedas e poções para sua jornada. "
                "Que a luz da flor guie seus passos, herói!",
            };
            break;
        case QuestState::Completed:
            seq.pages = {
                "O bosque já mostra seus primeiros brotos novos. Nunca esqueceremos "
                "o que você fez por nós.",
            };
            break;
        }
        return seq;
    }

    // Aldeão
    seq.speaker = "Aldeão";
    switch (quest.state) {
    case QuestState::NotStarted:
    case QuestState::Active:
        seq.pages = {
            "Oi! Você vai até a floresta? Cuidado com os slimes... parecem bobos, mas mordem!",
            "Dizem que um goblin enorme carrega a chave do portão da caverna. "
            "Derrote-o e o caminho ao norte se abrirá.",
        };
        break;
    case QuestState::ReturnWithFlower:
        seq.pages = {
            "Você tem a Flor de Lumina! Corre, volte com o Ancião! "
            "Sinto o bosque recuperando as cores daqui mesmo!",
        };
        break;
    case QuestState::Completed:
        seq.pages = {
            "Você é o herói da vila! As crianças já cantam suas aventuras nas ruas.",
        };
        break;
    }
    return seq;
}
