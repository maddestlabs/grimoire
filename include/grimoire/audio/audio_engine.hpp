#pragma once

#include "snes_dsp_core.hpp"
#include "prng.hpp"
#include <memory>
#include <atomic>
#include <string>

namespace grimoire::audio {

enum class KeySoundType {
    AlphaNumeric,
    Space,
    Backspace,
    ReturnKey,
    PageTurn
};

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    bool initialize(int sampleRate = 44100);
    void shutdown();

    void playKeyClick(KeySoundType type = KeySoundType::AlphaNumeric);
    void playChimeReward();
    void playCoinClink();
    void playPageTurn();

    void setRainEnabled(bool enabled) noexcept { rainEnabled_ = enabled; }
    [[nodiscard]] bool isRainEnabled() const noexcept { return rainEnabled_; }

    void setCandleCrackleEnabled(bool enabled) noexcept { crackleEnabled_ = enabled; }
    [[nodiscard]] bool isCandleCrackleEnabled() const noexcept { return crackleEnabled_; }

    void setMasterVolume(float vol) noexcept { masterVolume_ = std::clamp(vol, 0.0f, 1.0f); }
    [[nodiscard]] float getMasterVolume() const noexcept { return masterVolume_; }

    void setFoleyVolume(float vol) noexcept { foleyVolume_ = std::clamp(vol, 0.0f, 1.0f); }
    void setAmbientVolume(float vol) noexcept { ambientVolume_ = std::clamp(vol, 0.0f, 1.0f); }

    // Audio callback internal processing
    void renderAudioBlock(float* outL, float* outR, uint32_t numFrames);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;

    SNESDSPEngine snes_;
    DeterministicPRNG rng_{1337};

    std::atomic<bool> initialized_{false};
    std::atomic<bool> rainEnabled_{true};
    std::atomic<bool> crackleEnabled_{true};
    std::atomic<float> masterVolume_{0.75f};
    std::atomic<float> foleyVolume_{0.85f};
    std::atomic<float> ambientVolume_{0.40f};

    // Rain filter state
    float rainLpL_{0.0f};
    float rainLpR_{0.0f};
    float rainHpL_{0.0f};
    float rainHpR_{0.0f};

    // Candle crackle state
    uint32_t crackleCountdown_{0};
    float crackleBurst_{0.0f};
};

} // namespace grimoire::audio
