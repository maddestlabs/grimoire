#include "grimoire/ui/diegetic_ui.hpp"
#include <cstdio>
#include <cmath>
#include <algorithm>

namespace grimoire::ui {

DiegeticUI::DiegeticUI(StoryState& storyState) : storyState_(storyState) {
    // Set up hit-test rects
    // 3 Plot Device Slots
    plotSlotRects_[0] = Rect2I{120, 48, 26, 26};
    plotSlotRects_[1] = Rect2I{160, 48, 26, 26};
    plotSlotRects_[2] = Rect2I{200, 48, 26, 26};

    // 3 Shop Buy Buttons
    shopBuyRects_[0] = Rect2I{460, 118, 28, 16};
    shopBuyRects_[1] = Rect2I{500, 118, 28, 16};
    shopBuyRects_[2] = Rect2I{540, 118, 28, 16};

    // 3 Character Traits
    traitRects_[0] = Rect2I{252, 285, 32, 26};
    traitRects_[1] = Rect2I{304, 285, 32, 26};
    traitRects_[2] = Rect2I{356, 285, 32, 26};
}

void DiegeticUI::update(float dt) {
    animTime_ += dt;
}

bool DiegeticUI::handlePointerMove(int px, int py) {
    mouseX_ = px;
    mouseY_ = py;
    tooltip_.clear();

    // Check plot slots
    for (size_t i = 0; i < 3; ++i) {
        if (plotSlotRects_[i].contains(px, py)) {
            const auto& pds = storyState_.getPlotDevices();
            if (i < pds.size()) {
                tooltip_ = pds[i].name + (pds[i].active ? " [ACTIVE]" : " [Click to Invoke]");
                return true;
            }
        }
    }

    // Check shop buttons
    for (size_t i = 0; i < 3; ++i) {
        if (shopBuyRects_[i].contains(px, py)) {
            const auto& items = storyState_.getShopItems();
            if (i < items.size()) {
                if (items[i].purchased) {
                    tooltip_ = items[i].name + " (Purchased)";
                } else {
                    tooltip_ = items[i].name + ": " + items[i].description;
                }
                return true;
            }
        }
    }

    // Check trait cards
    for (size_t i = 0; i < 3; ++i) {
        if (traitRects_[i].contains(px, py)) {
            const auto& traits = storyState_.getCharacterTraits();
            if (i < traits.size()) {
                tooltip_ = traits[i].title + ": " + traits[i].description;
                return true;
            }
        }
    }

    // Check Word Bank
    if (px >= 66 && px <= 176 && py >= 145 && py <= 210) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Ink Embers: %d (Earned through writing)", storyState_.getEmbers());
        tooltip_ = buf;
        return true;
    }

    return false;
}

bool DiegeticUI::handlePointerClick(int px, int py) {
    // Check Plot Device clicks
    for (size_t i = 0; i < 3; ++i) {
        if (plotSlotRects_[i].contains(px, py)) {
            storyState_.togglePlotDevice(i);
            if (onPlotDeviceToggled) onPlotDeviceToggled();
            return true;
        }
    }

    // Check Shop Buy Button clicks
    for (size_t i = 0; i < 3; ++i) {
        if (shopBuyRects_[i].contains(px, py)) {
            if (storyState_.buyShopItem(i)) {
                if (onShopPurchase) onShopPurchase();
            }
            return true;
        }
    }

    return false;
}

void DiegeticUI::render(PixelCanvas& canvas) {
    renderDeskProps(canvas);
    renderPlotDevicesPanel(canvas);
    renderWordBankPanel(canvas);
    renderShopPanel(canvas);
    renderAchievementsPanel(canvas);
    renderCharacterTraitsPanel(canvas);
    renderActivePromptBanner(canvas);

    // Render tooltip if present
    if (!tooltip_.empty()) {
        int tw = static_cast<int>(tooltip_.length()) * 6 + 10;
        int tx = std::clamp(mouseX_ - tw / 2, 8, canvas.getWidth() - tw - 8);
        int ty = (mouseY_ > 30) ? mouseY_ - 18 : mouseY_ + 14;

        canvas.drawBevelBox(tx, ty, tw, 14, ColorRGBA(24, 20, 16, 240), ColorRGBA(75, 60, 45), ColorRGBA(10, 8, 6));
        canvas.drawText5x7(tx + 5, ty + 4, tooltip_, ColorRGBA(230, 205, 160));
    }
}

