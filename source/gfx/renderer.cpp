#include "renderer.hpp"
#include <SDL_image.h>
#include <cmath>
#include <algorithm>

namespace Gfx {

Renderer::Renderer() = default;

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Initialize(int logicalW, int logicalH) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        return false;
    }

    if (TTF_Init() < 0) {
        return false;
    }

    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_WEBP;
    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
        // Continue even if some image formats aren't present
    }

    // Initialize Switch joysticks
    for (int i = 0; i < 2; i++) {
        SDL_JoystickOpen(i);
    }

    m_window = SDL_CreateWindow(
        "LonetLaunch",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1920, 1080,
        SDL_WINDOW_SHOWN
    );
    if (!m_window) {
        return false;
    }

    m_renderer = SDL_CreateRenderer(
        m_window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!m_renderer) {
        return false;
    }

    // Set logical size for perfect resolution scaling between 720p and 1080p
    SDL_RenderSetLogicalSize(m_renderer, logicalW, logicalH);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    LoadSystemFont();
    return true;
}

bool Renderer::LoadSystemFont() {
    Result rc = plInitialize(PlServiceType_User);
    if (R_SUCCEEDED(rc)) {
        rc = plGetSharedFontByType(&m_sharedFontData, PlSharedFontType_Standard);
        if (R_SUCCEEDED(rc) && m_sharedFontData.address && m_sharedFontData.size > 0) {
            m_hasSharedFont = true;
            // 0 freesource = do not free the switch kernel memory block
            m_fontSmall  = TTF_OpenFontRW(SDL_RWFromMem(m_sharedFontData.address, m_sharedFontData.size), 0, 22);
            m_fontMedium = TTF_OpenFontRW(SDL_RWFromMem(m_sharedFontData.address, m_sharedFontData.size), 0, 32);
            m_fontLarge  = TTF_OpenFontRW(SDL_RWFromMem(m_sharedFontData.address, m_sharedFontData.size), 0, 48);
            return true;
        }
    }
    return false;
}

