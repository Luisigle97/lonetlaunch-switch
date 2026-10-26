#include "failsafe.hpp"
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>

namespace Core {

bool Failsafe::CheckFailsafeRequested(PadState* pad) {
    if (!pad) return false;
    padUpdate(pad);
    u64 keysDown = padGetButtons(pad);

    // If both shoulder buttons (L + R) or PLUS are held during boot, trigger failsafe
    if ((keysDown & HidNpadButton_L) && (keysDown & HidNpadButton_R)) {
        return true;
    }
    if (keysDown & HidNpadButton_Minus) {
        return true;
    }

    return false;
}

bool Failsafe::WasPreviousSessionCrashed() {
    struct stat st;
    if (stat(LOCK_FILE_PATH, &st) == 0) {
        // Lock file exists: previous session did not exit cleanly
        return true;
    }
    return false;
}

void Failsafe::MarkSessionStart() {
    FILE* f = fopen(LOCK_FILE_PATH, "w");
    if (f) {
        fputs("RUNNING", f);
        fclose(f);
    }
}

void Failsafe::MarkSessionCleanExit() {
    unlink(LOCK_FILE_PATH);
}

void Failsafe::ResetCrashFlag() {
    unlink(LOCK_FILE_PATH);
}

} // namespace Core