void DiegeticUI::renderDeskProps(PixelCanvas& canvas) {
    // 1. Vintage playing cards on left desk
    canvas.drawBevelBox(12, 238, 38, 54, ColorRGBA(205, 195, 175), ColorRGBA(235, 225, 210), ColorRGBA(140, 130, 115));
    canvas.drawRect(14, 240, 34, 50, ColorRGBA(160, 145, 125));
    canvas.setPixel(31, 265, ColorRGBA(150, 40, 30)); // Card red pip
    canvas.setPixel(31, 264, ColorRGBA(150, 40, 30));
    canvas.setPixel(30, 265, ColorRGBA(150, 40, 30));
    canvas.setPixel(32, 265, ColorRGBA(150, 40, 30));

    // Underneath overlapping card
    canvas.drawBevelBox(22, 248, 38, 54, ColorRGBA(215, 205, 185), ColorRGBA(240, 235, 220), ColorRGBA(150, 140, 120));
    canvas.drawRect(24, 250, 34, 50, ColorRGBA(170, 155, 135));

    // 2. Carved stone tokens & coins on desk
    canvas.fillCircle(85, 276, 7, ColorRGBA(160, 135, 80)); // Brass coin
    canvas.drawCircle(85, 276, 7, ColorRGBA(90, 75, 45));
    canvas.setPixel(85, 276, ColorRGBA(210, 180, 110));

    canvas.fillCircle(102, 280, 6, ColorRGBA(140, 115, 70)); // Smaller coin
    canvas.drawCircle(102, 280, 6, ColorRGBA(80, 65, 40));

    // Carved bone figure
    canvas.fillRect(124, 274, 14, 18, ColorRGBA(210, 200, 180));
    canvas.setPixel(128, 276, ColorRGBA(40, 30, 20));
    canvas.setPixel(134, 276, ColorRGBA(40, 30, 20));
    canvas.drawRect(124, 274, 14, 18, ColorRGBA(130, 120, 100));

    // 3. Embroidered cloth runner on right edge
    canvas.fillRect(575, 0, 65, 210, ColorRGBA(165, 160, 150));
    canvas.drawRect(577, 2, 61, 206, ColorRGBA(135, 130, 120));
    for (int y = 15; y < 200; y += 24) {
        canvas.drawIconCrest(600, y, ColorRGBA(115, 110, 100));
    }

    // 4. Wooden pot / cup on right desk
    canvas.fillRect(606, 175, 28, 38, ColorRGBA(78, 52, 32));
    canvas.drawRect(606, 175, 28, 38, ColorRGBA(115, 80, 50));
    canvas.fillRect(609, 170, 22, 6, ColorRGBA(45, 30, 20)); // Hollow rim

    // 5. Dice on right desk
    canvas.drawBevelBox(546, 276, 16, 16, ColorRGBA(230, 225, 215), ColorRGBA(250, 248, 240), ColorRGBA(160, 155, 145));
    // Pips
    canvas.setPixel(550, 280, ColorRGBA(25, 20, 15));
    canvas.setPixel(558, 280, ColorRGBA(25, 20, 15));
    canvas.setPixel(554, 284, ColorRGBA(25, 20, 15));
    canvas.setPixel(550, 288, ColorRGBA(25, 20, 15));
    canvas.setPixel(558, 288, ColorRGBA(25, 20, 15));

    canvas.drawBevelBox(568, 272, 16, 16, ColorRGBA(225, 220, 210), ColorRGBA(248, 245, 235), ColorRGBA(155, 150, 140));
    canvas.setPixel(576, 280, ColorRGBA(25, 20, 15));

    // 6. Left Candle
    // Candle wax column
    canvas.fillRect(82, 62, 16, 42, ColorRGBA(225, 215, 195));
    canvas.drawRect(82, 62, 16, 42, ColorRGBA(155, 140, 120));
    // Melted drips running down
    canvas.fillRect(80, 75, 4, 18, ColorRGBA(230, 220, 200));
    canvas.fillRect(96, 78, 3, 22, ColorRGBA(225, 215, 195));
    canvas.fillRect(76, 98, 28, 12, ColorRGBA(210, 195, 170)); // Pool of wax at base
    // Wick
    canvas.fillRect(89, 56, 2, 6, ColorRGBA(40, 30, 20));
    // Flame (animated flicker)
    float flameFlicker = std::sin(animTime_ * 14.0f) * 1.5f;
    int fx = 90 + static_cast<int>(flameFlicker * 0.4f);
    int fy = 52 + static_cast<int>(flameFlicker * 0.2f);
    canvas.drawCircleGlow(fx, fy, 14, ColorRGBA(255, 180, 50, 180));
    canvas.fillCircle(fx, fy, 4, ColorRGBA(255, 220, 80));
    canvas.fillCircle(fx, fy - 1, 2, ColorRGBA(255, 255, 210));

    // 7. Top Candle
    // Brass holder
    canvas.fillRect(386, 38, 20, 6, ColorRGBA(140, 115, 60));
    canvas.fillRect(392, 28, 8, 12, ColorRGBA(220, 210, 190));
    canvas.fillRect(395, 23, 2, 5, ColorRGBA(40, 30, 20)); // wick
    int tfx = 396 + static_cast<int>(std::sin(animTime_ * 10.0f) * 0.5f);
    int tfy = 20;
    canvas.drawCircleGlow(tfx, tfy, 12, ColorRGBA(255, 190, 70, 160));
    canvas.fillCircle(tfx, tfy, 3, ColorRGBA(255, 230, 100));
    canvas.setPixel(tfx, tfy, ColorRGBA(255, 255, 220));
}

