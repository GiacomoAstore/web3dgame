#include <iostream>
#include <memory>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "engine/core/engine.hpp"

// Global engine instance
extern Engine* g_engine;

// Emscripten callback: called every animation frame
void MainLoop() {
    if (g_engine && g_engine->IsRunning()) {
        g_engine->Tick();
    } else if (g_engine) {
        // Request to exit
        #ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
        #endif
        if (g_engine) {
            g_engine->Shutdown();
            delete g_engine;
            g_engine = nullptr;
        }
        std::cout << "[main] Engine loop ended" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    std::cout << "[main] Starting Racing 3D Multiplayer..." << std::endl;

    // Create and initialize engine
    g_engine = new Engine();
    if (!g_engine->Init(800, 600)) {
        std::cerr << "[main] Engine initialization failed" << std::endl;
        delete g_engine;
        g_engine = nullptr;
        return 1;
    }

    std::cout << "[main] Engine ready, starting main loop" << std::endl;

#ifdef __EMSCRIPTEN__
    // Emscripten: set up requestAnimationFrame loop (60fps by default)
    emscripten_set_main_loop(MainLoop, 0, 1);
#else
    // Desktop: simple loop
    while (g_engine && g_engine->IsRunning()) {
        MainLoop();
    }
    if (g_engine) {
        g_engine->Shutdown();
        delete g_engine;
        g_engine = nullptr;
    }
#endif

    std::cout << "[main] Clean exit" << std::endl;
    return 0;
}
