#pragma once

#include "grimoire/core/types.hpp"
#include "grimoire/core/story_state.hpp"
#include "grimoire/editor/manuscript_editor.hpp"
#include "grimoire/ui/diegetic_ui.hpp"
#include "pixel_canvas.hpp"
#include <vector>

namespace grimoire::render {

using namespace grimoire::core;
using namespace grimoire::editor;
using namespace grimoire::ui;

class SceneRenderer {
public:
    SceneRenderer(StoryState& storyState, ManuscriptEditor& editor, DiegeticUI& ui);

    void update(float dt);
    void render(PixelCanvas& canvas);

    [[nodiscard]] const std::vector<LightSource2D>& getLights() const noexcept { return lights_; }
    [[nodiscard]] std::vector<LightSource2D>& getLights() noexcept { return lights_; }

private:
    void renderDeskBackground(PixelCanvas& canvas);
    void renderOpenGrimoire(PixelCanvas& canvas);

    StoryState& storyState_;
    ManuscriptEditor& editor_;
    DiegeticUI& ui_;

    std::vector<LightSource2D> lights_;
    float totalTime_{0.0f};
};

} // namespace grimoire::render
