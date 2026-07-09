#pragma once

#include <string>

namespace gamecrate {

class DataPaths {
public:
    // %LOCALAPPDATA%\GameCrate by default, or GAMECRATE_DATA_ROOT when set (dev isolation).
    static std::wstring Root();
    static std::wstring ProfilesRoot();
    static std::wstring ProfileDataRoot(const std::wstring& profileId);
    static std::wstring DefaultSaveDir(const std::wstring& profileId);
};

}  // namespace gamecrate
