#include "core/app.hpp"
#include "core/failsafe.hpp"
#include <switch.h>

int main(int argc, char* argv[]) {
    // 1. Initial Pad Setup for Failsafe Emergency Detection
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);

    // 2. Failsafe check (Zero-risk guarantee)
    // If user holds L+R or MINUS during boot, bypass launcher immediately
    if (Core::Failsafe::CheckFailsafeRequested(&pad)) {
        return 0; // Graceful hand-off to official / fallback menu
    }

    // 3. Mark session start for crash monitoring
    Core::Failsafe::MarkSessionStart();

    // 4. Initialize and Run Application
    {
        Core::Application app;
        if (app.Initialize()) {
            app.Run();
        }
        app.Shutdown();
    }

    // 5. Clean exit confirmed: remove lockfile
    Core::Failsafe::MarkSessionCleanExit();

    return 0;
}