void DiegeticUI::renderPlotDevicesPanel(PixelCanvas& canvas) {
    // Metal framed panel matching screenshot
    int px = 105;
    int py = 30;
    int pw = 138;
    int ph = 52;

    canvas.drawOrnateFrame(px, py, pw, ph, ColorRGBA(42, 45, 42), ColorRGBA(70, 75, 68), ColorRGBA(140, 145, 135));
    canvas.drawText5x7(px + 14, py + 6, "ACTIVE PLOT DEVICES", ColorRGBA(160, 140, 95));

    // 3 Item Slots
    const auto& pds = storyState_.getPlotDevices();
    for (size_t i = 0; i < 3; ++i) {
        int sx = plotSlotRects_[i].x;
        int sy = plotSlotRects_[i].y;
        int sw = plotSlotRects_[i].width;
        int sh = plotSlotRects_[i].height;

        bool active = (i < pds.size()) && pds[i].active;
        ColorRGBA slotFill = active ? ColorRGBA(60, 52, 38) : ColorRGBA(28, 30, 28);
        ColorRGBA slotBorder = active ? ColorRGBA(210, 160, 65) : ColorRGBA(55, 60, 55);

        canvas.drawBevelBox(sx, sy, sw, sh, slotFill, slotBorder, ColorRGBA(15, 18, 15));

        // Draw slot contents
        if (i == 0) {
            canvas.drawIconDagger(sx + 7, sy + 7, ColorRGBA(190, 195, 205), ColorRGBA(150, 120, 60));
        } else if (i == 1) {
            canvas.drawIconQuill(sx + 7, sy + 7, ColorRGBA(210, 205, 190), ColorRGBA(140, 100, 50));
        } else if (i == 2) {
            canvas.drawIconLock(sx + 7, sy + 7, ColorRGBA(160, 130, 70), ColorRGBA(190, 195, 200));
        }
    }
}

