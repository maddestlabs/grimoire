#pragma once

#include "grimoire/core/types.hpp"
#include "grimoire/core/story_state.hpp"
#include "grimoire/render/pixel_canvas.hpp"
#include <string>
#include <functional>

namespace grimoire::ui {

using namespace grimoire::core;
using namespace grimoire::render;

class DiegeticUI {
public:
    DiegeticUI(StoryState& storyState);

    void update(float dt);
    void render(PixelCanvas& canvas);

    bool handlePointerClick(int px, int py);
    bool handlePointerMove(int px, int py);

    [[nodiscard]] const std::string& getTooltip() const noexcept { return tooltip_; }

    // Callbacks
    std::function<void()> onShopPurchase;
    std::function<void()> onPlotDeviceToggled;

private:
    void renderPlotDevicesPanel(PixelCanvas& canvas);
    void renderWordBankPanel(PixelCanvas& canvas);
    void renderShopPanel(PixelCanvas& canvas);
    void renderAchievementsPanel(PixelCanvas& canvas);
    void renderCharacterTraitsPanel(PixelCanvas& canvas);
    void renderDeskProps(PixelCanvas& canvas);
    void renderActivePromptBanner(PixelCanvas& canvas);

    StoryState& storyState_;
    std::string tooltip_;
    int mouseX_{0};
    int mouseY_{0};
    float animTime_{0.0f};

    // UI Regions for hit-testing
    Rect2I plotSlotRects_[3];
    Rect2I shopBuyRects_[3];
    Rect2I traitRects_[3];
    Rect2I prevPageRect_;
    Rect2I nextPageRect_;
};

} // namespace grimoire::ui