void Renderer::Shutdown() {
    if (m_fontSmall)  { TTF_CloseFont(m_fontSmall);  m_fontSmall = nullptr; }
    if (m_fontMedium) { TTF_CloseFont(m_fontMedium); m_fontMedium = nullptr; }
    if (m_fontLarge)  { TTF_CloseFont(m_fontLarge);  m_fontLarge = nullptr; }

    if (m_hasSharedFont) {
        plExit();
        m_hasSharedFont = false;
    }

    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

void Renderer::BeginFrame() {
    Clear(Color::DarkBg());
}

void Renderer::EndFrame() {
    SDL_RenderPresent(m_renderer);
}

void Renderer::Clear(Color color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(m_renderer);
}

void Renderer::DrawGradientBackground(Color topColor, Color bottomColor) {
    int w = 1920, h = 1080;
    for (int y = 0; y < h; y += 4) {
        float t = static_cast<float>(y) / static_cast<float>(h);
        Uint8 r = static_cast<Uint8>(topColor.r + t * (bottomColor.r - topColor.r));
        Uint8 g = static_cast<Uint8>(topColor.g + t * (bottomColor.g - topColor.g));
        Uint8 b = static_cast<Uint8>(topColor.b + t * (bottomColor.b - topColor.b));
        SDL_SetRenderDrawColor(m_renderer, r, g, b, 255);
        SDL_Rect line = {0, y, w, 4};
        SDL_RenderFillRect(m_renderer, &line);
    }
}

void Renderer::FillRect(const SDL_Rect& rect, Color color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(m_renderer, &rect);
}

void Renderer::DrawRect(const SDL_Rect& rect, Color color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(m_renderer, &rect);
}

void Renderer::FillRoundedRect(const SDL_Rect& rect, int radius, Color color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    
    // Middle core
    SDL_Rect inner = {rect.x + radius, rect.y, rect.w - 2 * radius, rect.h};
    SDL_RenderFillRect(m_renderer, &inner);

    // Left and right bars
    SDL_Rect left = {rect.x, rect.y + radius, radius, rect.h - 2 * radius};
    SDL_Rect right = {rect.x + rect.w - radius, rect.y + radius, radius, rect.h - 2 * radius};
    SDL_RenderFillRect(m_renderer, &left);
    SDL_RenderFillRect(m_renderer, &right);

    // 4 Corner discs
    for (int dy = 0; dy <= radius; dy++) {
        int dx = static_cast<int>(std::sqrt(radius * radius - dy * dy));
        // Top-left
        SDL_RenderDrawLine(m_renderer, rect.x + radius - dx, rect.y + radius - dy, rect.x + radius, rect.y + radius - dy);
        // Top-right
        SDL_RenderDrawLine(m_renderer, rect.x + rect.w - radius, rect.y + radius - dy, rect.x + rect.w - radius + dx, rect.y + radius - dy);
        // Bottom-left
        SDL_RenderDrawLine(m_renderer, rect.x + radius - dx, rect.y + rect.h - radius + dy, rect.x + radius, rect.y + rect.h - radius + dy);
        // Bottom-right
        SDL_RenderDrawLine(m_renderer, rect.x + rect.w - radius, rect.y + rect.h - radius + dy, rect.x + rect.w - radius + dx, rect.y + rect.h - radius + dy);
    }
}

void Renderer::DrawRoundedRect(const SDL_Rect& rect, int radius, Color color, int thickness) {
    for (int t = 0; t < thickness; t++) {
        SDL_Rect r = {rect.x - t, rect.y - t, rect.w + 2 * t, rect.h + 2 * t};
        SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
        
        // Lines
        SDL_RenderDrawLine(m_renderer, r.x + radius, r.y, r.x + r.w - radius, r.y);
        SDL_RenderDrawLine(m_renderer, r.x + radius, r.y + r.h, r.x + r.w - radius, r.y + r.h);
        SDL_RenderDrawLine(m_renderer, r.x, r.y + radius, r.x, r.y + r.h - radius);
        SDL_RenderDrawLine(m_renderer, r.x + r.w, r.y + radius, r.x + r.w, r.y + r.h - radius);
    }
}

void Renderer::DrawTexture(SDL_Texture* texture, const SDL_Rect& dest, Uint8 alpha) {
    if (!texture) return;
    SDL_SetTextureAlphaMod(texture, alpha);
    SDL_RenderCopy(m_renderer, texture, nullptr, &dest);
}

void Renderer::DrawTextureScaled(SDL_Texture* texture, int x, int y, int w, int h, Uint8 alpha) {
    SDL_Rect dest = {x, y, w, h};
    DrawTexture(texture, dest, alpha);
}

void Renderer::DrawText(const std::string& text, int x, int y, int fontSize, Color color, bool center) {
    if (text.empty()) return;

    TTF_Font* font = m_fontMedium;
    if (fontSize <= 24) font = m_fontSmall;
    else if (fontSize >= 40) font = m_fontLarge;

    if (!font) return;

    SDL_Color sdlColor = {color.r, color.g, color.b, color.a};
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), sdlColor);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (texture) {
        SDL_Rect dest = {x, y, surface->w, surface->h};
        if (center) {
            dest.x -= surface->w / 2;
            dest.y -= surface->h / 2;
        }
        SDL_RenderCopy(m_renderer, texture, nullptr, &dest);
        SDL_DestroyTexture(texture);
    }

    SDL_FreeSurface(surface);
}

void Renderer::GetTextSize(const std::string& text, int fontSize, int* outW, int* outH) {
    TTF_Font* font = m_fontMedium;
    if (fontSize <= 24) font = m_fontSmall;
    else if (fontSize >= 40) font = m_fontLarge;

    if (!font) {
        if (outW) *outW = 0;
        if (outH) *outH = 0;
        return;
    }
    TTF_SizeUTF8(font, text.c_str(), outW, outH);
}

} // namespace Gfx
