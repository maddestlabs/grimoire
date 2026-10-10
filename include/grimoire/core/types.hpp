#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <chrono>

namespace grimoire::core {

struct ColorRGBA {
    uint8_t r{255};
    uint8_t g{255};
    uint8_t b{255};
    uint8_t a{255};

    constexpr ColorRGBA() = default;
    constexpr ColorRGBA(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}

    [[nodiscard]] constexpr uint32_t toU32() const noexcept {
        return (static_cast<uint32_t>(a) << 24) |
               (static_cast<uint32_t>(b) << 16) |
               (static_cast<uint32_t>(g) << 8)  |
               static_cast<uint32_t>(r);
    }

    static constexpr ColorRGBA fromU32(uint32_t u) noexcept {
        return ColorRGBA(
            static_cast<uint8_t>(u & 0xFF),
            static_cast<uint8_t>((u >> 8) & 0xFF),
            static_cast<uint8_t>((u >> 16) & 0xFF),
            static_cast<uint8_t>((u >> 24) & 0xFF)
        );
    }
};

struct Point2I {
    int x{0};
    int y{0};
};

struct Rect2I {
    int x{0};
    int y{0};
    int width{0};
    int height{0};

    [[nodiscard]] constexpr bool contains(int px, int py) const noexcept {
        return px >= x && px < (x + width) && py >= y && py < (y + height);
    }
};

struct LightSource2D {
    float x{0.0f};
    float y{0.0f};
    float z{40.0f};         // Virtual height above desk surface
    float radius{320.0f};
    float intensity{1.0f};
    ColorRGBA color{255, 185, 90, 255}; // Warm burning candle amber
    float flickerPhase{0.0f};
    float flickerSpeed{1.0f};
};

} // namespace grimoire::core
