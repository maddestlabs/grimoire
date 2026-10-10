cmake_minimum_required(VERSION 3.20)
project(GrimoireEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Options
option(GRIMOIRE_BUILD_TESTS "Build Grimoire automated test suite" ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sokol
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/miniaudio
)

if(MSVC)
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        SOKOL_D3D11
    )
    # Enable multi-processor compilation
    add_compile_options(/MP /W3 /utf-8)
elseif(EMSCRIPTEN)
    add_compile_definitions(SOKOL_GLES3)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sUSE_WEBGL2=1 -sALLOW_MEMORY_GROWTH=1")
else()
    add_compile_definitions(SOKOL_GLCORE)
endif()

# Core static library
add_library(grimoire_core STATIC
    src/core/story_state.cpp
    src/audio/audio_engine.cpp
    src/render/pixel_canvas.cpp
    src/render/pixel_font.cpp
    src/render/monospace_font_8x16.cpp
    src/render/scene_renderer.cpp
    src/editor/manuscript_editor.cpp
    src/editor/prose_highlighter.cpp
    src/ui/diegetic_ui.cpp
)

target_include_directories(grimoire_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Native desktop app
add_executable(grimoire
    src/main.cpp
)

target_link_libraries(grimoire PRIVATE grimoire_core)

if(WIN32)
    target_link_libraries(grimoire PRIVATE
        d3d11
        dxgi
        d3dcompiler
        user32
        gdi32
        shell32
        ole32
    )
endif()

if(GRIMOIRE_BUILD_TESTS)
    enable_testing()
    add_executable(test_grimoire
        tests/test_grimoire.cpp
    )
    target_link_libraries(test_grimoire PRIVATE grimoire_core)
    add_test(NAME GrimoireCoreTests COMMAND test_grimoire)
endif()
