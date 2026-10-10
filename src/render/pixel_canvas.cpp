#include "grimoire/render/pixel_canvas.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace grimoire::render {

PixelCanvas::PixelCanvas(int width, int height) {
    resize(width, height);
}

void PixelCanvas::resize(int width, int height) {
    width_ = std::max(1, width);
    height_ = std::max(1, height);
    pixels_.resize(static_cast<size_t>(width_ * height_), 0xFF000000);
    heightMap_.resize(static_cast<size_t>(width_ * height_), 0);
}

void PixelCanvas::clear(ColorRGBA color) {
    uint32_t val = color.toU32();
    std::fill(pixels_.begin(), pixels_.end(), val);
}

void PixelCanvas::clearHeight(uint8_t h) {
    std::fill(heightMap_.begin(), heightMap_.end(), h);
}

void PixelCanvas::setPixel(int x, int y, ColorRGBA color) noexcept {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        pixels_[static_cast<size_t>(y * width_ + x)] = color.toU32();
    }
}

void PixelCanvas::setPixelBlend(int x, int y, ColorRGBA color) noexcept {
    if (x < 0 || x >= width_ || y < 0 || y >= height_ || color.a == 0) return;

    size_t idx = static_cast<size_t>(y * width_ + x);
    if (color.a == 255) {
        pixels_[idx] = color.toU32();
        return;
    }

    ColorRGBA dst = ColorRGBA::fromU32(pixels_[idx]);
    uint32_t alpha = color.a;
    uint32_t invAlpha = 255 - alpha;

    uint8_t r = static_cast<uint8_t>((color.r * alpha + dst.r * invAlpha) / 255);
    uint8_t g = static_cast<uint8_t>((color.g * alpha + dst.g * invAlpha) / 255);
    uint8_t b = static_cast<uint8_t>((color.b * alpha + dst.b * invAlpha) / 255);

    pixels_[idx] = ColorRGBA(r, g, b, 255).toU32();
}

ColorRGBA PixelCanvas::getPixel(int x, int y) const noexcept {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        return ColorRGBA::fromU32(pixels_[static_cast<size_t>(y * width_ + x)]);
    }
    return ColorRGBA(0, 0, 0, 0);
}

void PixelCanvas::fillRect(int x, int y, int w, int h, ColorRGBA color) {
    int x0 = std::clamp(x, 0, width_);
    int y0 = std::clamp(y, 0, height_);
    int x1 = std::clamp(x + w, 0, width_);
    int y1 = std::clamp(y + h, 0, height_);

    if (color.a == 255) {
        uint32_t val = color.toU32();
        for (int py = y0; py < y1; ++py) {
            size_t rowStart = static_cast<size_t>(py * width_ + x0);
            std::fill_n(pixels_.data() + rowStart, x1 - x0, val);
        }
    } else {
        for (int py = y0; py < y1; ++py) {
            for (int px = x0; px < x1; ++px) {
                setPixelBlend(px, py, color);
            }
        }
    }
}

void PixelCanvas::drawRect(int x, int y, int w, int h, ColorRGBA color) {
    if (w <= 0 || h <= 0) return;
    fillRect(x, y, w, 1, color);
    fillRect(x, y + h - 1, w, 1, color);
    fillRect(x, y, 1, h, color);
    fillRect(x + w - 1, y, 1, h, color);
}

void PixelCanvas::drawBevelBox(int x, int y, int w, int h, ColorRGBA fill, ColorRGBA lightBorder, ColorRGBA darkBorder) {
    fillRect(x + 1, y + 1, w - 2, h - 2, fill);
    fillRect(x, y, w - 1, 1, lightBorder);
    fillRect(x, y, 1, h - 1, lightBorder);
    fillRect(x, y + h - 1, w, 1, darkBorder);
    fillRect(x + w - 1, y, 1, h, darkBorder);
}

void PixelCanvas::drawOrnateFrame(int x, int y, int w, int h, ColorRGBA fill, ColorRGBA border, ColorRGBA rivet) {
    // Fill main interior
    fillRect(x + 2, y + 2, w - 4, h - 4, fill);

    // Double border
    drawRect(x, y, w, h, border);
    drawRect(x + 2, y + 2, w - 4, h - 4, border);

    // 4 Corner rivets
    setPixel(x + 1, y + 1, rivet);
    setPixel(x + w - 2, y + 1, rivet);
    setPixel(x + 1, y + h - 2, rivet);
    setPixel(x + w - 2, y + h - 2, rivet);

    // Intermediate rivets if wide enough
    if (w >= 40) {
        setPixel(x + w / 2, y + 1, rivet);
        setPixel(x + w / 2, y + h - 2, rivet);
    }
}

