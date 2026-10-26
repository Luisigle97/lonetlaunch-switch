#include "title_manager.hpp"
#include <SDL_image.h>
#include <algorithm>
#include <cstring>

namespace Sys {

TitleManager::TitleManager() = default;

TitleManager::~TitleManager() {
    Shutdown();
}

bool TitleManager::Initialize() {
    Result rc = nsInitialize();
    m_nsInitialized = R_SUCCEEDED(rc);

    m_running = true;
    m_workerThread = std::thread(&TitleManager::AsyncWorker, this);

    ScanInstalledTitles();
    return true;
}

void TitleManager::Shutdown() {
    m_running = false;
    m_cv.notify_all();

    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }

    {
        std::lock_guard<std::mutex> lock(m_titlesMutex);
        for (auto& item : m_titles) {
            if (item->iconTexture) {
                SDL_DestroyTexture(item->iconTexture);
                item->iconTexture = nullptr;
            }
        }
        m_titles.clear();
    }

    if (m_nsInitialized) {
        nsExit();
        m_nsInitialized = false;
    }
}

void TitleManager::ScanInstalledTitles() {
    if (!m_nsInitialized) return;

    constexpr s32 MAX_RECORDS = 128;
    std::vector<NsApplicationRecord> records(MAX_RECORDS);
    s32 outEntryCount = 0;

    Result rc = nsListApplicationRecord(records.data(), MAX_RECORDS, 0, &outEntryCount);
    if (R_FAILED(rc) || outEntryCount <= 0) {
        // Fallback / mock item if no installed games found (e.g. testing in Hbmenu or emulator)
        auto dummy = std::make_unique<TitleItem>();
        dummy->titleId = 0x010000000000100D;
        dummy->name = "Nintendo Switch Homebrew";
        dummy->author = "Switchbrew Community";
        dummy->version = "v1.0";
        dummy->metadataReady = true;

        std::lock_guard<std::mutex> lock(m_titlesMutex);
        m_titles.push_back(std::move(dummy));
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_titlesMutex);
        m_titles.clear();

        for (s32 i = 0; i < outEntryCount; ++i) {
            auto item = std::make_unique<TitleItem>();
            item->titleId = records[i].application_id;
            item->name = "Cargando...";
            item->metadataReady = false;
            m_titles.push_back(std::move(item));
        }
    }

    m_cv.notify_one();
}

void TitleManager::AsyncWorker() {
    // Background worker thread for non-blocking disk & IPC reads
    size_t currentIndex = 0;

    while (m_running) {
        TitleItem* currentItem = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_titlesMutex);
            if (currentIndex < m_titles.size()) {
                currentItem = m_titles[currentIndex].get();
            }
        }

        if (currentItem && !currentItem->metadataReady) {
            // Buffer for ControlData
            std::unique_ptr<NsApplicationControlData> buf = std::make_unique<NsApplicationControlData>();
            u64 actualSize = 0;

            Result rc = nsGetApplicationControlData(
                NsApplicationControlSource_Storage,
                currentItem->titleId,
                buf.get(),
                sizeof(NsApplicationControlData),
                &actualSize
            );

            if (R_SUCCEEDED(rc) && actualSize >= sizeof(buf->nacp)) {
                NacpLanguageEntry* langEntry = nullptr;
                rc = nacpGetLanguageEntry(&buf->nacp, &langEntry);
                if (R_SUCCEEDED(rc) && langEntry && langEntry->name[0] != '\0') {
                    char safeName[0x201] = {0};
                    std::strncpy(safeName, langEntry->name, sizeof(safeName) - 1);
                    currentItem->name = safeName;

                    char safeAuthor[0x101] = {0};
                    std::strncpy(safeAuthor, langEntry->author, sizeof(safeAuthor) - 1);
                    currentItem->author = safeAuthor;
                }

                if (buf->nacp.display_version[0] != '\0') {
                    char safeVer[0x11] = {0};
                    std::strncpy(safeVer, buf->nacp.display_version, sizeof(safeVer) - 1);
                    currentItem->version = safeVer;
                }

                u64 iconSize = actualSize - sizeof(buf->nacp);
                if (iconSize > 0) {
                    currentItem->iconJpegBuffer.assign(buf->icon, buf->icon + iconSize);
                }
            } else {
                currentItem->name = "Título #" + std::to_string(currentItem->titleId);
            }

            currentItem->metadataReady = true;
            currentIndex++;
        } else {
            // Wait for new work or exit
            std::unique_lock<std::mutex> lock(m_titlesMutex);
            m_cv.wait_for(lock, std::chrono::milliseconds(50), [this]() {
                return !m_running;
            });
        }
    }
}

void TitleManager::ProcessPendingTextures(SDL_Renderer* renderer) {
    if (!renderer) return;

    std::lock_guard<std::mutex> lock(m_titlesMutex);
    for (auto& item : m_titles) {
        if (item->metadataReady && !item->textureUploaded && !item->iconJpegBuffer.empty()) {
            SDL_RWops* rw = SDL_RWFromMem(item->iconJpegBuffer.data(), static_cast<int>(item->iconJpegBuffer.size()));
            if (rw) {
                SDL_Surface* surface = IMG_Load_RW(rw, 1);
                if (surface) {
                    item->iconTexture = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);
                }
            }
            // Free CPU buffer memory once uploaded to GPU
            item->iconJpegBuffer.clear();
            item->iconJpegBuffer.shrink_to_fit();
            item->textureUploaded = true;
        }
    }
}

size_t TitleManager::GetTitleCount() const {
    std::lock_guard<std::mutex> lock(m_titlesMutex);
    return m_titles.size();
}

TitleItem* TitleManager::GetTitle(size_t index) {
    std::lock_guard<std::mutex> lock(m_titlesMutex);
    if (index < m_titles.size()) {
        return m_titles[index].get();
    }
    return nullptr;
}

bool TitleManager::LaunchTitle(u64 titleId) {
    // Attempt standard application launch request
    Result rc = appletRequestLaunchApplication(titleId, nullptr);
    if (R_SUCCEEDED(rc)) {
        return true;
    }

    // If running in SystemApplet context (qlaunch replacement mode)
    AppletApplication app;
    rc = appletCreateApplication(&app, titleId);
    if (R_SUCCEEDED(rc)) {
        rc = appletApplicationStart(&app);
        return R_SUCCEEDED(rc);
    }

    return false;
}

} // namespace Sys
