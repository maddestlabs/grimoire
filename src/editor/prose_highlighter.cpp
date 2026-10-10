#include "grimoire/editor/prose_highlighter.hpp"

namespace grimoire::editor {

ProseHighlighter::ProseHighlighter() = default;

std::vector<StyledToken> ProseHighlighter::highlightLine(std::string_view line) {
    std::vector<StyledToken> tokens;
    if (line.empty()) return tokens;

    int len = static_cast<int>(line.length());
    int i = 0;

    // Check for header (# ...)
    if (line[0] == '#') {
        tokens.push_back({TokenType::Header, 0, len, headerColor_});
        return tokens;
    }

    // Check for bullet (- ...)
    if (line.length() >= 2 && (line[0] == '-' || line[0] == '*') && line[1] == ' ') {
        tokens.push_back({TokenType::Bullet, 0, 2, bulletColor_});
        i = 2;
    }

    while (i < len) {
        char ch = line[static_cast<size_t>(i)];

        if (ch == '"') {
            // Dialogue
            int start = i;
            i++;
            while (i < len && line[static_cast<size_t>(i)] != '"') {
                i++;
            }
            if (i < len && line[static_cast<size_t>(i)] == '"') {
                i++; // include closing quote
            }
            tokens.push_back({TokenType::Dialogue, start, i - start, dialogueColor_});
        } else if (ch == '[') {
            // Entity / Character name [Name]
            int start = i;
            i++;
            while (i < len && line[static_cast<size_t>(i)] != ']') {
                i++;
            }
            if (i < len && line[static_cast<size_t>(i)] == ']') {
                i++;
            }
            tokens.push_back({TokenType::Entity, start, i - start, entityColor_});
        } else if (ch == '*' && i + 1 < len && line[static_cast<size_t>(i + 1)] != ' ') {
            // Emphasis *word*
            int start = i;
            i++;
            while (i < len && line[static_cast<size_t>(i)] != '*') {
                i++;
            }
            if (i < len && line[static_cast<size_t>(i)] == '*') {
                i++;
            }
            tokens.push_back({TokenType::Emphasis, start, i - start, emphasisColor_});
        } else {
            // Normal text
            int start = i;
            while (i < len) {
                char cur = line[static_cast<size_t>(i)];
                if (cur == '"' || cur == '[' || cur == '*') break;
                i++;
            }
            tokens.push_back({TokenType::Normal, start, i - start, normalColor_});
        }
    }

    return tokens;
}

} // namespace grimoire::editor
