#pragma once

#include "grimoire/core/types.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace grimoire::editor {

using namespace grimoire::core;

enum class TokenType : uint8_t {
    Normal,
    Dialogue,   // "..."
    Entity,     // [...] or @name
    Emphasis,   // *...*
    Bullet,     // - or * at start
    Header      // # Header
};

struct StyledToken {
    TokenType type{TokenType::Normal};
    int startCol{0};
    int length{0};
    ColorRGBA color{42, 34, 28, 255};
};

class ProseHighlighter {
public:
    ProseHighlighter();

    std::vector<StyledToken> highlightLine(std::string_view line);

    void setDialogueColor(ColorRGBA c) noexcept { dialogueColor_ = c; }
    void setEntityColor(ColorRGBA c) noexcept { entityColor_ = c; }
    void setEmphasisColor(ColorRGBA c) noexcept { emphasisColor_ = c; }
    void setNormalColor(ColorRGBA c) noexcept { normalColor_ = c; }

private:
    ColorRGBA normalColor_{42, 34, 28, 255};
    ColorRGBA dialogueColor_{195, 125, 45, 255};  // Warm amber
    ColorRGBA entityColor_{110, 75, 160, 255};    // Royal purple
    ColorRGBA emphasisColor_{175, 140, 50, 255};  // Soft gold
    ColorRGBA bulletColor_{90, 75, 60, 255};
    ColorRGBA headerColor_{20, 15, 10, 255};
};

} // namespace grimoire::editor
