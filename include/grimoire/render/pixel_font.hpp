#pragma once

#include <cstdint>
#include <string_view>
#include "monospace_font_8x16.hpp"

namespace grimoire::render {

// 5x7 Micro Bitmap Font for compact HUD labels, prices, and stats.
// Each glyph is 5 columns wide, 7 rows tall, packed into 5 bytes (1 byte per column, 7 bits used).
extern const uint8_t kMicroFont5x7[128 * 5];

inline const uint8_t* getGlyph5x7Bitmap(uint8_t c) noexcept {
    if (c >= 128) c = '?';
    return &kMicroFont5x7[static_cast<size_t>(c) * 5];
}

} // namespace grimoire::render
