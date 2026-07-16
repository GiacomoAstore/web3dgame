#include "engine.hpp"

#include "../renderer/renderer.hpp"
#include "../../game/game.hpp"

#include <iostream>
#include <chrono>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

// Global engine instance
Engine* g_engine = nullptr;

bool Engine::Init(uint32_t windowWidth, uint32_t windowHeight) {
    std::cout << "[Engine] Initializing... (requested: " << windowWidth << "x" << windowHeight << ")" << std::endl;

    // Create and initialize renderer
    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->Init(windowWidth, windowHeight)) {
        std::cerr << "[Engine] Renderer initialization failed" << std::endl;
        return false;
    }
    std::cout << "[Engine] Renderer initialized" << std::endl;

    // Create and initialize game
    m_game = std::make_unique<Game>();
    if (!m_game->Init(m_renderer.get())) {
        std::cerr << "[Engine] Game initialization failed" << std::endl;
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
        m_game.reset();
    }

    if (m_renderer) {
        m_renderer->Shutdown();
        m_renderer.reset();
    }

    std::cout << "[Engine] Shutdown complete" << std::endl;
}

void Engine::Tick() {
    if (!m_isRunning) {
        return;
    }

    // Simple frame timing (in a real engine, use a proper timer)
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    m_deltaTime = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;

    // Cap delta time to avoid jumps (e.g., if tab loses focus)
    if (m_deltaTime > 0.05f) {
        m_deltaTime = 0.05f;  // 50ms cap
    }

    m_currentTime += m_deltaTime;

    // Update game logic
    m_game->Update(m_deltaTime);

    // Render frame
    m_renderer->BeginFrame();
    m_game->Render(m_renderer.get());
    m_renderer->EndFrame();
}
