#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <memory>
#include <switch.h>

namespace Gfx {

struct Color {
    Uint8 r{255}, g{255}, b{255}, a{255};

    static constexpr Color White() { return {255, 255, 255, 255}; }
    static constexpr Color Black() { return {0, 0, 0, 255}; }
    static constexpr Color DarkBg() { return {18, 20, 26, 255}; }
    static constexpr Color CardBg() { return {28, 32, 42, 240}; }
    static constexpr Color Accent() { return {0, 210, 255, 255}; } // Cyan neon glow
    static constexpr Color AccentPurple() { return {168, 85, 247, 255}; }
    static constexpr Color TextMuted() { return {160, 168, 185, 255}; }
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Initialize(int logicalW = 1920, int logicalH = 1080);
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void Clear(Color color = Color::DarkBg());
    void DrawGradientBackground(Color topColor, Color bottomColor);

    void FillRect(const SDL_Rect& rect, Color color);
    void DrawRect(const SDL_Rect& rect, Color color);
    void FillRoundedRect(const SDL_Rect& rect, int radius, Color color);
    void DrawRoundedRect(const SDL_Rect& rect, int radius, Color color, int thickness = 2);

    void DrawTexture(SDL_Texture* texture, const SDL_Rect& dest, Uint8 alpha = 255);
    void DrawTextureScaled(SDL_Texture* texture, int x, int y, int w, int h, Uint8 alpha = 255);

    void DrawText(const std::string& text, int x, int y, int fontSize, Color color, bool center = false);
    void GetTextSize(const std::string& text, int fontSize, int* outW, int* outH);

    SDL_Renderer* GetNativeRenderer() const { return m_renderer; }

private:
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;

    PlFontData m_sharedFontData{};
    bool m_hasSharedFont = false;

    TTF_Font* m_fontSmall = nullptr;
    TTF_Font* m_fontMedium = nullptr;
    TTF_Font* m_fontLarge = nullptr;

    bool LoadSystemFont();
};

} // namespace Gfx