void PixelCanvas::drawCircle(int cx, int cy, int r, ColorRGBA color) {
    int x = r;
    int y = 0;
    int err = 0;

    while (x >= y) {
        setPixelBlend(cx + x, cy + y, color);
        setPixelBlend(cx + y, cy + x, color);
        setPixelBlend(cx - y, cy + x, color);
        setPixelBlend(cx - x, cy + y, color);
        setPixelBlend(cx - x, cy - y, color);
        setPixelBlend(cx - y, cy - x, color);
        setPixelBlend(cx + y, cy - x, color);
        setPixelBlend(cx + x, cy - y, color);

        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

void PixelCanvas::fillCircle(int cx, int cy, int r, ColorRGBA color) {
    for (int y = -r; y <= r; ++y) {
        int dx = static_cast<int>(std::sqrt(r * r - y * y));
        fillRect(cx - dx, cy + y, dx * 2 + 1, 1, color);
    }
}

void PixelCanvas::drawCircleGlow(int cx, int cy, int r, ColorRGBA color, float falloffPower) {
    int r2 = r * r;
    for (int dy = -r; dy <= r; ++dy) {
        int py = cy + dy;
        if (py < 0 || py >= height_) continue;
        for (int dx = -r; dx <= r; ++dx) {
            int px = cx + dx;
            if (px < 0 || px >= width_) continue;
            int d2 = dx * dx + dy * dy;
            if (d2 <= r2) {
                float distNorm = std::sqrt(static_cast<float>(d2)) / static_cast<float>(r);
                float alpha = std::pow(1.0f - distNorm, falloffPower);
                ColorRGBA c = color;
                c.a = static_cast<uint8_t>(std::clamp(alpha * static_cast<float>(color.a), 0.0f, 255.0f));
                setPixelBlend(px, py, c);
            }
        }
    }
}

void PixelCanvas::drawGlyph8x16(int x, int y, uint8_t c, ColorRGBA color) {
    const uint8_t* bitmap = getGlyph8x16Bitmap(c);
    for (int row = 0; row < 16; ++row) {
        int py = y + row;
        if (py < 0 || py >= height_) continue;
        uint8_t rowByte = bitmap[row];
        if (rowByte == 0) continue;

        for (int col = 0; col < 8; ++col) {
            if ((rowByte & (0x80 >> col)) != 0) {
                setPixelBlend(x + col, py, color);
            }
        }
    }
}

void PixelCanvas::drawText8x16(int x, int y, std::string_view text, ColorRGBA color) {
    int curX = x;
    int curY = y;
    for (char ch : text) {
        if (ch == '\n') {
            curX = x;
            curY += 16;
            continue;
        }
        drawGlyph8x16(curX, curY, static_cast<uint8_t>(ch), color);
        curX += 8;
    }
}

void PixelCanvas::drawGlyph5x7(int x, int y, uint8_t c, ColorRGBA color) {
    const uint8_t* cols = getGlyph5x7Bitmap(c);
    for (int col = 0; col < 5; ++col) {
        int px = x + col;
        if (px < 0 || px >= width_) continue;
        uint8_t colByte = cols[col];
        if (colByte == 0) continue;

        for (int row = 0; row < 7; ++row) {
            int py = y + row;
            if (py < 0 || py >= height_) continue;
            if ((colByte & (1 << row)) != 0) {
                setPixelBlend(px, py, color);
            }
        }
    }
}

void PixelCanvas::drawText5x7(int x, int y, std::string_view text, ColorRGBA color) {
    int curX = x;
    int curY = y;
    for (char ch : text) {
        if (ch == '\n') {
            curX = x;
            curY += 8;
            continue;
        }
        drawGlyph5x7(curX, curY, static_cast<uint8_t>(ch), color);
        curX += 6; // 5 px font + 1 px spacing
    }
}

void PixelCanvas::setHeightRect(int x, int y, int w, int h, uint8_t heightVal) {
    int x0 = std::clamp(x, 0, width_);
    int y0 = std::clamp(y, 0, height_);
    int x1 = std::clamp(x + w, 0, width_);
    int y1 = std::clamp(y + h, 0, height_);

    for (int py = y0; py < y1; ++py) {
        size_t rowStart = static_cast<size_t>(py * width_ + x0);
        std::fill_n(heightMap_.data() + rowStart, x1 - x0, heightVal);
    }
}

void PixelCanvas::setHeightCurveH(int x, int y, int w, int h, uint8_t midH, uint8_t edgeH) {
    int x0 = std::clamp(x, 0, width_);
    int y0 = std::clamp(y, 0, height_);
    int x1 = std::clamp(x + w, 0, width_);
    int y1 = std::clamp(y + h, 0, height_);

    float halfW = static_cast<float>(w) * 0.5f;
    for (int px = x0; px < x1; ++px) {
        float relX = (static_cast<float>(px - x) - halfW) / (halfW > 0.0f ? halfW : 1.0f);
        float arch = 1.0f - (relX * relX);
        uint8_t curH = static_cast<uint8_t>(edgeH + arch * static_cast<float>(midH - edgeH));

        for (int py = y0; py < y1; ++py) {
            heightMap_[static_cast<size_t>(py * width_ + px)] = curH;
        }
    }
}

// Procedural pixel icons matching screenshot
void PixelCanvas::drawIconDagger(int x, int y, ColorRGBA color, ColorRGBA hilt) {
    // 12x12 diagonal dagger
    setPixel(x + 9, y + 2, color);
    setPixel(x + 8, y + 3, color);
    setPixel(x + 7, y + 4, color);
    setPixel(x + 6, y + 5, color);
    setPixel(x + 5, y + 6, color);
    setPixel(x + 4, y + 7, color);

    // Crossguard
    setPixel(x + 3, y + 8, hilt);
    setPixel(x + 4, y + 9, hilt);
    setPixel(x + 5, y + 7, hilt);

    // Pommel
    setPixel(x + 2, y + 9, hilt);
    setPixel(x + 1, y + 10, hilt);
}

void PixelCanvas::drawIconQuill(int x, int y, ColorRGBA feather, ColorRGBA stem) {
    // Quill pen
    setPixel(x + 2, y + 10, stem);
    setPixel(x + 3, y + 9, stem);
    setPixel(x + 4, y + 8, stem);
    setPixel(x + 5, y + 7, stem);
    setPixel(x + 6, y + 5, stem);
    setPixel(x + 7, y + 3, stem);
    setPixel(x + 8, y + 2, stem);

    // Vane feathers
    setPixel(x + 4, y + 6, feather);
    setPixel(x + 5, y + 5, feather);
    setPixel(x + 6, y + 4, feather);
    setPixel(x + 7, y + 2, feather);
    setPixel(x + 8, y + 1, feather);
    setPixel(x + 9, y + 2, feather);
    setPixel(x + 8, y + 4, feather);
}

void PixelCanvas::drawIconKey(int x, int y, ColorRGBA color) {
    drawCircle(x + 4, y + 4, 3, color);
    fillRect(x + 6, y + 4, 6, 1, color);
    fillRect(x + 9, y + 5, 1, 2, color);
    fillRect(x + 11, y + 5, 1, 3, color);
}

void PixelCanvas::drawIconLock(int x, int y, ColorRGBA body, ColorRGBA shackle) {
    drawCircle(x + 6, y + 4, 3, shackle);
    fillRect(x + 2, y + 5, 9, 7, body);
    drawRect(x + 2, y + 5, 9, 7, ColorRGBA(40, 30, 20));
    setPixel(x + 6, y + 7, ColorRGBA(20, 15, 10));
    fillRect(x + 6, y + 8, 1, 2, ColorRGBA(20, 15, 10));
}

void PixelCanvas::drawIconCompass(int x, int y, ColorRGBA ring, ColorRGBA needle) {
    drawCircle(x + 6, y + 6, 5, ring);
    setPixel(x + 6, y + 6, ring);
    // Needle
    setPixel(x + 6, y + 3, needle);
    setPixel(x + 6, y + 4, needle);
    setPixel(x + 6, y + 8, ColorRGBA(180, 180, 180));
    setPixel(x + 6, y + 9, ColorRGBA(180, 180, 180));
}

void PixelCanvas::drawIconLantern(int x, int y, ColorRGBA metal, ColorRGBA flame) {
    // Handle
    drawRect(x + 4, y + 1, 5, 2, metal);
    // Roof
    fillRect(x + 3, y + 3, 7, 2, metal);
    // Glass pane
    fillRect(x + 4, y + 5, 5, 5, ColorRGBA(60, 45, 25));
    // Flame
    fillCircle(x + 6, y + 7, 1, flame);
    setPixel(x + 6, y + 6, ColorRGBA(255, 250, 200));
    // Base
    fillRect(x + 3, y + 10, 7, 2, metal);
}

void PixelCanvas::drawIconStar(int x, int y, ColorRGBA glow, ColorRGBA core) {
    // 4-pointed radiant spark
    setPixel(x + 6, y + 2, glow);
    setPixel(x + 6, y + 10, glow);
    setPixel(x + 2, y + 6, glow);
    setPixel(x + 10, y + 6, glow);

    fillRect(x + 5, y + 4, 3, 5, glow);
    fillRect(x + 4, y + 5, 5, 3, glow);
    fillRect(x + 5, y + 5, 3, 3, core);
}

void PixelCanvas::drawIconTrophy(int x, int y, ColorRGBA gold) {
    // Cup
    fillRect(x + 4, y + 3, 5, 4, gold);
    fillRect(x + 5, y + 7, 3, 2, gold);
    // Handles
    setPixel(x + 2, y + 4, gold);
    setPixel(x + 2, y + 5, gold);
    setPixel(x + 10, y + 4, gold);
    setPixel(x + 10, y + 5, gold);
    // Stem & Base
    fillRect(x + 6, y + 9, 1, 2, gold);
    fillRect(x + 4, y + 11, 5, 1, gold);
}

void PixelCanvas::drawIconEmber(int x, int y, ColorRGBA fire) {
    // Small flame teardrop
    setPixel(x + 3, y + 1, fire);
    fillRect(x + 2, y + 2, 3, 2, fire);
    fillRect(x + 1, y + 4, 5, 2, fire);
    setPixel(x + 3, y + 4, ColorRGBA(255, 240, 160));
    fillRect(x + 2, y + 6, 3, 1, fire);
}

void PixelCanvas::drawIconSword(int x, int y, ColorRGBA blade, ColorRGBA guard) {
    // Vertical sword
    fillRect(x + 5, y + 1, 2, 7, blade);
    setPixel(x + 5, y + 0, blade);
    fillRect(x + 3, y + 8, 6, 1, guard);
    fillRect(x + 5, y + 9, 2, 2, guard);
    setPixel(x + 5, y + 11, blade);
}

void PixelCanvas::drawIconCrest(int x, int y, ColorRGBA crest) {
    // 8-petal rosette
    drawCircle(x + 6, y + 6, 4, crest);
    setPixel(x + 6, y + 6, crest);
    setPixel(x + 6, y + 3, crest);
    setPixel(x + 6, y + 9, crest);
    setPixel(x + 3, y + 6, crest);
    setPixel(x + 9, y + 6, crest);
}

// 2.5D Dynamic Lighting Pass
void PixelCanvas::applyLighting(const std::vector<LightSource2D>& lights, float timeSeconds) {
    if (lights.empty()) return;

    // Precalculate light properties and procedural flicker
    struct LightWork {
        float x;
        float y;
        float z;
        float radius;
        float radiusSq;
        float r;
        float g;
        float b;
    };

    std::vector<LightWork> activeLights;
    activeLights.reserve(lights.size());

    for (const auto& l : lights) {
        float flicker = 1.0f + 0.08f * std::sin(timeSeconds * 12.0f * l.flickerSpeed + l.flickerPhase)
                             * std::cos(timeSeconds * 7.3f * l.flickerSpeed + l.flickerPhase * 1.3f);
        float mult = l.intensity * flicker;

        LightWork lw;
        lw.x = l.x;
        lw.y = l.y;
        lw.z = l.z;
        lw.radius = l.radius;
        lw.radiusSq = l.radius * l.radius;
        lw.r = (static_cast<float>(l.color.r) / 255.0f) * mult;
        lw.g = (static_cast<float>(l.color.g) / 255.0f) * mult;
        lw.b = (static_cast<float>(l.color.b) / 255.0f) * mult;
        activeLights.push_back(lw);
    }

    // Process every pixel
    for (int py = 0; py < height_; ++py) {
        size_t rowOffset = static_cast<size_t>(py * width_);
        for (int px = 0; px < width_; ++px) {
            size_t idx = rowOffset + static_cast<size_t>(px);

            ColorRGBA src = ColorRGBA::fromU32(pixels_[idx]);
            float pz = static_cast<float>(heightMap_[idx]) * 0.25f;

            // Height bump pseudo-normal
            float nx = 0.0f;
            float ny = 0.0f;
            if (px > 0 && px + 1 < width_) {
                nx = (static_cast<float>(heightMap_[idx - 1]) - static_cast<float>(heightMap_[idx + 1])) * 0.04f;
            }
            if (py > 0 && py + 1 < height_) {
                ny = (static_cast<float>(heightMap_[idx - width_]) - static_cast<float>(heightMap_[idx + width_])) * 0.04f;
            }
            float nz = 1.0f;

            // Ambient base light (soft midnight desk shadow)
            float totalR = 0.20f;
            float totalG = 0.17f;
            float totalB = 0.15f;

            for (const auto& l : activeLights) {
                float dx = l.x - static_cast<float>(px);
                float dy = l.y - static_cast<float>(py);
                float dz = l.z - pz;
                float distSq = dx * dx + dy * dy + dz * dz;

                if (distSq < l.radiusSq) {
                    float dist = std::sqrt(distSq);
                    // Quadratic falloff
                    float atten = 1.0f / (1.0f + 0.005f * dist + 0.000035f * distSq);

                    // Normal bump
                    float dotN = (dx * nx + dy * ny + dz * nz) / (dist > 0.001f ? dist : 1.0f);
                    float diffuse = std::clamp(dotN, 0.5f, 1.4f);

                    float factor = atten * diffuse;
                    totalR += l.r * factor;
                    totalG += l.g * factor;
                    totalB += l.b * factor;
                }
            }

            // Warm candle color grading
            uint8_t outR = static_cast<uint8_t>(std::clamp(static_cast<float>(src.r) * totalR, 0.0f, 255.0f));
            uint8_t outG = static_cast<uint8_t>(std::clamp(static_cast<float>(src.g) * totalG, 0.0f, 255.0f));
            uint8_t outB = static_cast<uint8_t>(std::clamp(static_cast<float>(src.b) * totalB, 0.0f, 255.0f));

            pixels_[idx] = ColorRGBA(outR, outG, outB, src.a).toU32();
        }
    }
}

void PixelCanvas::applyFoliageShadows(float timeSeconds) {
    // Subtle animated organic vine/leaf silhouettes in the corners
    // Bottom-Left foliage cluster
    float sway = std::sin(timeSeconds * 1.5f) * 1.5f;

    auto drawLeaf = [this](int lx, int ly, int size, float angle) {
        float cosA = std::cos(angle);
        float sinA = std::sin(angle);
        for (int dy = -size; dy <= size; ++dy) {
            for (int dx = -size; dx <= size; ++dx) {
                float u = dx * cosA - dy * sinA;
                float v = dx * sinA + dy * cosA;
                if ((u * u) / (size * size) + (v * v) / (size * 0.35f * size * 0.35f) <= 1.0f) {
                    setPixelBlend(lx + dx, ly + dy, ColorRGBA(5, 7, 5, 210));
                }
            }
        }
    };

    // Bottom left corner leaves
    drawLeaf(15 + static_cast<int>(sway), 340, 32, -0.6f);
    drawLeaf(45 + static_cast<int>(sway * 0.8f), 350, 26, -0.3f);
    drawLeaf(20, 310 + static_cast<int>(sway * 0.5f), 24, -1.0f);

    // Bottom right corner leaves
    drawLeaf(620 - static_cast<int>(sway), 345, 30, 0.7f);
    drawLeaf(590 - static_cast<int>(sway * 0.7f), 355, 24, 0.4f);
    drawLeaf(630, 315 + static_cast<int>(sway * 0.6f), 22, 1.1f);

    // Top left subtle branch
    drawLeaf(10, 15 + static_cast<int>(sway * 0.4f), 20, 0.8f);
}

void PixelCanvas::applyVignette(float strength) {
    float cx = static_cast<float>(width_) * 0.5f;
    float cy = static_cast<float>(height_) * 0.5f;
    float maxDistSq = cx * cx + cy * cy;

    for (int py = 0; py < height_; ++py) {
        float dy = static_cast<float>(py) - cy;
        size_t rowOffset = static_cast<size_t>(py * width_);
        for (int px = 0; px < width_; ++px) {
            float dx = static_cast<float>(px) - cx;
            float distSq = dx * dx + dy * dy;
            float v = 1.0f - (distSq / maxDistSq) * strength;
            v = std::clamp(v, 0.15f, 1.0f);

            size_t idx = rowOffset + static_cast<size_t>(px);
            ColorRGBA c = ColorRGBA::fromU32(pixels_[idx]);
            c.r = static_cast<uint8_t>(static_cast<float>(c.r) * v);
            c.g = static_cast<uint8_t>(static_cast<float>(c.g) * v);
            c.b = static_cast<uint8_t>(static_cast<float>(c.b) * v);
            pixels_[idx] = c.toU32();
        }
    }
}

} // namespace grimoire::render
