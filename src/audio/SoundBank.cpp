#include "audio/SoundBank.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

constexpr unsigned int kSampleRate = 44100;

enum class WaveShape { Sine, Square, Saw, Triangle, Noise };

// Um segmento de som: varredura de frequência com envelope de volume.
struct Segment {
    float freqStart;
    float freqEnd;
    float duration; // segundos
    WaveShape shape;
    float volume;   // 0..1
};

// Constrói um Wave 16-bit mono a partir dos segmentos (com envelope suave).
Wave buildWave(const std::vector<Segment>& segments)
{
    std::vector<short> samples;
    samples.reserve(kSampleRate); // maioria dos efeitos tem < 1s

    float phase = 0.0f;
    for (const Segment& seg : segments) {
        const int count = (int)(seg.duration * kSampleRate);
        for (int i = 0; i < count; i++) {
            const float t = (float)i / (float)count;
            const float freq = seg.freqStart + (seg.freqEnd - seg.freqStart) * t;
            phase += freq / (float)kSampleRate;

            float sample = 0.0f;
            switch (seg.shape) {
            case WaveShape::Sine:
                sample = std::sin(phase * 6.2831853f);
                break;
            case WaveShape::Square:
                sample = (phase - std::floor(phase)) < 0.5f ? 1.0f : -1.0f;
                break;
            case WaveShape::Saw:
                sample = 2.0f * (phase - std::floor(phase)) - 1.0f;
                break;
            case WaveShape::Triangle:
                sample = 2.0f * std::fabs(2.0f * (phase - std::floor(phase)) - 1.0f) - 1.0f;
                break;
            case WaveShape::Noise:
                sample = ((float)std::rand() / (float)RAND_MAX) * 2.0f - 1.0f;
                break;
            }

            // Envelope: ataque curto e decaimento suave evita estalos.
            const float envelope = (t < 0.05f) ? (t / 0.05f) : (1.0f - t) * (1.0f - t);
            const float value = sample * seg.volume * envelope;
            samples.push_back((short)(value * 32000.0f));
        }
    }

    // O raylib libera data com MemFree: usamos calloc para compatibilidade.
    const size_t bytes = samples.size() * sizeof(short);
    void* data = std::calloc(samples.empty() ? 1 : samples.size(), sizeof(short));
    if (!samples.empty()) std::memcpy(data, samples.data(), bytes);

    Wave wave{};
    wave.frameCount = (unsigned int)samples.size();
    wave.sampleRate = kSampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = data;
    return wave;
}

} // namespace

bool SoundBank::init()
{
    if (!IsAudioDeviceReady()) return false;

    // Combinações curtas para cada evento do jogo.
    struct Entry {
        Sfx sfx;
        std::vector<Segment> segments;
    };

    const Entry entries[] = {
        {Sfx::MenuMove, {{660.0f, 660.0f, 0.05f, WaveShape::Square, 0.25f}}},
        {Sfx::Confirm, {{520.0f, 780.0f, 0.09f, WaveShape::Square, 0.3f}}},
        {Sfx::Cancel, {{320.0f, 180.0f, 0.12f, WaveShape::Square, 0.3f}}},
        {Sfx::Attack, {{820.0f, 220.0f, 0.10f, WaveShape::Saw, 0.25f},
                       {300.0f, 120.0f, 0.05f, WaveShape::Noise, 0.12f}}},
        {Sfx::Hit, {{240.0f, 90.0f, 0.08f, WaveShape::Noise, 0.3f}}},
        {Sfx::EnemyDie, {{420.0f, 80.0f, 0.18f, WaveShape::Square, 0.28f}}},
        {Sfx::PlayerHurt, {{220.0f, 110.0f, 0.16f, WaveShape::Saw, 0.3f}}},
        {Sfx::Coin, {{880.0f, 1320.0f, 0.10f, WaveShape::Sine, 0.3f}}},
        {Sfx::Potion, {{440.0f, 880.0f, 0.22f, WaveShape::Sine, 0.28f}}},
        {Sfx::Key, {{523.0f, 1046.0f, 0.14f, WaveShape::Triangle, 0.28f}}},
        {Sfx::Save, {{392.0f, 392.0f, 0.08f, WaveShape::Sine, 0.28f},
                     {523.0f, 523.0f, 0.12f, WaveShape::Sine, 0.28f}}},
        {Sfx::Quest, {{523.0f, 523.0f, 0.11f, WaveShape::Sine, 0.3f},
                      {659.0f, 659.0f, 0.11f, WaveShape::Sine, 0.3f},
                      {784.0f, 784.0f, 0.18f, WaveShape::Sine, 0.32f}}},
        {Sfx::Gate, {{120.0f, 60.0f, 0.28f, WaveShape::Noise, 0.3f}}},
    };

    for (const Entry& entry : entries) {
        Wave wave = buildWave(entry.segments);
        sounds_[static_cast<size_t>(entry.sfx)] = LoadSoundFromWave(wave);
        UnloadWave(wave); // o som já tem uma cópia própria dos samples
    }

    ready_ = true;
    return true;
}

void SoundBank::shutdown()
{
    if (!ready_) return;
    for (Sound& sound : sounds_) {
        UnloadSound(sound);
        sound = Sound{};
    }
    ready_ = false;
}

void SoundBank::play(Sfx sfx)
{
    if (!ready_) return;
    const Sound& sound = sounds_[static_cast<size_t>(sfx)];
    PlaySound(sound);
}
