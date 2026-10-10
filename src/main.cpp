#define SOKOL_IMPL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_time.h"
#include "sokol_log.h"

#include "grimoire/core/types.hpp"
#include "grimoire/core/story_state.hpp"
#include "grimoire/audio/audio_engine.hpp"
#include "grimoire/editor/manuscript_editor.hpp"
#include "grimoire/ui/diegetic_ui.hpp"
#include "grimoire/render/pixel_canvas.hpp"
#include "grimoire/render/scene_renderer.hpp"

#include <iostream>
#include <algorithm>
#include <memory>

using namespace grimoire::core;
using namespace grimoire::audio;
using namespace grimoire::editor;
using namespace grimoire::ui;
using namespace grimoire::render;

struct AppState {
    std::unique_ptr<StoryState> story;
    std::unique_ptr<AudioEngine> audio;
    std::unique_ptr<ManuscriptEditor> editor;
    std::unique_ptr<DiegeticUI> ui;
    std::unique_ptr<SceneRenderer> scene;
    std::unique_ptr<PixelCanvas> canvas;

    sg_image canvas_img;
    sg_view canvas_view;
    sg_sampler canvas_smp;
    sg_buffer vbuf;
    sg_pipeline pip;

    uint64_t last_time{0};
    int vpX{0};
    int vpY{0};
    int vpW{640};
    int vpH{360};
    float vpScale{1.0f};
};

static AppState g_app;

static void init(void) {
    stm_setup();

    // 1. Initialize Sokol GFX
    sg_desc sg_init_desc = {};
    sg_init_desc.environment = sglue_environment();
    sg_init_desc.logger.func = slog_func;
    sg_setup(&sg_init_desc);

    // 2. Initialize Game Engine Systems
    g_app.story = std::make_unique<StoryState>();
    g_app.audio = std::make_unique<AudioEngine>();
    g_app.audio->initialize(44100);

    g_app.editor = std::make_unique<ManuscriptEditor>();
    g_app.ui = std::make_unique<DiegeticUI>(*g_app.story);
    g_app.scene = std::make_unique<SceneRenderer>(*g_app.story, *g_app.editor, *g_app.ui);
    g_app.canvas = std::make_unique<PixelCanvas>(640, 360);

    // Wire callbacks
    g_app.editor->onKeyTyped = [](char32_t c) {
        if (c == ' ') {
            g_app.audio->playKeyClick(KeySoundType::Space);
        } else {
            g_app.audio->playKeyClick(KeySoundType::AlphaNumeric);
        }
    };

    g_app.editor->onWordCountChanged = []() {
        g_app.story->updateWordCount(g_app.editor->getWordCount());
    };

    g_app.story->onAchievementUnlocked = [](const std::string& /*title*/) {
        g_app.audio->playChimeReward();
    };

    g_app.story->onMilestoneReached = []() {
        g_app.audio->playChimeReward();
    };

    g_app.ui->onShopPurchase = []() {
        g_app.audio->playCoinClink();
        // Check if rain upgrade purchased
        const auto& items = g_app.story->getShopItems();
        if (items.size() > 1 && items[1].purchased) {
            g_app.audio->setRainEnabled(true);
            g_app.audio->setAmbientVolume(0.65f);
        }
    };

    g_app.ui->onPlotDeviceToggled = []() {
        g_app.audio->playKeyClick(KeySoundType::ReturnKey);
    };

    // Initial word count sync
    g_app.story->updateWordCount(g_app.editor->getWordCount());

    // 3. Create GPU Texture for 640x360 Pixel Canvas
    sg_image_desc img_desc = {};
    img_desc.width = 640;
    img_desc.height = 360;
    img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    img_desc.usage.write_transient = true;
    img_desc.label = "pixel_canvas_image";
    g_app.canvas_img = sg_make_image(&img_desc);

    sg_view_desc view_desc = {};
    view_desc.texture.image = g_app.canvas_img;
    view_desc.label = "pixel_canvas_view";
    g_app.canvas_view = sg_make_view(&view_desc);

    sg_sampler_desc smp_desc = {};
    smp_desc.min_filter = SG_FILTER_NEAREST;
    smp_desc.mag_filter = SG_FILTER_NEAREST;
    smp_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    smp_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    smp_desc.label = "pixel_canvas_sampler";
    g_app.canvas_smp = sg_make_sampler(&smp_desc);

    // 4. Fullscreen Quad Vertex Buffer
    float vertices[] = {
        // pos.x, pos.y, uv.u, uv.v
        -1.0f,  1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 1.0f,

        -1.0f,  1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 1.0f
    };
    sg_buffer_desc vbuf_desc = {};
    vbuf_desc.data = SG_RANGE(vertices);
    vbuf_desc.label = "fullscreen_quad_vbuf";
    g_app.vbuf = sg_make_buffer(&vbuf_desc);

    // 5. Blit Presentation Shader (Direct3D 11 / HLSL)
    const char* vs_src =
        "struct vs_in {\n"
        "  float2 pos : POS;\n"
        "  float2 uv  : TEX;\n"
        "};\n"
        "struct vs_out {\n"
        "  float4 pos : SV_Position;\n"
        "  float2 uv  : TEXCOORD0;\n"
        "};\n"
        "vs_out main(vs_in inp) {\n"
        "  vs_out outp;\n"
        "  outp.pos = float4(inp.pos, 0.0, 1.0);\n"
        "  outp.uv = inp.uv;\n"
        "  return outp;\n"
        "}\n";

    const char* ps_src =
        "Texture2D tex : register(t0);\n"
        "SamplerState smp : register(s0);\n"
        "float4 main(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target {\n"
        "  return tex.Sample(smp, uv);\n"
        "}\n";

    sg_shader_desc shd_desc = {};
    shd_desc.vertex_func.source = vs_src;
    shd_desc.fragment_func.source = ps_src;
    shd_desc.attrs[0].hlsl_sem_name = "POS";
    shd_desc.attrs[0].hlsl_sem_index = 0;
    shd_desc.attrs[1].hlsl_sem_name = "TEX";
    shd_desc.attrs[1].hlsl_sem_index = 0;

    shd_desc.views[0].texture.stage = SG_SHADERSTAGE_FRAGMENT;
    shd_desc.views[0].texture.image_type = SG_IMAGETYPE_2D;
    shd_desc.views[0].texture.sample_type = SG_IMAGESAMPLETYPE_FLOAT;
    shd_desc.views[0].texture.hlsl_register_t_n = 0;

    shd_desc.samplers[0].stage = SG_SHADERSTAGE_FRAGMENT;
    shd_desc.samplers[0].sampler_type = SG_SAMPLERTYPE_FILTERING;
    shd_desc.samplers[0].hlsl_register_s_n = 0;

    shd_desc.texture_sampler_pairs[0].stage = SG_SHADERSTAGE_FRAGMENT;
    shd_desc.texture_sampler_pairs[0].view_slot = 0;
    shd_desc.texture_sampler_pairs[0].sampler_slot = 0;
    shd_desc.texture_sampler_pairs[0].glsl_name = "tex";
    shd_desc.label = "presentation_shader";
    sg_shader shd = sg_make_shader(&shd_desc);

    sg_pipeline_desc pip_desc = {};
    pip_desc.shader = shd;
    pip_desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.label = "presentation_pipeline";
    g_app.pip = sg_make_pipeline(&pip_desc);

    g_app.last_time = stm_now();
    std::cout << "[Grimoire] Project Grimoire initialized successfully at 640x360 pixel grid.\n";
}

