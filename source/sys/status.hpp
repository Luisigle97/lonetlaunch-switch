#pragma once

#include <string>
#include <switch.h>

namespace Sys {

class SystemStatus {
public:
    static bool Initialize();
    static void Exit();

    static int GetBatteryPercentage();
    static bool IsBatteryCharging();
    static std::string GetFormattedTime();
    static bool IsDocked();

private:
    static inline bool s_psmInitialized = false;
    static inline bool s_timeInitialized = false;
};

} // namespace Sys
