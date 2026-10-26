#include "app.hpp"
#include "failsafe.hpp"
#include "sys/status.hpp"
#include <cmath>
#include <algorithm>

namespace Core {

Application::Application() = default;

Application::~Application() {
    Shutdown();
}

bool Application::Initialize() {
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&m_pad);

    if (!m_renderer.Initialize(1920, 1080)) {
        return false;
    }

    Sys::SystemStatus::Initialize();

    if (!m_titleManager.Initialize()) {
        return false;
    }

    m_running = true;
    m_lastFrameTime = SDL_GetTicks();
    return true;
}

void Application::Shutdown() {
    m_running = false;
    m_titleManager.Shutdown();
    Sys::SystemStatus::Exit();
    m_renderer.Shutdown();
}

void Application::Run() {
    while (m_running && appletMainLoop()) {
        Uint32 currentTime = SDL_GetTicks();
        float dt = (currentTime - m_lastFrameTime) / 1000.0f;
        m_lastFrameTime = currentTime;

        // Cap dt to prevent huge leaps if stalled
        if (dt > 0.1f) dt = 0.1f;

        HandleInput();
        Update(dt);
        Render();
    }
}

void Application::HandleInput() {
    padUpdate(&m_pad);
    u64 kDown = padGetButtonsDown(&m_pad);

    // SDL Event polling for touch screen / joystick events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            m_running = false;
        }
        else if (event.type == SDL_FINGERDOWN) {
            // Touch screen support: touch right/left to navigate
            float touchX = event.tfinger.x * 1920.0f;
            if (touchX > 1200) {
                size_t count = m_titleManager.GetTitleCount();
                if (count > 0 && m_selectedIndex < static_cast<int>(count) - 1) {
                    m_selectedIndex++;
                }
            } else if (touchX < 720) {
                if (m_selectedIndex > 0) {
                    m_selectedIndex--;
                }
            } else {
                // Tapped center: launch
                auto* selected = m_titleManager.GetTitle(m_selectedIndex);
                if (selected) {
                    m_titleManager.LaunchTitle(selected->titleId);
                }
            }
        }
    }

    size_t count = m_titleManager.GetTitleCount();

    // D-Pad and Left Stick Navigation
    if ((kDown & HidNpadButton_Right) || (kDown & HidNpadButton_StickLRight)) {
        if (count > 0 && m_selectedIndex < static_cast<int>(count) - 1) {
            m_selectedIndex++;
        }
    }
    if ((kDown & HidNpadButton_Left) || (kDown & HidNpadButton_StickLLeft)) {
        if (m_selectedIndex > 0) {
            m_selectedIndex--;
        }
    }

    // Launch title on (A)
    if (kDown & HidNpadButton_A) {
        auto* selected = m_titleManager.GetTitle(m_selectedIndex);
        if (selected) {
            m_titleManager.LaunchTitle(selected->titleId);
        }
    }

    // Refresh titles on (Y)
    if (kDown & HidNpadButton_Y) {
        m_titleManager.ScanInstalledTitles();
        m_selectedIndex = 0;
    }

    // Exit on (+)
    if (kDown & HidNpadButton_Plus) {
        m_running = false;
    }
}

void Application::Update(float dt) {
    // Smooth pulse animation for glow effect
    m_pulseAnim += dt * 3.0f;

    // Smooth lerp scrolling to the selected game card
    constexpr float CARD_STRIDE = 330.0f;
    m_targetScrollX = -static_cast<float>(m_selectedIndex) * CARD_STRIDE;
    
    // Smooth exponential decay lerp for fluid 60fps tracking
    m_scrollX += (m_targetScrollX - m_scrollX) * (1.0f - std::exp(-15.0f * dt));

    // Upload pending textures from background async thread
    m_titleManager.ProcessPendingTextures(m_renderer.GetNativeRenderer());
}

void Application::Render() {
    m_renderer.BeginFrame();

    // Subtle modern background gradient
    m_renderer.DrawGradientBackground({14, 17, 23, 255}, {7, 8, 12, 255});

    RenderTopBar();
    RenderCarousel();
    RenderSelectedInfo();
    RenderFooter();

    m_renderer.EndFrame();
}

void Application::RenderTopBar() {
    // Top bar background strip
    m_renderer.FillRect({0, 0, 1920, 90}, {18, 22, 31, 220});

    // Logo & branding badge
    m_renderer.FillRoundedRect({50, 20, 240, 50}, 12, {28, 35, 50, 255});
    m_renderer.DrawText("LONETLAUNCH", 170, 45, 24, Gfx::Color::Accent(), true);

    // Center Mode Indicator (Docked vs Handheld)
    bool isDocked = Sys::SystemStatus::IsDocked();
    std::string modeStr = isDocked ? "MODO TV (1080p)" : "MODO PORTATIL (720p)";
    m_renderer.FillRoundedRect({820, 22, 280, 46}, 10, {26, 31, 44, 255});
    m_renderer.DrawText(modeStr, 960, 45, 20, Gfx::Color::TextMuted(), true);

    // Right Side: Clock & Battery
    std::string timeStr = Sys::SystemStatus::GetFormattedTime();
    m_renderer.DrawText(timeStr, 1680, 45, 26, Gfx::Color::White(), true);

    int battery = Sys::SystemStatus::GetBatteryPercentage();
    bool charging = Sys::SystemStatus::IsBatteryCharging();
    std::string batStr = std::to_string(battery) + "%" + (charging ? " ⚡" : "");
    
    Gfx::Color batColor = charging ? Gfx::Color::Accent() : (battery < 20 ? Gfx::Color{239, 68, 68, 255} : Gfx::Color::White());
    m_renderer.FillRoundedRect({1770, 22, 110, 46}, 10, {26, 31, 44, 255});
    m_renderer.DrawText(batStr, 1825, 45, 20, batColor, true);
}