void DiegeticUI::renderWordBankPanel(PixelCanvas& canvas) {
    int px = 66;
    int py = 145;
    int pw = 110;
    int ph = 65;

    canvas.drawOrnateFrame(px, py, pw, ph, ColorRGBA(40, 42, 40), ColorRGBA(68, 72, 65), ColorRGBA(135, 140, 130));
    canvas.drawText5x7(px + 24, py + 7, "WORD BANK", ColorRGBA(155, 135, 90));

    // Large Word Count Text (e.g. "750/1000")
    char countBuf[32];
    snprintf(countBuf, sizeof(countBuf), "%zu/%zu", storyState_.getWordCount(), storyState_.getGoalWords());

    // Center the count text
    int textW = static_cast<int>(std::string_view(countBuf).length()) * 8;
    int textX = px + (pw - textW) / 2;
    canvas.drawText8x16(textX, py + 22, countBuf, ColorRGBA(205, 165, 85));

    // "WORDS" label below
    canvas.drawText5x7(px + 40, py + 40, "WORDS", ColorRGBA(135, 120, 95));

    // Progress bar groove
    int barX = px + 10;
    int barY = py + 50;
    int barW = pw - 20;
    int barH = 7;
    canvas.drawBevelBox(barX, barY, barW, barH, ColorRGBA(20, 22, 20), ColorRGBA(15, 18, 15), ColorRGBA(65, 70, 65));

    // Filled progress
    float ratio = storyState_.getProgressRatio();
    int fillW = static_cast<int>(ratio * static_cast<float>(barW - 2));
    if (fillW > 0) {
        canvas.fillRect(barX + 1, barY + 1, fillW, barH - 2, ColorRGBA(215, 145, 45));
        canvas.fillRect(barX + 1, barY + 1, fillW, 1, ColorRGBA(250, 200, 100)); // highlight edge
    }
}

void DiegeticUI::renderShopPanel(PixelCanvas& canvas) {
    int px = 450;
    int py = 70;
    int pw = 125;
    int ph = 74;

    canvas.drawOrnateFrame(px, py, pw, ph, ColorRGBA(40, 42, 40), ColorRGBA(68, 72, 65), ColorRGBA(135, 140, 130));
    canvas.drawText5x7(px + 10, py + 6, "STORY UPGRADE SHOP", ColorRGBA(155, 135, 90));

    const auto& items = storyState_.getShopItems();

    // 3 Item Icons & Badges
    for (size_t i = 0; i < 3; ++i) {
        int ix = px + 12 + static_cast<int>(i) * 36;
        int iy = py + 18;

        canvas.drawCircle(ix + 12, iy + 12, 11, ColorRGBA(65, 70, 65));
        canvas.fillCircle(ix + 12, iy + 12, 10, ColorRGBA(28, 30, 28));

        if (i == 0) {
            canvas.drawIconCompass(ix + 6, iy + 6, ColorRGBA(180, 150, 80), ColorRGBA(210, 60, 50));
        } else if (i == 1) {
            canvas.drawIconLantern(ix + 6, iy + 6, ColorRGBA(160, 140, 100), ColorRGBA(255, 180, 50));
        } else if (i == 2) {
            canvas.drawIconStar(ix + 6, iy + 6, ColorRGBA(210, 175, 80), ColorRGBA(255, 240, 180));
        }

        // Tiny item count/badge in upper right
        char badgeBuf[8];
        snprintf(badgeBuf, sizeof(badgeBuf), "%d", (i == 1) ? 10 : 15);
        canvas.drawText5x7(ix + 18, iy, badgeBuf, ColorRGBA(180, 170, 150));
    }

    // 3 Price Buttons
    for (size_t i = 0; i < 3; ++i) {
        int bx = shopBuyRects_[i].x;
        int by = shopBuyRects_[i].y;
        int bw = shopBuyRects_[i].width;
        int bh = shopBuyRects_[i].height;

        bool purchased = (i < items.size()) && items[i].purchased;
        ColorRGBA btnFill = purchased ? ColorRGBA(45, 55, 45) : ColorRGBA(32, 34, 32);
        ColorRGBA btnBorder = purchased ? ColorRGBA(90, 140, 90) : ColorRGBA(70, 75, 70);

        canvas.drawBevelBox(bx, by, bw, bh, btnFill, btnBorder, ColorRGBA(18, 20, 18));

        // Draw Ember icon and cost
        canvas.drawIconEmber(bx + 3, by + 4, ColorRGBA(235, 135, 40));

        char costBuf[8];
        if (purchased) {
            canvas.drawText5x7(bx + 11, by + 5, "OK", ColorRGBA(120, 210, 120));
        } else {
            snprintf(costBuf, sizeof(costBuf), "%d", items[i].cost);
            canvas.drawText5x7(bx + 11, by + 5, costBuf, ColorRGBA(210, 175, 95));
        }
    }
}

