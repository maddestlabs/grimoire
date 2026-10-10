#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "grimoire/audio/audio_engine.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace grimoire::audio {

struct AudioEngine::Impl {
    ma_device device;
    ma_device_config config;
};

static void audioDataCallback(ma_device* pDevice, void* pOutput, const void* /*pInput*/, ma_uint32 frameCount) {
    auto* engine = static_cast<AudioEngine*>(pDevice->pUserData);
    if (!engine) return;

    auto* out = static_cast<float*>(pOutput);
    // Miniaudio output is interleaved stereo: L, R, L, R...
    // Render in blocks
    constexpr uint32_t kChunkSize = 256;
    float bufL[kChunkSize];
    float bufR[kChunkSize];

    uint32_t framesRemaining = frameCount;
    uint32_t outIdx = 0;

    while (framesRemaining > 0) {
        uint32_t framesToProcess = std::min(framesRemaining, kChunkSize);
        engine->renderAudioBlock(bufL, bufR, framesToProcess);

        for (uint32_t i = 0; i < framesToProcess; ++i) {
            out[outIdx++] = bufL[i];
            out[outIdx++] = bufR[i];
        }

        framesRemaining -= framesToProcess;
    }
}

AudioEngine::AudioEngine() : impl_(std::make_unique<Impl>()) {
    snes_.reset();
    snes_.echo.enabled = true;
    snes_.echo.setFIRProfile("surround_reverb");
    snes_.echo.setDelayMs(100);
    snes_.echo.feedback = 0.35f;
    snes_.echo.volume = 0.25f;
}

AudioEngine::~AudioEngine() {
    shutdown();
}

bool AudioEngine::initialize(int sampleRate) {
    if (initialized_) return true;

    impl_->config = ma_device_config_init(ma_device_type_playback);
    impl_->config.playback.format = ma_format_f32;
    impl_->config.playback.channels = 2;
    impl_->config.sampleRate = static_cast<ma_uint32>(sampleRate);
    impl_->config.dataCallback = audioDataCallback;
    impl_->config.pUserData = this;

    if (ma_device_init(nullptr, &impl_->config, &impl_->device) != MA_SUCCESS) {
        std::cerr << "[Grimoire::Audio] Failed to initialize default audio device. Operating in silent mode.\n";
        return false;
    }

    if (ma_device_start(&impl_->device) != MA_SUCCESS) {
        ma_device_uninit(&impl_->device);
        std::cerr << "[Grimoire::Audio] Failed to start audio playback device.\n";
        return false;
    }

    initialized_ = true;
    std::cout << "[Grimoire::Audio] SNES S-DSP lo-fi sound core initialized at " << sampleRate << " Hz\n";
    return true;
}

void AudioEngine::shutdown() {
    if (initialized_) {
        ma_device_uninit(&impl_->device);
        initialized_ = false;
    }
}

void AudioEngine::playKeyClick(KeySoundType type) {
    // Randomize micro-pitch (+/- 3%)
    float pitchVariation = 1.0f + (rng_.nextFloat() - 0.5f) * 0.06f;

    switch (type) {
        case KeySoundType::AlphaNumeric: {
            // Tactile mechanical typewriter / quill key click
            int vIdx = 0;
            snes_.voices[vIdx].waveform = SNESWaveform::Pulse25;
            snes_.voices[vIdx].attack = 0.001f;
            snes_.voices[vIdx].decay = 0.016f;
            snes_.voices[vIdx].sustain = 0.0f;
            snes_.voices[vIdx].release = 0.01f;
            snes_.voices[vIdx].noiseEnabled = true;
            snes_.voices[vIdx].noiseRate = 12;
            snes_.voices[vIdx].noiseMix = 0.35f;

            float baseFreq = 480.0f * pitchVariation;
            snes_.noteOn(vIdx, baseFreq, 0.45f * foleyVolume_, 0.5f);
            break;
        }
        case KeySoundType::Space: {
            // Wooden spacebar thud
            int vIdx = 1;
            snes_.voices[vIdx].waveform = SNESWaveform::Triangle;
            snes_.voices[vIdx].attack = 0.002f;
            snes_.voices[vIdx].decay = 0.035f;
            snes_.voices[vIdx].sustain = 0.0f;
            snes_.voices[vIdx].release = 0.02f;
            snes_.voices[vIdx].noiseEnabled = true;
            snes_.voices[vIdx].noiseRate = 6;
            snes_.voices[vIdx].noiseMix = 0.2f;

            float baseFreq = 160.0f * pitchVariation;
            snes_.noteOn(vIdx, baseFreq, 0.55f * foleyVolume_, 0.5f);
            break;
        }
        case KeySoundType::Backspace: {
            // Crisp delete / backspace click
            int vIdx = 2;
            snes_.voices[vIdx].waveform = SNESWaveform::Pulse12;
            snes_.voices[vIdx].attack = 0.001f;
            snes_.voices[vIdx].decay = 0.022f;
            snes_.voices[vIdx].sustain = 0.0f;
            snes_.voices[vIdx].release = 0.01f;
            snes_.voices[vIdx].noiseEnabled = true;
            snes_.voices[vIdx].noiseRate = 14;
            snes_.voices[vIdx].noiseMix = 0.5f;

            float baseFreq = 360.0f * pitchVariation;
            snes_.noteOn(vIdx, baseFreq, 0.40f * foleyVolume_, 0.5f);
            break;
        }
        case KeySoundType::ReturnKey: {
            // Return latch / pleasant carriage chime
            int vIdx = 3;
            snes_.voices[vIdx].waveform = SNESWaveform::Sine;
            snes_.voices[vIdx].attack = 0.001f;
            snes_.voices[vIdx].decay = 0.08f;
            snes_.voices[vIdx].sustain = 0.0f;
            snes_.voices[vIdx].release = 0.04f;
            snes_.voices[vIdx].noiseEnabled = false;

            float baseFreq = 587.33f; // D5
            snes_.noteOn(vIdx, baseFreq, 0.50f * foleyVolume_, 0.6f);
            break;
        }
        case KeySoundType::PageTurn: {
            // Soft parchment rustle
            int vIdx = 4;
            snes_.voices[vIdx].waveform = SNESWaveform::Noise;
            snes_.voices[vIdx].attack = 0.015f;
            snes_.voices[vIdx].decay = 0.12f;
            snes_.voices[vIdx].sustain = 0.0f;
            snes_.voices[vIdx].release = 0.05f;
            snes_.voices[vIdx].noiseEnabled = true;
            snes_.voices[vIdx].noiseRate = 4;
            snes_.voices[vIdx].noiseMix = 1.0f;

            snes_.noteOn(vIdx, 220.0f, 0.40f * foleyVolume_, 0.5f);
            break;
        }
    }
}