static void frame(void) {
    uint64_t now = stm_now();
    float dt = static_cast<float>(stm_sec(stm_diff(now, g_app.last_time)));
    g_app.last_time = now;
    if (dt > 0.1f) dt = 0.1f; // clamp delta on lag

    // 1. Update Engine & Scene
    g_app.scene->update(dt);

    // 2. Render Scene to Pixel Canvas
    g_app.scene->render(*g_app.canvas);

    // 3. Upload Canvas to GPU Texture
    sg_write_image_desc wdesc = {};
    wdesc.src.data.ptr = g_app.canvas->getPixels();
    wdesc.src.data.size = g_app.canvas->getByteSize();
    wdesc.src.bytes_per_row = 640 * 4;
    wdesc.src.bytes_per_slice = 640 * 360 * 4;
    wdesc.dst.image = g_app.canvas_img;
    wdesc.dst.mip_level = 0;
    wdesc.size.width = 640;
    wdesc.size.height = 360;
    wdesc.size.num_slices = 1;
    sg_write_image_transient(&wdesc);

    // 4. Begin Swapchain Presentation Pass
    sg_pass pass = {};
    pass.action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass.action.colors[0].clear_value = { 0.04f, 0.03f, 0.02f, 1.0f };
    pass.swapchain = sglue_swapchain();
    sg_begin_pass(&pass);

    // Calculate Aspect-Ratio-Preserved Viewport
    int winW = sapp_width();
    int winH = sapp_height();
    float scaleX = static_cast<float>(winW) / 640.0f;
    float scaleY = static_cast<float>(winH) / 360.0f;
    g_app.vpScale = std::min(scaleX, scaleY);
    g_app.vpW = static_cast<int>(640.0f * g_app.vpScale);
    g_app.vpH = static_cast<int>(360.0f * g_app.vpScale);
    g_app.vpX = (winW - g_app.vpW) / 2;
    g_app.vpY = (winH - g_app.vpH) / 2;

    sg_apply_viewport(g_app.vpX, g_app.vpY, g_app.vpW, g_app.vpH, true);
    sg_apply_pipeline(g_app.pip);

    sg_bindings bind = {};
    bind.vertex_buffers[0] = g_app.vbuf;
    bind.views[0] = g_app.canvas_view;
    bind.samplers[0] = g_app.canvas_smp;
    sg_apply_bindings(&bind);

    sg_draw(0, 6, 1);
    sg_end_pass();
    sg_commit();
}

