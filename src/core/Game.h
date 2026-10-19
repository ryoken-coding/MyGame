// Game.h - laço principal e máquina de estados do jogo.
#pragma once

#include <string>

#include "audio/SoundBank.h"
#include "core/SaveSystem.h"
#include "entities/Player.h"
#include "systems/Dialogue.h"
#include "systems/Quest.h"
#include "ui/MessageLog.h"
#include "world/World.h"

enum class GameState { Title, Playing, Paused, GameOver, Victory };

class Game {
public:
    // Carrega recursos iniciais (sons e definições das áreas).
    bool init(std::string& error);
    void shutdown();

    void update(float dt);
    void draw(float time);

    bool wantsQuit() const { return wantsQuit_; }
    void requestQuit() { wantsQuit_ = true; }

    GameState state() const { return state_; }

    // Operações públicas usadas pelos menus e pelo modo de verificação (--smoke).
    bool startNewGame(std::string& error);
    bool saveGame(std::string& error);
    bool loadGame(std::string& error);

private:
    GameState state_ = GameState::Title;
    bool wantsQuit_ = false;

    Player player_;
    World world_;
    QuestStatus quest_;
    DialogueSystem dialogue_;
    MessageLog messages_;
    SoundBank sounds_;
    Camera2D camera_{};

    std::string notice_;             // aviso persistente na tela de título (erros de save)
    std::string pendingInteract_;    // texto do prompt "[E] ..." quando próximo de algo
    int titleSelection_ = 0;
    int pauseSelection_ = 0;
    int gameOverSelection_ = 0;
    int victorySelection_ = 0;
    int enemiesDefeated_ = 0;
    float gateHintCooldown_ = 0.0f;
    DialogueOutcome dialogueOutcome_ = DialogueOutcome::None;

    void handleTitleInput();
    void handlePlayingInput(float dt);
    void handlePauseInput();
    void handleGameOverInput();
    void handleVictoryInput();

    void updateCamera(float dt);
    void drawWorld(float time) const;

    void tryInteract();
    void doAttack();
    void checkPickups();
    void checkEnemyContact();
    void checkGate();
    void checkTransitions();

    bool changeArea(const std::string& areaName, const std::string& spawnName, std::string& error);
    void onPlayerDeath();
    void onQuestCompleted();
    void applySaveData(const SaveData& data);
    SaveData collectSaveData() const;
    void placePlayerAt(const std::string& spawnName);
};
