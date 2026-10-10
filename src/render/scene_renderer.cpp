#include "grimoire/render/scene_renderer.hpp"
#include <cmath>

namespace grimoire::render {

SceneRenderer::SceneRenderer(StoryState& storyState, ManuscriptEditor& editor, DiegeticUI& ui)
    : storyState_(storyState), editor_(editor), ui_(ui) {

    // Configure 2.5D candle point lights matching screenshot
    // Light 0: Left Candle
    LightSource2D leftCandle;
    leftCandle.x = 90.0f;
    leftCandle.y = 56.0f;
    leftCandle.z = 45.0f;
    leftCandle.radius = 360.0f;
    leftCandle.intensity = 1.25f;
    leftCandle.color = ColorRGBA(255, 185, 80);
    leftCandle.flickerPhase = 0.0f;
    leftCandle.flickerSpeed = 1.0f;
    lights_.push_back(leftCandle);

    // Light 1: Top Candle
    LightSource2D topCandle;
    topCandle.x = 396.0f;
    topCandle.y = 24.0f;
    topCandle.z = 40.0f;
    topCandle.radius = 310.0f;
    topCandle.intensity = 1.05f;
    topCandle.color = ColorRGBA(255, 205, 110);
    topCandle.flickerPhase = 2.4f;
    topCandle.flickerSpeed = 0.85f;
    lights_.push_back(topCandle);
}

void SceneRenderer::update(float dt) {
    totalTime_ += dt;
    editor_.update(dt);
    ui_.update(dt);

    // Dynamic upgrades reaction
    const auto& items = storyState_.getShopItems();
    if (!items.empty() && items[0].purchased) {
        // Scented amber wax upgrade
        lights_[0].color = ColorRGBA(255, 195, 65);
        lights_[0].intensity = 1.35f;
    }
}

void SceneRenderer::render(PixelCanvas& canvas) {
    // 1. Clear canvas and height map
    canvas.clear(ColorRGBA(30, 22, 16));
    canvas.clearHeight(0);

    // 2. Render base desk wood planks
    renderDeskBackground(canvas);

    // 3. Render open Grimoire (leather binding + aged parchment)
    renderOpenGrimoire(canvas);

    // 4. Render manuscript text (with syntax highlighting and blinking cursor)
    editor_.render(canvas);

    // 5. Render diegetic HUD panels, icons, props, and active prompts
    ui_.render(canvas);

    // 6. Multi-point dynamic 2.5D candle lighting pass
    canvas.applyLighting(lights_, totalTime_);

    // 7. Organic corner foliage shadow silhouettes
    canvas.applyFoliageShadows(totalTime_);

    // 8. Subtle vignette pass
    canvas.applyVignette(0.36f);
}

void SceneRenderer::renderDeskBackground(PixelCanvas& canvas) {
    int w = canvas.getWidth();
    int h = canvas.getHeight();

    // Wood planks
    constexpr int kPlankHeight = 65;
    for (int y = 0; y < h; y += kPlankHeight) {
        ColorRGBA plankColor = ((y / kPlankHeight) % 2 == 0) ? ColorRGBA(40, 28, 20) : ColorRGBA(36, 25, 18);
        canvas.fillRect(0, y, w, kPlankHeight - 1, plankColor);
        canvas.fillRect(0, y + kPlankHeight - 1, w, 1, ColorRGBA(20, 14, 10)); // plank seam
    }

    // Desk rivets along upper beam matching screenshot
    for (int x = 110; x < 540; x += 32) {
        canvas.setPixel(x, 18, ColorRGBA(90, 85, 80));
        canvas.setPixel(x, 19, ColorRGBA(30, 28, 26));
    }
}

void SceneRenderer::renderOpenGrimoire(PixelCanvas& canvas) {
    // Book Dimensions matching screenshot
    int bx = 182;
    int by = 66;
    int bw = 276;
    int bh = 188;

    // Leather cover binding base
    canvas.drawBevelBox(bx, by, bw, bh, ColorRGBA(105, 70, 38), ColorRGBA(140, 95, 55), ColorRGBA(55, 35, 18));

    // Corner Brass Protectors
    canvas.fillRect(bx, by, 10, 10, ColorRGBA(195, 160, 65));
    canvas.fillRect(bx + bw - 10, by, 10, 10, ColorRGBA(195, 160, 65));
    canvas.fillRect(bx, by + bh - 10, 10, 10, ColorRGBA(195, 160, 65));
    canvas.fillRect(bx + bw - 10, by + bh - 10, 10, 10, ColorRGBA(195, 160, 65));

    // Golden nib upgrade gilded edges
    const auto& items = storyState_.getShopItems();
    if (items.size() > 2 && items[2].purchased) {
        canvas.drawRect(bx + 2, by + 2, bw - 4, bh - 4, ColorRGBA(215, 175, 75));
    }

    // Deckle edge page edges (stacked parchment layers)
    for (int i = 0; i < 6; ++i) {
        canvas.fillRect(bx + 5 + i, by + 4 + i, 2, bh - 8 - i * 2, ColorRGBA(190, 175, 150));
        canvas.fillRect(bx + bw - 7 - i, by + 4 + i, 2, bh - 8 - i * 2, ColorRGBA(190, 175, 150));
    }

    // Left Page
    int lpx = bx + 12;
    int lpy = by + 6;
    int lpw = 124;
    int lph = bh - 12;

    // Aged parchment base
    canvas.fillRect(lpx, lpy, lpw, lph, ColorRGBA(222, 210, 186));
    // Curled page edge shading
    canvas.drawRect(lpx, lpy, lpw, lph, ColorRGBA(195, 180, 155));

    // Right Page
    int rpx = bx + 138;
    int rpy = by + 6;
    int rpw = 124;
    int rph = bh - 12;

    canvas.fillRect(rpx, rpy, rpw, rph, ColorRGBA(220, 208, 184));
    canvas.drawRect(rpx, rpy, rpw, rph, ColorRGBA(195, 180, 155));

    // Book spine center crease & gutter shadow
    canvas.fillRect(bx + 133, by + 5, 8, bh - 10, ColorRGBA(145, 130, 110));
    canvas.fillRect(bx + 135, by + 4, 4, bh - 8, ColorRGBA(110, 95, 75));
    canvas.fillRect(bx + 136, by + 3, 2, bh - 6, ColorRGBA(80, 68, 52));

    // Red bookmark ribbon trailing at bottom center
    canvas.fillRect(bx + 135, by + bh - 15, 4, 25, ColorRGBA(170, 42, 35));
    canvas.fillRect(bx + 134, by + bh + 8, 6, 4, ColorRGBA(150, 35, 30));

    // Setup 2.5D Height Map for Book curvature
    // Book has elevated surface (height 45) arching towards center gutter
    canvas.setHeightCurveH(lpx, lpy, lpw, lph, 48, 28);
    canvas.setHeightCurveH(rpx, rpy, rpw, rph, 48, 28);

    // Spine has low height (gutter indentation)
    canvas.setHeightRect(bx + 134, by + 4, 6, bh - 8, 15);
}

} // namespace grimoire::render
