#include "status.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>

namespace Sys {

bool SystemStatus::Initialize() {
    Result rc = psmInitialize();
    s_psmInitialized = R_SUCCEEDED(rc);

    rc = timeInitialize();
    s_timeInitialized = R_SUCCEEDED(rc);

    return true;
}

void SystemStatus::Exit() {
    if (s_psmInitialized) {
        psmExit();
        s_psmInitialized = false;
    }
    if (s_timeInitialized) {
        timeExit();
        s_timeInitialized = false;
    }
}

int SystemStatus::GetBatteryPercentage() {
    if (!s_psmInitialized) return 100;
    u32 pct = 100;
    Result rc = psmGetBatteryChargePercentage(&pct);
    if (R_SUCCEEDED(rc)) {
        return static_cast<int>(pct);
    }
    return 100;
}

bool SystemStatus::IsBatteryCharging() {
    if (!s_psmInitialized) return false;
    PsmChargerType type = PsmChargerType_Unconnected;
    Result rc = psmGetChargerType(&type);
    if (R_SUCCEEDED(rc)) {
        return (type != PsmChargerType_Unconnected);
    }
    return false;
}

std::string SystemStatus::GetFormattedTime() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    if (!localTime) return "12:00";

    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d", localTime->tm_hour, localTime->tm_min);
    return std::string(buf);
}

bool SystemStatus::IsDocked() {
    AppletOperationMode mode = appletGetOperationMode();
    return (mode == AppletOperationMode_Console);
}

} // namespace Sys
