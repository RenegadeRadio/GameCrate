#include "gamecrate/DataPaths.hpp"

#include <ShlObj.h>
#include <Windows.h>

#include <iterator>

namespace gamecrate {

namespace {

std::wstring TrimTrailingSeparators(std::wstring path) {
    while (!path.empty() && (path.back() == L'\\' || path.back() == L'/')) {
        path.pop_back();
    }
    return path;
}

std::wstring LocalAppDataRoot() {
    wchar_t localAppData[MAX_PATH] = {};
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, localAppData))) {
        return std::wstring(localAppData) + L"\\GameCrate";
    }
    return L"C:\\Users\\Public\\GameCrate";
}

std::wstring DataRootFromEnvironment() {
    wchar_t buffer[32768] = {};
    const DWORD length = GetEnvironmentVariableW(L"GAMECRATE_DATA_ROOT", buffer, static_cast<DWORD>(std::size(buffer)));
    if (length == 0 || length >= std::size(buffer)) {
        return L"";
    }
    return TrimTrailingSeparators(buffer);
}

}  // namespace

std::wstring DataPaths::Root() {
    const std::wstring overrideRoot = DataRootFromEnvironment();
    if (!overrideRoot.empty()) {
        return overrideRoot;
    }
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
