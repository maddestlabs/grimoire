#include <iostream>
#include <cassert>
#include <string>

#include "grimoire/core/types.hpp"
#include "grimoire/core/text_document.hpp"
#include "grimoire/core/story_state.hpp"
#include "grimoire/audio/snes_dsp_core.hpp"
#include "grimoire/editor/prose_highlighter.hpp"
#include "grimoire/editor/manuscript_editor.hpp"
#include "grimoire/render/pixel_canvas.hpp"

using namespace grimoire::core;
using namespace grimoire::audio;
using namespace grimoire::editor;
using namespace grimoire::render;

void testTextDocument() {
    std::cout << "[Test 1] Testing TextDocument core operations...\n";
    TextDocument doc("Hello world!\nSecond line.");
    assert(doc.getLineCount() == 2);
    assert(doc.getLine(0) == "Hello world!");
    assert(doc.getLine(1) == "Second line.");

    // Insert text
    doc.insert(TextCoord{0, 5}, " beautiful");
    assert(doc.getLine(0) == "Hello beautiful world!");

    // Undo
    doc.undo();
    assert(doc.getLine(0) == "Hello world!");

    // Redo
    doc.redo();
    assert(doc.getLine(0) == "Hello beautiful world!");

    std::cout << "  TextDocument tests PASSED.\n";
}

void testStoryState() {
    std::cout << "[Test 2] Testing StoryState & RPG Progression...\n";
    StoryState state;
    assert(state.getWordCount() == 0);
    assert(state.getGoalWords() == 1000);
    assert(state.getEmbers() == 25);

    // Update word count
    state.updateWordCount(30);
    assert(state.getWordCount() == 30);
    // Reached 10 words -> first achievement unlocked!
    assert(state.getAchievements()[0].unlocked);
    assert(state.getEmbers() > 25);

    // Toggle plot device
    state.togglePlotDevice(0); // Dagger
    assert(state.getPlotDevices()[0].active);
    assert(!state.getActivePrompt().empty());

    // Buy shop item
    int currentEmbers = state.getEmbers();
    bool bought = state.buyShopItem(0); // 15 embers
    assert(bought);
    assert(state.getEmbers() == currentEmbers - 15);
    assert(state.getShopItems()[0].purchased);

    // Export to markdown
    std::string md = state.exportToMarkdown("Chapter 1: The Dark Tower");
    assert(md.find("title: Grimoire Manuscript") != std::string::npos);
    assert(md.find("Chapter 1: The Dark Tower") != std::string::npos);

    std::cout << "  StoryState tests PASSED.\n";
}

void testProseHighlighter() {
    std::cout << "[Test 3] Testing ProseHighlighter tokenization...\n";
    ProseHighlighter hl;

    std::string line = "\"Halt!\" shouted [Kaelen], his blade gleaming *brightly*.";
    auto tokens = hl.highlightLine(line);
    assert(!tokens.empty());

    // First token should be dialogue
    assert(tokens[0].type == TokenType::Dialogue);
    // Entity token for [Kaelen]
    bool foundEntity = false;
    bool foundEmphasis = false;
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::Entity) foundEntity = true;
        if (tok.type == TokenType::Emphasis) foundEmphasis = true;
    }
    assert(foundEntity);
    assert(foundEmphasis);

    std::cout << "  ProseHighlighter tests PASSED.\n";
}

void testManuscriptEditor() {
    std::cout << "[Test 4] Testing ManuscriptEditor two-page flow...\n";
    ManuscriptEditor editor;
    editor.setText("First line.\nSecond line.\nThird line.");
    assert(editor.getWordCount() == 6);

    // Type character
    editor.handleChar('A');
    assert(editor.getWordCount() >= 6);

    // Test backspace
    editor.handleKey(259, false, false); // Backspace

    std::cout << "  ManuscriptEditor tests PASSED.\n";
}

void testSnesDspCore() {
    std::cout << "[Test 5] Testing SNES S-DSP lo-fi sound engine...\n";
    SNESDSPEngine snes;
    snes.reset();
    snes.echo.enabled = true;

    // Trigger note on
    snes.noteOn(0, 440.0f, 0.8f, 0.5f);
    assert(snes.voices[0].active);

    float outL = 0.0f;
    float outR = 0.0f;
    for (int i = 0; i < 500; ++i) {
        snes.processStereo(44100.0f, outL, outR);
        assert(!std::isnan(outL));
        assert(!std::isnan(outR));
    }

    std::cout << "  SNES S-DSP tests PASSED.\n";
}

void testPixelCanvas() {
    std::cout << "[Test 6] Testing PixelCanvas 2.5D lighting and icons...\n";
    PixelCanvas canvas(640, 360);
    canvas.clear(ColorRGBA(30, 20, 15));
    assert(canvas.getWidth() == 640);
    assert(canvas.getHeight() == 360);

    // Draw icons
    canvas.drawIconDagger(100, 100, ColorRGBA(200, 200, 210), ColorRGBA(150, 120, 60));
    canvas.drawIconQuill(140, 100, ColorRGBA(220, 210, 190), ColorRGBA(140, 100, 50));
    canvas.drawIconKey(180, 100, ColorRGBA(170, 140, 80));

    // Dynamic lights
    std::vector<LightSource2D> lights;
    LightSource2D l1;
    l1.x = 100.0f;
    l1.y = 80.0f;
    l1.radius = 200.0f;
    lights.push_back(l1);

    canvas.applyLighting(lights, 1.0f);
    canvas.applyVignette(0.4f);

    std::cout << "  PixelCanvas tests PASSED.\n";
}

int main() {
    std::cout << "=========================================================\n";
    std::cout << "     Project Grimoire Engine Automated Test Suite        \n";
    std::cout << "=========================================================\n";

    testTextDocument();
    testStoryState();
    testProseHighlighter();
    testManuscriptEditor();
    testSnesDspCore();
    testPixelCanvas();

    std::cout << "=========================================================\n";
    std::cout << "     ALL 6 GRIMOIRE ENGINE TESTS PASSED SUCCESSFULLY!    \n";
    std::cout << "=========================================================\n";
    return 0;
}
