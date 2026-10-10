#pragma once

#include "grimoire/core/text_document.hpp"
#include "grimoire/core/types.hpp"
#include "grimoire/render/pixel_canvas.hpp"
#include "prose_highlighter.hpp"
#include <string>
#include <vector>
#include <chrono>
#include <functional>

namespace grimoire::editor {

using namespace grimoire::core;
using namespace grimoire::render;

struct PageLayoutMetrics {
    int leftPageX{202};
    int leftPageY{86};
    int rightPageX{332};
    int rightPageY{86};
    int pageWidth{108};
    int pageHeight{152};
    int maxCols{18};
    int maxRows{16};
    int charWidth{6};
    int lineHeight{9};
};

struct VisualLine {
    int docLine{0};
    int docStartCol{0};
    int docLength{0};
    std::string text;
    int pageIndex{0}; // 0 = left page, 1 = right page of current spread
    int visualRow{0};
};

class ManuscriptEditor {
public:
    ManuscriptEditor();

    void setText(std::string_view text);
    [[nodiscard]] std::string getText() const;
    [[nodiscard]] size_t getWordCount() const noexcept;
    [[nodiscard]] size_t getCharCount() const noexcept;

    // Navigation and editing
    bool handleKey(int key, bool shift, bool ctrl);
    bool handleChar(char32_t codepoint);
    bool handleClick(int px, int py);

    void insertText(std::string_view str);
    void deleteBackward();
    void deleteForward();

    void prevPage();
    void nextPage();
    [[nodiscard]] int getCurrentSpread() const noexcept { return currentSpread_; }
    [[nodiscard]] int getTotalSpreads() const noexcept;

    void update(float dt);
    void render(PixelCanvas& canvas);

    [[nodiscard]] TextCoord getCursor() const noexcept { return cursor_; }
    void setCursor(TextCoord pos) noexcept { cursor_ = pos; anchor_ = pos; }

    [[nodiscard]] const PageLayoutMetrics& getMetrics() const noexcept { return metrics_; }
    void setMetrics(const PageLayoutMetrics& m) noexcept { metrics_ = m; reflow(); }

    // Callbacks
    std::function<void(char32_t c)> onKeyTyped;
    std::function<void()> onWordCountChanged;

private:
    void reflow();
    void ensureCursorVisible();

    TextDocument doc_;
    TextCoord cursor_{0, 0};
    TextCoord anchor_{0, 0};

    PageLayoutMetrics metrics_;
    ProseHighlighter highlighter_;

    std::vector<VisualLine> layoutLines_;
    int currentSpread_{0};

    float cursorBlinkTimer_{0.0f};
    bool cursorVisible_{true};

    size_t cachedWordCount_{0};
};

} // namespace grimoire::editor
