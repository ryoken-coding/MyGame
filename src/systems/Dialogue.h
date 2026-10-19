// Dialogue.h - caixa de diálogo com páginas de texto.
#pragma once

#include <string>
#include <vector>

#include "entities/Npc.h"
#include "systems/Quest.h"

struct DialogueSequence {
    std::string speaker;
    std::vector<std::string> pages;
};

class DialogueSystem {
public:
    void start(DialogueSequence sequence);
    void advance(); // passa para a próxima página; termina a conversa na última
    void cancel();

    bool active() const { return active_; }
    const std::string& speaker() const { return sequence_.speaker; }
    const std::string& currentPage() const;
    int pageIndex() const { return index_ + 1; }
    int pageCount() const { return static_cast<int>(sequence_.pages.size()); }

private:
    DialogueSequence sequence_;
    int index_ = 0;
    bool active_ = false;
};

// Constrói a fala do NPC conforme o progresso da missão.
DialogueSequence buildNpcDialogue(NpcKind kind, const QuestStatus& quest);

// Resultado de uma conversa, aplicado quando o diálogo termina.
enum class DialogueOutcome { None, AcceptQuest, CompleteQuest };
