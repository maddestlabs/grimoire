#include "grimoire/editor/manuscript_editor.hpp"
#include <cctype>
#include <sstream>
#include <algorithm>

namespace grimoire::editor {

ManuscriptEditor::ManuscriptEditor() {
    // Initial sample text matching the manuscript theme
    std::string sample = 
        "\"Writing grimoire!\" you\n"
        "set star of your\n"
        "hand mine, and you\n"
        "and a writing your\n"
        "prompt. Let us in a\n"
        "story a gently hear\n"
        "grimoire.\n\n"
        "Write to embrace the\n"
        "next story your land.\n"
        "- Come raise your star,\n"
        "and we do want to\n"
        "your sword war this\n"
        "story?\n\n"
        "- When you see you\n"
        "compete to achieve to\n"
        "reach milestone.\n\n"
        "- Will unlock upgrade\n"
        "your last story.";

    setText(sample);
}

void ManuscriptEditor::setText(std::string_view text) {
    doc_.setText(text);
    cursor_ = TextCoord{0, 0};
    anchor_ = cursor_;
    reflow();
}

std::string ManuscriptEditor::getText() const {
    return doc_.getText();
}

size_t ManuscriptEditor::getWordCount() const noexcept {
    return cachedWordCount_;
}

size_t ManuscriptEditor::getCharCount() const noexcept {
    size_t count = 0;
    for (size_t i = 0; i < doc_.getLineCount(); ++i) {
        count += doc_.getLine(i).length();
    }
    return count;
}

void ManuscriptEditor::reflow() {
    layoutLines_.clear();
    cachedWordCount_ = 0;

    int currentSpread = 0;
    int currentPage = 0; // 0 = left, 1 = right
    int visualRow = 0;

    for (size_t lIdx = 0; lIdx < doc_.getLineCount(); ++lIdx) {
        const std::string& line = doc_.getLine(lIdx);

        // Count words
        bool inWord = false;
        for (char ch : line) {
            if (std::isalnum(static_cast<unsigned char>(ch))) {
                if (!inWord) {
                    inWord = true;
                    cachedWordCount_++;
                }
            } else {
                inWord = false;
            }
        }

        // Line wrapping
        if (line.empty()) {
            VisualLine vl;
            vl.docLine = static_cast<int>(lIdx);
            vl.docStartCol = 0;
            vl.docLength = 0;
            vl.text = "";
            vl.pageIndex = currentSpread * 2 + currentPage;
            vl.visualRow = visualRow++;
            layoutLines_.push_back(vl);

            if (visualRow >= metrics_.maxRows) {
                visualRow = 0;
                currentPage++;
                if (currentPage >= 2) {
                    currentPage = 0;
                    currentSpread++;
                }
            }
            continue;
        }

        int col = 0;
        int lineLen = static_cast<int>(line.length());
        while (col < lineLen) {
            int chunkLen = std::min(metrics_.maxCols, lineLen - col);

            // If we are cutting through a word, break on last whitespace
            if (col + chunkLen < lineLen) {
                int lastSpace = -1;
                for (int i = chunkLen - 1; i > 0; --i) {
                    if (std::isspace(static_cast<unsigned char>(line[static_cast<size_t>(col + i)]))) {
                        lastSpace = i;
                        break;
                    }
                }
                if (lastSpace > 0) {
                    chunkLen = lastSpace + 1; // include the space
                }
            }

            VisualLine vl;
            vl.docLine = static_cast<int>(lIdx);
            vl.docStartCol = col;
            vl.docLength = chunkLen;
            vl.text = line.substr(static_cast<size_t>(col), static_cast<size_t>(chunkLen));
            vl.pageIndex = currentSpread * 2 + currentPage;
            vl.visualRow = visualRow++;
            layoutLines_.push_back(vl);

            col += chunkLen;

            if (visualRow >= metrics_.maxRows) {
                visualRow = 0;
                currentPage++;
                if (currentPage >= 2) {
                    currentPage = 0;
                    currentSpread++;
                }
            }
        }
    }

    if (onWordCountChanged) {
        onWordCountChanged();
    }
}

int ManuscriptEditor::getTotalSpreads() const noexcept {
    if (layoutLines_.empty()) return 1;
    int maxPage = 0;
    for (const auto& vl : layoutLines_) {
        if (vl.pageIndex > maxPage) maxPage = vl.pageIndex;
    }
    return (maxPage / 2) + 1;
}

void ManuscriptEditor::prevPage() {
    if (currentSpread_ > 0) {
        currentSpread_--;
    }
}

void ManuscriptEditor::nextPage() {
    if (currentSpread_ + 1 < getTotalSpreads()) {
        currentSpread_++;
    }
}

void ManuscriptEditor::ensureCursorVisible() {
    for (const auto& vl : layoutLines_) {
        if (vl.docLine == cursor_.line &&
            cursor_.column >= vl.docStartCol &&
            cursor_.column <= vl.docStartCol + vl.docLength) {
            int spread = vl.pageIndex / 2;
            currentSpread_ = spread;
            break;
        }
    }
}

void ManuscriptEditor::insertText(std::string_view str) {
    cursor_ = doc_.insert(cursor_, str);
    anchor_ = cursor_;
    cursorVisible_ = true;
    cursorBlinkTimer_ = 0.0f;
    reflow();
    ensureCursorVisible();
}

void ManuscriptEditor::deleteBackward() {
    if (cursor_.column == 0 && cursor_.line == 0) return;

    if (cursor_.column > 0) {
        TextRange range{TextCoord{cursor_.line, cursor_.column - 1}, cursor_};
        cursor_ = doc_.erase(range);
    } else {
        int prevLine = cursor_.line - 1;
        int prevLen = static_cast<int>(doc_.getLine(static_cast<size_t>(prevLine)).length());
        TextRange range{TextCoord{prevLine, prevLen}, cursor_};
        cursor_ = doc_.erase(range);
    }
    anchor_ = cursor_;
    cursorVisible_ = true;
    cursorBlinkTimer_ = 0.0f;
    reflow();
    ensureCursorVisible();
}

void ManuscriptEditor::deleteForward() {
    int lineLen = static_cast<int>(doc_.getLine(static_cast<size_t>(cursor_.line)).length());
    if (cursor_.column < lineLen) {
        TextRange range{cursor_, TextCoord{cursor_.line, cursor_.column + 1}};
        doc_.erase(range);
    } else if (cursor_.line + 1 < static_cast<int>(doc_.getLineCount())) {
        TextRange range{cursor_, TextCoord{cursor_.line + 1, 0}};
        doc_.erase(range);
    }
    anchor_ = cursor_;
    cursorVisible_ = true;
    cursorBlinkTimer_ = 0.0f;
    reflow();
    ensureCursorVisible();
}

bool ManuscriptEditor::handleChar(char32_t codepoint) {
    if (codepoint < 32 || codepoint > 126) return false;
    char ch = static_cast<char>(codepoint);
    std::string s(1, ch);
    insertText(s);
    if (onKeyTyped) onKeyTyped(codepoint);
    return true;
}

bool ManuscriptEditor::handleKey(int key, bool /*shift*/, bool ctrl) {
    // 259 = Backspace, 257 = Enter, 261 = Delete, 262 = Right, 263 = Left, 264 = Down, 265 = Up
    // (Standard GLFW / Sokol keycodes)
    if (ctrl) {
        if (key == 'Z' || key == 'z') {
            cursor_ = doc_.undo();
            anchor_ = cursor_;
            reflow();
            ensureCursorVisible();
            return true;
        } else if (key == 'Y' || key == 'y') {
            cursor_ = doc_.redo();
            anchor_ = cursor_;
            reflow();
            ensureCursorVisible();
            return true;
        }
    }

    switch (key) {
        case 259: // Backspace
            deleteBackward();
            return true;
        case 261: // Delete
            deleteForward();
            return true;
        case 257: // Enter
            insertText("\n");
            return true;
        case 258: // Tab
            insertText("  ");
            return true;
        case 263: // Left
            if (cursor_.column > 0) {
                cursor_.column--;
            } else if (cursor_.line > 0) {
                cursor_.line--;
                cursor_.column = static_cast<int>(doc_.getLine(static_cast<size_t>(cursor_.line)).length());
            }
            anchor_ = cursor_;
            cursorVisible_ = true;
            cursorBlinkTimer_ = 0.0f;
            ensureCursorVisible();
            return true;
        case 262: // Right
            if (cursor_.column < static_cast<int>(doc_.getLine(static_cast<size_t>(cursor_.line)).length())) {
                cursor_.column++;
            } else if (cursor_.line + 1 < static_cast<int>(doc_.getLineCount())) {
                cursor_.line++;
                cursor_.column = 0;
            }
            anchor_ = cursor_;
            cursorVisible_ = true;
            cursorBlinkTimer_ = 0.0f;
            ensureCursorVisible();
            return true;
        case 265: // Up
            if (cursor_.line > 0) {
                cursor_.line--;
                cursor_.column = std::min(cursor_.column, static_cast<int>(doc_.getLine(static_cast<size_t>(cursor_.line)).length()));
            }
            anchor_ = cursor_;
            cursorVisible_ = true;
            cursorBlinkTimer_ = 0.0f;
            ensureCursorVisible();
            return true;
        case 264: // Down
            if (cursor_.line + 1 < static_cast<int>(doc_.getLineCount())) {
                cursor_.line++;
                cursor_.column = std::min(cursor_.column, static_cast<int>(doc_.getLine(static_cast<size_t>(cursor_.line)).length()));
            }
            anchor_ = cursor_;
            cursorVisible_ = true;
            cursorBlinkTimer_ = 0.0f;
            ensureCursorVisible();
            return true;
        case 266: // Page Up
            prevPage();
            return true;
        case 267: // Page Down
            nextPage();
            return true;
    }

    return false;
}

bool ManuscriptEditor::handleClick(int px, int py) {
    // Check if clicked in left page
    int leftPage = currentSpread_ * 2;
    int rightPage = currentSpread_ * 2 + 1;

    int targetPage = -1;
    int relX = 0;
    int relY = 0;

    if (px >= metrics_.leftPageX && px < metrics_.leftPageX + metrics_.pageWidth &&
        py >= metrics_.leftPageY && py < metrics_.leftPageY + metrics_.pageHeight) {
        targetPage = leftPage;
        relX = px - metrics_.leftPageX;
        relY = py - metrics_.leftPageY;
    } else if (px >= metrics_.rightPageX && px < metrics_.rightPageX + metrics_.pageWidth &&
               py >= metrics_.rightPageY && py < metrics_.rightPageY + metrics_.pageHeight) {
        targetPage = rightPage;
        relX = px - metrics_.rightPageX;
        relY = py - metrics_.rightPageY;
    }

    if (targetPage < 0) return false;

    int targetRow = relY / metrics_.lineHeight;
    int targetCol = relX / metrics_.charWidth;

    // Find corresponding line
    for (const auto& vl : layoutLines_) {
        if (vl.pageIndex == targetPage && vl.visualRow == targetRow) {
            cursor_.line = vl.docLine;
            cursor_.column = std::min(vl.docStartCol + targetCol, vl.docStartCol + vl.docLength);
            anchor_ = cursor_;
            cursorVisible_ = true;
            cursorBlinkTimer_ = 0.0f;
            return true;
        }
    }

    // If clicked past end of text on that page, put at end of last visual line on that page
    for (auto it = layoutLines_.rbegin(); it != layoutLines_.rend(); ++it) {
        if (it->pageIndex == targetPage) {
            cursor_.line = it->docLine;
            cursor_.column = it->docStartCol + it->docLength;
            anchor_ = cursor_;
            cursorVisible_ = true;
            cursorBlinkTimer_ = 0.0f;
            return true;
        }
    }

    return false;
}

void ManuscriptEditor::update(float dt) {
    cursorBlinkTimer_ += dt;
    if (cursorBlinkTimer_ >= 0.5f) {
        cursorBlinkTimer_ = 0.0f;
        cursorVisible_ = !cursorVisible_;
    }
}

void ManuscriptEditor::render(PixelCanvas& canvas) {
    int leftPage = currentSpread_ * 2;
    int rightPage = currentSpread_ * 2 + 1;

    for (const auto& vl : layoutLines_) {
        if (vl.pageIndex != leftPage && vl.pageIndex != rightPage) continue;

        int pageX = (vl.pageIndex == leftPage) ? metrics_.leftPageX : metrics_.rightPageX;
        int pageY = (vl.pageIndex == leftPage) ? metrics_.leftPageY : metrics_.rightPageY;

        int lineY = pageY + vl.visualRow * metrics_.lineHeight;

        // Highlight tokens
        auto tokens = highlighter_.highlightLine(vl.text);
        if (tokens.empty()) {
            canvas.drawText5x7(pageX, lineY, vl.text, ColorRGBA(42, 34, 28));
        } else {
            int curCol = 0;
            for (const auto& tok : tokens) {
                int tokX = pageX + curCol * metrics_.charWidth;
                std::string tokText = vl.text.substr(static_cast<size_t>(tok.startCol), static_cast<size_t>(tok.length));
                canvas.drawText5x7(tokX, lineY, tokText, tok.color);
                curCol += tok.length;
            }
        }

        // Render Cursor
        if (cursorVisible_ && vl.docLine == cursor_.line) {
            if (cursor_.column >= vl.docStartCol && cursor_.column <= vl.docStartCol + vl.docLength) {
                // If cursor is at end of line or in middle
                bool atLineEnd = (cursor_.column == vl.docStartCol + vl.docLength);
                if (!atLineEnd || vl.docStartCol + vl.docLength == static_cast<int>(doc_.getLine(static_cast<size_t>(vl.docLine)).length())) {
                    int cX = pageX + (cursor_.column - vl.docStartCol) * metrics_.charWidth;
                    // Draw warm amber illuminated cursor bar
                    canvas.fillRect(cX, lineY, 1, 8, ColorRGBA(220, 130, 40));
                }
            }
        }
    }

    // Render Page Number Footers
    char pBuf[32];
    snprintf(pBuf, sizeof(pBuf), "%d", leftPage + 1);
    canvas.drawText5x7(metrics_.leftPageX + metrics_.pageWidth / 2 - 3, metrics_.leftPageY + metrics_.pageHeight - 8, pBuf, ColorRGBA(130, 115, 95));

    snprintf(pBuf, sizeof(pBuf), "%d", rightPage + 1);
    canvas.drawText5x7(metrics_.rightPageX + metrics_.pageWidth / 2 - 3, metrics_.rightPageY + metrics_.pageHeight - 8, pBuf, ColorRGBA(130, 115, 95));
}

} // namespace grimoire::editor
