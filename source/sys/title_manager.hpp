#pragma once

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <SDL.h>
#include <switch.h>

namespace Sys {

struct TitleItem {
    u64 titleId = 0;
    std::string name = "Cargando...";
    std::string author = "";
    std::string version = "1.0.0";
    
    SDL_Texture* iconTexture = nullptr;
    std::vector<u8> iconJpegBuffer;
    std::atomic<bool> metadataReady{false};
    bool textureUploaded = false;
};

class TitleManager {
public:
    TitleManager();
    ~TitleManager();

    bool Initialize();
    void Shutdown();

    void ScanInstalledTitles();
    void ProcessPendingTextures(SDL_Renderer* renderer);

    size_t GetTitleCount() const;
    TitleItem* GetTitle(size_t index);

    bool LaunchTitle(u64 titleId);

private:
    std::vector<std::unique_ptr<TitleItem>> m_titles;
    mutable std::mutex m_titlesMutex;

    std::thread m_workerThread;
    std::atomic<bool> m_running{false};
    std::condition_variable m_cv;

    bool m_nsInitialized = false;

    void AsyncWorker();
};

} // namespace Sys
