#pragma once

#include <string>
#include <switch.h>

namespace Core {

class Failsafe {
public:
    static bool CheckFailsafeRequested(PadState* pad);
    static void MarkSessionStart();
    static void MarkSessionCleanExit();
    static bool WasPreviousSessionCrashed();
    static void ResetCrashFlag();

private:
    static constexpr const char* LOCK_FILE_PATH = "sdmc:/switch/lonetlaunch_lock.tmp";
};

} // namespace Core