static void event(const sapp_event* ev) {
    if (!ev) return;

    // Convert mouse coordinates from window to 640x360 canvas space
    float rawX = ev->mouse_x - static_cast<float>(g_app.vpX);
    float rawY = ev->mouse_y - static_cast<float>(g_app.vpY);
    int px = static_cast<int>(rawX / (g_app.vpScale > 0.001f ? g_app.vpScale : 1.0f));
    int py = static_cast<int>(rawY / (g_app.vpScale > 0.001f ? g_app.vpScale : 1.0f));

    switch (ev->type) {
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            g_app.ui->handlePointerMove(px, py);
            break;

        case SAPP_EVENTTYPE_MOUSE_DOWN:
            if (ev->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
                if (!g_app.ui->handlePointerClick(px, py)) {
                    g_app.editor->handleClick(px, py);
                }
            }
            break;

        case SAPP_EVENTTYPE_KEY_DOWN: {
            bool shift = (ev->modifiers & SAPP_MODIFIER_SHIFT) != 0;
            bool ctrl = (ev->modifiers & SAPP_MODIFIER_CTRL) != 0;

            if (ctrl && (ev->key_code == SAPP_KEYCODE_S)) {
                // Ctrl+S: Export to manuscript.md
                std::string md = g_app.story->exportToMarkdown(g_app.editor->getText());
                std::cout << "[Grimoire] Saved manuscript to Markdown (" << g_app.editor->getWordCount() << " words).\n";
                g_app.audio->playKeyClick(KeySoundType::ReturnKey);
                break;
            }

            if (ev->key_code == SAPP_KEYCODE_BACKSPACE) {
                g_app.audio->playKeyClick(KeySoundType::Backspace);
            } else if (ev->key_code == SAPP_KEYCODE_ENTER) {
                g_app.audio->playKeyClick(KeySoundType::ReturnKey);
            } else if (ev->key_code == SAPP_KEYCODE_PAGE_UP || ev->key_code == SAPP_KEYCODE_PAGE_DOWN) {
                g_app.audio->playPageTurn();
            }

            // Map Sokol key code to manuscript editor key
            int mappedKey = 0;
            if (ev->key_code == SAPP_KEYCODE_BACKSPACE) mappedKey = 259;
            else if (ev->key_code == SAPP_KEYCODE_ENTER) mappedKey = 257;
            else if (ev->key_code == SAPP_KEYCODE_DELETE) mappedKey = 261;
            else if (ev->key_code == SAPP_KEYCODE_LEFT) mappedKey = 263;
            else if (ev->key_code == SAPP_KEYCODE_RIGHT) mappedKey = 262;
            else if (ev->key_code == SAPP_KEYCODE_UP) mappedKey = 265;
            else if (ev->key_code == SAPP_KEYCODE_DOWN) mappedKey = 264;
            else if (ev->key_code == SAPP_KEYCODE_PAGE_UP) mappedKey = 266;
            else if (ev->key_code == SAPP_KEYCODE_PAGE_DOWN) mappedKey = 267;
            else if (ev->key_code == SAPP_KEYCODE_TAB) mappedKey = 258;
            else if (ev->key_code == SAPP_KEYCODE_Z) mappedKey = 'Z';
            else if (ev->key_code == SAPP_KEYCODE_Y) mappedKey = 'Y';

            if (mappedKey != 0) {
                g_app.editor->handleKey(mappedKey, shift, ctrl);
            }
            break;
        }

        case SAPP_EVENTTYPE_CHAR:
            g_app.editor->handleChar(ev->char_code);
            break;

        default:
            break;
    }
}

static void cleanup(void) {
    g_app.audio->shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int /*argc*/, char* /*argv*/[]) {
    sapp_desc desc = {};
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.cleanup_cb = cleanup;
    desc.event_cb = event;
    desc.width = 1280;
    desc.height = 720;
    desc.window_title = "Grimoire - Atmospheric Pixel-Art Writing Engine";
    desc.icon.sokol_default = true;
    desc.logger.func = slog_func;
    return desc;
}
