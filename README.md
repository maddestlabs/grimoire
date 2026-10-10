# Grimoire Engine (Scribe & Ember)
> **Atmospheric Pixel-Art Writing Game Engine**  
> *Extracted and adapted from the Eatsbits C++20 real-time workstation core.*

Inspired by the top-down 2.5D aesthetic of *Slots & Daggers*, **Grimoire** is a distraction-free, gamified creative writing environment and specialized engine. It features an open retro-monospace grimoire, dynamic flickering candle point-light shaders, diegetic RPG progression mechanics (Word Bank, Plot Devices, Story Shop), and tactile lo-fi audio powered by an authentic Super Nintendo (SNES / SPC700) sound core.

---

## 1. Architectural Blueprint & Eatsbits Extraction

```
       [ EATSBITS DAW CORE ]
                 │
                 ├── STRIP: ──> TB-303, DaisySP Modular Nodes, VCV Rack, CLAP hosting, MIDI Sequencer
                 │
                 └── RETAIN & ADAPT:
                       ├── Sokol GFX + App (Native multi-pass GPU swapchain pipeline)
                       ├── Monospace Text Engine (TextDocument buffer, undo/redo, 2-page pagination)
                       ├── SNES / SPC700 Audio Core (32 kHz lo-fi atmosphere & tactile Foley)
                       ├── 2.5D Normal-Mapped Pixel Canvas (640x360 chunky retro presentation)
                       └── Diegetic Progression (Word Goals, Embers, Plot Devices, Story Shop)
```

---

## 2. Key Subsystems

### 2.1 The Two-Page Manuscript Text Engine
- **Layout**: Centered two-page open grimoire (Left and Right page spread) with automatic word wrapping and multi-spread pagination.
- **Prose Highlighting ("Plot Highlighting")**:
  - `"Quotes / Dialogue"` in warm amber/cream (`#c37d2d`).
  - `[Character Names]` or `@Mentions` in royal purple (`#6e4ba0`).
  - `*Emphases / Actions*` in soft italicized gold (`#af8c32`).
  - `- Bullet points` or `# Headers` in distinct tones.
  - Standard text in deep manuscript ink (`#2a221c`).
- **Core Editor**: Backed by `grimoire::core::TextDocument` with full keyboard navigation (arrows, page up/down, home/end), selection, undo/redo (`Ctrl+Z`, `Ctrl+Y`), and auto-scroll/page synchronization.

### 2.2 2.5D Candle-Lit Rendering Pipeline
- **Canvas Resolution**: $640 \times 360$ chunky retro pixel framebuffer.
- **Dynamic 2.5D Candle Lights**:
  - Multi-point lighting with quadratic distance falloff:
    $$\text{atten} = \frac{1}{1.0 + 0.005 \cdot d + 0.000035 \cdot d^2}$$
  - Multi-frequency procedural candle flicker:
    $$\text{flicker} = 1.0 + 0.08 \cdot \sin(12 t) \cdot \cos(7.3 t)$$
  - Normal / height bump calculations for curled page edges, spine curvature, and desk wood grain.
  - Organic animated leaf silhouette shadows in corners and subtle edge vignette.
- **Presentation Pass**: Sokol GFX (Direct3D 11 on Windows, WebGL on web) with nearest-neighbor integer aspect-ratio scaling to native window dimensions.

### 2.3 Lo-Fi SNES / SPC700 Audio Core
- **Tactile Keypress Foley**:
  - Mechanical typewriter and quill key clicks generated with $\pm 3\%$ micro-pitch randomization.
  - Distinct auditory feedback for alphanumeric keys, spacebar wooden thuds, backspace eraser clicks, and carriage-return chime latches.
- **Atmospheric Soundscapes**:
  - Procedural rain loop filtered with pink/brownian noise and windowpane high-pass sizzle.
  - Periodic candle wax sizzling and micro-pops.
  - 3-voice celesta chord progressions with 8-tap FIR echo/reverb on milestones and achievements.

