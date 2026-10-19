// SoundBank.h - efeitos sonoros gerados proceduralmente (sem arquivos de áudio externos).
#pragma once

#include <array>
#include <string>

#include "raylib.h"

enum class Sfx {
    MenuMove,
    Confirm,
    Cancel,
    Attack,
    Hit,
    EnemyDie,
    PlayerHurt,
    Coin,
    Potion,
    Key,
    Save,
    Quest,
    Gate,
    Count
};

class SoundBank {
public:
    // Gera os sons. Se o dispositivo de áudio não estiver disponível,
    // play() vira no-op e ready() devolve false (o jogo continua normalmente).
    bool init();
    void shutdown();

    void play(Sfx sfx);
    bool ready() const { return ready_; }

private:
    std::array<Sound, static_cast<size_t>(Sfx::Count)> sounds_{};
    bool ready_ = false;
};
