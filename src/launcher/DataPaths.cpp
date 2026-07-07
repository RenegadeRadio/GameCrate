#include "gamecrate/DataPaths.hpp"

#include <ShlObj.h>

namespace gamecrate {

namespace {

std::wstring LocalAppDataRoot() {
    wchar_t localAppData[MAX_PATH] = {};
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, localAppData))) {
        return std::wstring(localAppData) + L"\\GameCrate";
    }
    return L"C:\\Users\\Public\\GameCrate";
}

}  // namespace

std::wstring DataPaths::Root() {
    return LocalAppDataRoot();
}

std::wstring DataPaths::ProfilesRoot() {
    return Root() + L"\\profiles";
}

std::wstring DataPaths::ProfileDataRoot(const std::wstring& profileId) {
    return Root() + L"\\" + profileId;
}

std::wstring DataPaths::DefaultSaveDir(const std::wstring& profileId) {
    return ProfileDataRoot(profileId) + L"\\saves";
}

}  // namespace gamecrate