### 2.4 Diegetic RPG Progression Mechanics
- **The Word Bank**: Live progress tracking against customizable word goals (e.g., $750 / 1000$ words).
- **Ink Embers**: Currency earned through sustained writing (1 ember per 25 words typed + milestone bonuses).
- **Active Plot Devices**: 3 clickable slots on the desk:
  1. *Dagger of Peril*: Generates conflict and climax constraints.
  2. *Quill of Revelation*: Generates lore and worldbuilding prompts.
  3. *Skeleton Key*: Generates mystery and secret discovery prompts.
- **Story Upgrade Shop**:
  - *Scented Amber Wax*: Warms candle color temperature and enhances flicker smoothness.
  - *Sanctuary Lantern*: Summons a calming rainstorm soundscape.
  - *Golden Starburst Nib*: Gilds page edges and unlocks enchanted ink flourishes.
- **Persistence & Markdown Export**:
  - `Ctrl+S` exports the full manuscript with YAML frontmatter containing word count, goals, embers, and unlocked traits.

---

## 3. Directory Layout

```
c:\git\grimoire/
├── CMakeLists.txt              # Standard C++20 CMake configuration
├── build.ps1                   # Automated build, test, and run script
├── README.md
├── include/grimoire/
│   ├── core/
│   │   ├── types.hpp           # ColorRGBA, Rect2I, Point2I, LightSource2D
│   │   ├── text_document.hpp   # High-performance gap buffer & undo stack
│   │   └── story_state.hpp     # Word goals, embers, plot devices, shop items
│   ├── audio/
│   │   ├── snes_dsp_core.hpp   # 16-bit SNES S-DSP emulator with 8-tap FIR echo
│   │   ├── prng.hpp            # Fast deterministic Mulberry32 generator
│   │   └── audio_engine.hpp    # Miniaudio stream + Foley and rain synthesizers
│   ├── render/
│   │   ├── pixel_canvas.hpp    # 640x360 software rasterizer & 2.5D lighting
│   │   ├── pixel_font.hpp      # 5x7 micro font & 8x16 monospace font
│   │   ├── monospace_font_8x16.hpp
│   │   └── scene_renderer.hpp  # Compositor for desk, book, lighting, and HUD
│   ├── editor/
│   │   ├── prose_highlighter.hpp # Dialogue, entity, and emphasis tokenizer
│   │   └── manuscript_editor.hpp # Two-page layout, wrapping, cursor, and input
│   └── ui/
│       └── diegetic_ui.hpp     # Hit-testing, tooltips, and interactive desk props
├── src/
│   ├── core/story_state.cpp
│   ├── audio/audio_engine.cpp
│   ├── render/
│   │   ├── pixel_canvas.cpp
│   │   ├── pixel_font.cpp
│   │   ├── monospace_font_8x16.cpp
│   │   └── scene_renderer.cpp
│   ├── editor/
│   │   ├── manuscript_editor.cpp
│   │   └── prose_highlighter.cpp
│   ├── ui/diegetic_ui.cpp
│   └── main.cpp                # Sokol App & Direct3D 11 presentation pipeline
├── tests/
│   └── test_grimoire.cpp       # Automated unit test suite (6 test groups)
└── third_party/
    ├── sokol/                  # Header-only Sokol graphics & app libraries
    └── miniaudio/              # Lightweight audio device management
```

---

## 4. Building and Running

### Prerequisites
- Windows 10/11 with Visual Studio 2022 (MSVC C++20)
- CMake 3.20+
- PowerShell

### Commands
```powershell
# 1. Compile and execute automated test suite
.\build.ps1 -Test

# 2. Launch Grimoire application
.\build.ps1 -Run

# 3. Clean rebuild and launch
.\build.ps1 -Clean -Run
```

---

## 5. Acceptance & Benchmark Results
- **Binary Footprint**: Standalone executable `grimoire.exe` is **663 KB** (vs. 15 MB limit).
- **Compilation Speed**: Clean build compiles in **~11.9 seconds**.
- **Test Suite**: 6/6 test suites passed cleanly.
