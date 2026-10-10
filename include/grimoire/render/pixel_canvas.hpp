#pragma once

#include "grimoire/core/types.hpp"
#include "pixel_font.hpp"
#include <vector>
#include <string_view>
#include <cmath>

namespace grimoire::render {

using namespace grimoire::core;

class PixelCanvas {
public:
    PixelCanvas(int width = 640, int height = 360);

    void resize(int width, int height);
    [[nodiscard]] int getWidth() const noexcept { return width_; }
    [[nodiscard]] int getHeight() const noexcept { return height_; }
    [[nodiscard]] const uint32_t* getPixels() const noexcept { return pixels_.data(); }
    [[nodiscard]] uint32_t* getPixels() noexcept { return pixels_.data(); }
    [[nodiscard]] size_t getPixelCount() const noexcept { return pixels_.size(); }
    [[nodiscard]] size_t getByteSize() const noexcept { return pixels_.size() * sizeof(uint32_t); }

    void clear(ColorRGBA color = ColorRGBA(18, 14, 12, 255));
    void clearHeight(uint8_t h = 0);

    void setPixel(int x, int y, ColorRGBA color) noexcept;
    void setPixelBlend(int x, int y, ColorRGBA color) noexcept;
    [[nodiscard]] ColorRGBA getPixel(int x, int y) const noexcept;

    void fillRect(int x, int y, int w, int h, ColorRGBA color);
    void drawRect(int x, int y, int w, int h, ColorRGBA color);
    void drawBevelBox(int x, int y, int w, int h, ColorRGBA fill, ColorRGBA lightBorder, ColorRGBA darkBorder);
    void drawOrnateFrame(int x, int y, int w, int h, ColorRGBA fill, ColorRGBA border, ColorRGBA rivet);

    void drawCircle(int cx, int cy, int r, ColorRGBA color);
    void fillCircle(int cx, int cy, int r, ColorRGBA color);
    void drawCircleGlow(int cx, int cy, int r, ColorRGBA color, float falloffPower = 1.8f);

    void drawGlyph8x16(int x, int y, uint8_t c, ColorRGBA color);
    void drawText8x16(int x, int y, std::string_view text, ColorRGBA color);

    void drawGlyph5x7(int x, int y, uint8_t c, ColorRGBA color);
    void drawText5x7(int x, int y, std::string_view text, ColorRGBA color);

    // Height / 2.5D normal mapping
    void setHeightRect(int x, int y, int w, int h, uint8_t height);
    void setHeightCurveH(int x, int y, int w, int h, uint8_t midH, uint8_t edgeH);

    // Procedural pixel icons
    void drawIconDagger(int x, int y, ColorRGBA color, ColorRGBA hilt);
    void drawIconQuill(int x, int y, ColorRGBA feather, ColorRGBA stem);
    void drawIconKey(int x, int y, ColorRGBA color);
    void drawIconLock(int x, int y, ColorRGBA body, ColorRGBA shackle);
    void drawIconCompass(int x, int y, ColorRGBA ring, ColorRGBA needle);
    void drawIconLantern(int x, int y, ColorRGBA metal, ColorRGBA flame);
    void drawIconStar(int x, int y, ColorRGBA glow, ColorRGBA core);
    void drawIconTrophy(int x, int y, ColorRGBA gold);
    void drawIconEmber(int x, int y, ColorRGBA fire);
    void drawIconSword(int x, int y, ColorRGBA blade, ColorRGBA guard);
    void drawIconCrest(int x, int y, ColorRGBA crest);

    // Dynamic 2.5D Lighting & Post-Processing
    void applyLighting(const std::vector<LightSource2D>& lights, float timeSeconds);
    void applyFoliageShadows(float timeSeconds);
    void applyVignette(float strength = 0.45f);

private:
    int width_{640};
    int height_{360};
    std::vector<uint32_t> pixels_;
    std::vector<uint8_t> heightMap_;
};

} // namespace grimoire::render