void DiegeticUI::renderAchievementsPanel(PixelCanvas& canvas) {
    int px = 455;
    int py = 158;
    int pw = 118;
    int ph = 62;

    canvas.drawOrnateFrame(px, py, pw, ph, ColorRGBA(40, 42, 40), ColorRGBA(68, 72, 65), ColorRGBA(135, 140, 130));
    canvas.drawText5x7(px + 24, py + 6, "ACHIEVEMENTS", ColorRGBA(155, 135, 90));

    const auto& achs = storyState_.getAchievements();

    // 3 Round Medals matching screenshot
    for (size_t i = 0; i < 3; ++i) {
        int mx = px + 12 + static_cast<int>(i) * 34;
        int my = py + 22;

        bool unlocked = (i < achs.size()) && achs[i].unlocked;
        ColorRGBA rimColor = unlocked ? ColorRGBA(215, 165, 65) : ColorRGBA(65, 70, 65);
        ColorRGBA innerColor = unlocked ? ColorRGBA(45, 38, 25) : ColorRGBA(26, 28, 26);

        canvas.drawCircle(mx + 13, my + 13, 12, rimColor);
        canvas.fillCircle(mx + 13, my + 13, 11, innerColor);

        if (i == 0) {
            canvas.drawIconDagger(mx + 7, my + 7, unlocked ? ColorRGBA(220, 220, 230) : ColorRGBA(100, 105, 110), ColorRGBA(140, 110, 50));
        } else if (i == 1) {
            canvas.drawIconLantern(mx + 7, my + 7, unlocked ? ColorRGBA(190, 160, 110) : ColorRGBA(90, 85, 75), unlocked ? ColorRGBA(255, 190, 60) : ColorRGBA(80, 60, 30));
        } else if (i == 2) {
            canvas.drawIconTrophy(mx + 7, my + 7, unlocked ? ColorRGBA(230, 190, 75) : ColorRGBA(100, 95, 80));
        }
    }
}

void DiegeticUI::renderCharacterTraitsPanel(PixelCanvas& canvas) {
    int px = 216;
    int py = 268;
    int pw = 208;
    int ph = 50;

    canvas.drawOrnateFrame(px, py, pw, ph, ColorRGBA(40, 42, 40), ColorRGBA(68, 72, 65), ColorRGBA(135, 140, 130));
    canvas.drawText5x7(px + 54, py + 6, "CHARACTER TRAITS", ColorRGBA(155, 135, 90));

    // 3 Trait Cards matching screenshot
    for (size_t i = 0; i < 3; ++i) {
        int tx = traitRects_[i].x;
        int ty = traitRects_[i].y;
        int tw = traitRects_[i].width;
        int th = traitRects_[i].height;

        canvas.drawBevelBox(tx, ty, tw, th, ColorRGBA(28, 30, 28), ColorRGBA(65, 72, 65), ColorRGBA(16, 18, 16));

        if (i == 0) {
            canvas.drawIconSword(tx + 10, ty + 7, ColorRGBA(190, 195, 205), ColorRGBA(140, 110, 50));
        } else if (i == 1) {
            canvas.drawIconCrest(tx + 10, ty + 7, ColorRGBA(150, 160, 170));
        } else if (i == 2) {
            canvas.drawIconStar(tx + 10, ty + 7, ColorRGBA(170, 175, 185), ColorRGBA(220, 225, 235));
        }
    }
}

void DiegeticUI::renderActivePromptBanner(PixelCanvas& canvas) {
    const std::string& prompt = storyState_.getActivePrompt();
    if (prompt.empty()) return;

    // Narrative prompt banner at top center
    int bw = 420;
    int bh = 22;
    int bx = (canvas.getWidth() - bw) / 2;
    int by = 6;

    canvas.drawBevelBox(bx, by, bw, bh, ColorRGBA(36, 26, 18, 245), ColorRGBA(195, 145, 55), ColorRGBA(20, 14, 10));
    canvas.drawText5x7(bx + 8, by + 7, prompt, ColorRGBA(250, 225, 170));
}

} // namespace grimoire::ui