void AudioEngine::playChimeReward() {
    // Gentle 3-note lo-fi celesta chord (C5, G5, C6) with S-DSP FIR echo
    constexpr float chordFreqs[] = { 523.25f, 783.99f, 1046.50f };
    for (int i = 0; i < 3; ++i) {
        int vIdx = 5 + i;
        snes_.voices[vIdx].waveform = SNESWaveform::Chime;
        snes_.voices[vIdx].attack = 0.005f;
        snes_.voices[vIdx].decay = 0.45f + i * 0.1f;
        snes_.voices[vIdx].sustain = 0.05f;
        snes_.voices[vIdx].release = 0.3f;
        snes_.voices[vIdx].noiseEnabled = false;

        float pan = 0.35f + 0.15f * static_cast<float>(i);
        snes_.noteOn(vIdx, chordFreqs[i], 0.35f * foleyVolume_, pan);
    }
}

void AudioEngine::playCoinClink() {
    // Coin/ember clink for shop purchases
    int vIdx = 7;
    snes_.voices[vIdx].waveform = SNESWaveform::Organ;
    snes_.voices[vIdx].attack = 0.001f;
    snes_.voices[vIdx].decay = 0.06f;
    snes_.voices[vIdx].sustain = 0.0f;
    snes_.voices[vIdx].release = 0.02f;
    snes_.voices[vIdx].noiseEnabled = false;

    snes_.noteOn(vIdx, 1318.51f, 0.45f * foleyVolume_, 0.65f); // E6
}

void AudioEngine::playPageTurn() {
    playKeyClick(KeySoundType::PageTurn);
}

void AudioEngine::renderAudioBlock(float* outL, float* outR, uint32_t numFrames) {
    constexpr float kSampleRate = 44100.0f;
    const float master = masterVolume_.load();
    const float ambVol = ambientVolume_.load() * master;

    // Filter coefficients for soothing window rain
    // Gentle pink/brownian noise model
    constexpr float rainAlpha = 0.045f;

    for (uint32_t i = 0; i < numFrames; ++i) {
        // 1. SNES Voice + FIR Echo processing
        float snesL = 0.0f;
        float snesR = 0.0f;
        snes_.processStereo(kSampleRate, snesL, snesR);

        // 2. Procedural Lo-Fi Ambient Rain
        float rainL = 0.0f;
        float rainR = 0.0f;
        if (rainEnabled_) {
            float whiteL = (rng_.nextFloat() - 0.5f) * 0.08f;
            float whiteR = (rng_.nextFloat() - 0.5f) * 0.08f;

            // Low-pass smooth rain rumble
            rainLpL_ += rainAlpha * (whiteL - rainLpL_);
            rainLpR_ += rainAlpha * (whiteR - rainLpR_);

            // High-pass soft droplet sizzle
            rainHpL_ = 0.95f * rainHpL_ + 0.05f * whiteL;
            rainHpR_ = 0.95f * rainHpR_ + 0.05f * whiteR;

            rainL = (rainLpL_ * 2.2f + rainHpL_ * 0.35f) * ambVol;
            rainR = (rainLpR_ * 2.2f + rainHpR_ * 0.35f) * ambVol;
        }

        // 3. Candle wax sizzle and micro-pops
        float crackleSample = 0.0f;
        if (crackleEnabled_) {
            if (crackleCountdown_ == 0) {
                // Next pop timing: every 800 to 7000 samples
                crackleCountdown_ = static_cast<uint32_t>(rng_.nextInt(800, 7000));
                crackleBurst_ = (rng_.nextFloat() > 0.6f) ? rng_.nextRange(0.08f, 0.22f) : 0.0f;
            } else {
                crackleCountdown_--;
            }

            if (crackleBurst_ > 0.001f) {
                crackleSample = (rng_.nextFloat() - 0.5f) * crackleBurst_ * ambVol * 0.4f;
                crackleBurst_ *= 0.85f; // rapid burst decay
            }
        }

        outL[i] = (snesL * master) + rainL + crackleSample;
        outR[i] = (snesR * master) + rainR + crackleSample;
    }
}

} // namespace grimoire::audio