void Application::RenderCarousel() {
    size_t count = m_titleManager.GetTitleCount();
    if (count == 0) return;

    constexpr float CENTER_X = 1920.0f / 2.0f;
    constexpr float BASE_Y = 280.0f;
    constexpr float CARD_STRIDE = 330.0f;

    for (size_t i = 0; i < count; ++i) {
        float itemCenterX = CENTER_X + m_scrollX + (i * CARD_STRIDE);

        // Cull items outside screen bounds
        if (itemCenterX < -250.0f || itemCenterX > 2170.0f) {
            continue;
        }

        bool isFocused = (static_cast<int>(i) == m_selectedIndex);
        float cardSize = isFocused ? 320.0f : 256.0f;
        float cardY = BASE_Y - (isFocused ? 32.0f : 0.0f);
        float cardX = itemCenterX - (cardSize / 2.0f);

        SDL_Rect cardRect = {
            static_cast<int>(cardX),
            static_cast<int>(cardY),
            static_cast<int>(cardSize),
            static_cast<int>(cardSize)
        };

        // Draw shadow / base
        SDL_Rect shadowRect = {cardRect.x + 4, cardRect.y + 8, cardRect.w, cardRect.h};
        m_renderer.FillRoundedRect(shadowRect, 18, {0, 0, 0, 110});

        // Draw Card Background
        m_renderer.FillRoundedRect(cardRect, 18, Gfx::Color::CardBg());

        auto* title = m_titleManager.GetTitle(i);
        if (title && title->iconTexture) {
            // Draw game icon
            SDL_Rect iconRect = {cardRect.x + 10, cardRect.y + 10, cardRect.w - 20, cardRect.h - 20};
            m_renderer.DrawTexture(title->iconTexture, iconRect);
        } else {
            // Loading skeleton placeholder
            m_renderer.DrawText("🎮", cardRect.x + cardRect.w / 2, cardRect.y + cardRect.h / 2 - 20, 48, Gfx::Color::TextMuted(), true);
            m_renderer.DrawText("Cargando...", cardRect.x + cardRect.w / 2, cardRect.y + cardRect.h / 2 + 35, 18, Gfx::Color::TextMuted(), true);
        }

        // Focused glow outline with smooth pulsing
        if (isFocused) {
            Uint8 glowAlpha = static_cast<Uint8>(180 + 75 * std::sin(m_pulseAnim));
            Gfx::Color glowColor = Gfx::Color::Accent();
            glowColor.a = glowAlpha;
            m_renderer.DrawRoundedRect(cardRect, 18, glowColor, 4);
        }
    }
}

void Application::RenderSelectedInfo() {
    auto* selected = m_titleManager.GetTitle(m_selectedIndex);
    if (!selected) return;

    // Info card container
    SDL_Rect infoRect = {360, 680, 1200, 160};
    m_renderer.FillRoundedRect(infoRect, 20, {22, 26, 36, 230});
    m_renderer.DrawRoundedRect(infoRect, 20, {40, 48, 66, 255}, 1);

    // Title name
    m_renderer.DrawText(selected->name, 960, 725, 36, Gfx::Color::White(), true);

    // Author & Version
    std::string metaStr = selected->author;
    if (!selected->version.empty()) {
        if (!metaStr.empty()) metaStr += "  •  ";
        metaStr += "Versión " + selected->version;
    }
    m_renderer.DrawText(metaStr, 960, 775, 22, Gfx::Color::TextMuted(), true);

    // Title ID badge
    char idBuf[32];
    std::snprintf(idBuf, sizeof(idBuf), "TID: %016llX", static_cast<unsigned long long>(selected->titleId));
    m_renderer.DrawText(idBuf, 960, 810, 18, {100, 116, 139, 255}, true);
}

void Application::RenderFooter() {
    // Footer button hint strip
    m_renderer.FillRect({0, 990, 1920, 90}, {14, 17, 24, 240});

    auto renderPill = [this](int x, int y, const std::string& btn, const std::string& label) {
        m_renderer.FillRoundedRect({x, y, 42, 42}, 21, {45, 55, 75, 255});
        m_renderer.DrawText(btn, x + 21, y + 21, 20, Gfx::Color::White(), true);
        m_renderer.DrawText(label, x + 55, y + 21, 22, Gfx::Color::TextMuted(), false);
    };

    renderPill(450, 1015, "A", "Iniciar Juego");
    renderPill(850, 1015, "Y", "Recargar Catálogo");
    renderPill(1250, 1015, "+", "Cerrar");
}

} // namespace Core
