#pragma once

#include "gfx/renderer.hpp"
#include "sys/title_manager.hpp"
#include <switch.h>

namespace Core {

class Application {
public:
    Application();
    ~Application();

    bool Initialize();
    void Run();
    void Shutdown();

private:
    Gfx::Renderer m_renderer;
    Sys::TitleManager m_titleManager;
    PadState m_pad{};

    bool m_running = false;
    int m_selectedIndex = 0;
    float m_scrollX = 0.0f;
    float m_targetScrollX = 0.0f;

    Uint32 m_lastFrameTime = 0;
    float m_pulseAnim = 0.0f;

    void HandleInput();
    void Update(float dt);
    void Render();

    void RenderTopBar();
    void RenderCarousel();
    void RenderSelectedInfo();
    void RenderFooter();
};

} // namespace Core
