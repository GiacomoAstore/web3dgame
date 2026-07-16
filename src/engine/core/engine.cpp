#include "engine.hpp"

#include "../renderer/renderer.hpp"
#include "../../game/game.hpp"

#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

// Global engine instance
Engine* g_engine = nullptr;

bool Engine::Init(unsigned int windowWidth, unsigned int windowHeight) {
    std::cout << "[Engine] Initializing... (requested: " << windowWidth << "x" << windowHeight << ")" << std::endl;

    // Create and initialize renderer
    m_renderer = new Renderer();
    if (!m_renderer->Init(windowWidth, windowHeight)) {
        std::cerr << "[Engine] Renderer initialization failed" << std::endl;
        delete m_renderer;
        m_renderer = nullptr;
        return false;
    }
    std::cout << "[Engine] Renderer initialized" << std::endl;

    // Create and initialize game
    m_game = new Game();
    if (!m_game->Init(m_renderer)) {
        std::cerr << "[Engine] Game initialization failed" << std::endl;
        delete m_game;
        m_game = nullptr;
        return false;
    }
    std::cout << "[Engine] Game initialized" << std::endl;

    m_isRunning = true;
    m_lastFrameTime = 0.0f;
    m_currentTime = 0.0f;

    std::cout << "[Engine] Startup complete" << std::endl;
    return true;
}

void Engine::Shutdown() {
    std::cout << "[Engine] Shutting down..." << std::endl;

    if (m_game) {
        m_game->Shutdown();
        delete m_game;
        m_game = nullptr;
    }

    if (m_renderer) {
        m_renderer->Shutdown();
        delete m_renderer;
        m_renderer = nullptr;
    }

    std::cout << "[Engine] Shutdown complete" << std::endl;
}

void Engine::Tick() {
    if (!m_isRunning) {
        return;
    }

#ifdef __EMSCRIPTEN__
    // Emscripten: timing via emscripten_get_now() (in milliseconds)
    static double lastTime = 0.0;
    double now = emscripten_get_now();
    if (lastTime == 0.0) {
        lastTime = now;  // First call
    }
    m_deltaTime = (float)((now - lastTime) / 1000.0);  // Convert to seconds
    lastTime = now;
#else
    // Desktop fallback: simple fixed timestep
    m_deltaTime = 0.016f;  // ~60 fps
#endif

    // Cap delta time to avoid jumps (e.g., if tab loses focus)
    if (m_deltaTime > 0.05f) {
        m_deltaTime = 0.05f;  // 50ms cap
    }

    m_currentTime += m_deltaTime;

    // Update game logic
    m_game->Update(m_deltaTime);

    // Render frame
    m_renderer->BeginFrame();
    m_game->Render(m_renderer);
    m_renderer->EndFrame();
}
