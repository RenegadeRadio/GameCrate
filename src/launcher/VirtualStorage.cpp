#include "gamecrate/DataPaths.hpp"
#include "gamecrate/VirtualStorage.hpp"

#include <filesystem>
#include <unordered_map>

namespace gamecrate {

namespace {

bool EnsureDirectory(const std::wstring& path) {
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    return std::filesystem::exists(path, ec);
}

}  // namespace

std::wstring VirtualStorage::ProfileRoot(const std::wstring& profileId) {
    return DataPaths::ProfileDataRoot(profileId) + L"\\virtual";
}

VirtualStorageLayout VirtualStorage::Ensure(const std::wstring& profileId) {
    VirtualStorageLayout layout;
    layout.root = ProfileRoot(profileId);
    layout.appDataRoaming = layout.root + L"\\AppData\\Roaming";
    layout.appDataLocal = layout.root + L"\\AppData\\Local";
    layout.temp = layout.root + L"\\Temp";

    EnsureDirectory(layout.appDataRoaming);
    EnsureDirectory(layout.appDataLocal);
    EnsureDirectory(layout.temp);

    return layout;
}

std::vector<std::wstring> VirtualStorage::WritableRoots(const std::wstring& profileId) {
    const VirtualStorageLayout layout = Ensure(profileId);
    return {layout.root, layout.appDataRoaming, layout.appDataLocal, layout.temp};
}

std::vector<std::pair<std::wstring, std::wstring>> VirtualStorage::EnvironmentOverrides(
    const VirtualStorageLayout& layout) {
    return {
        {L"APPDATA", layout.appDataRoaming},
        {L"LOCALAPPDATA", layout.appDataLocal},
        {L"TEMP", layout.temp},
        {L"TMP", layout.temp},
    };
}

std::unique_ptr<wchar_t[]> VirtualStorage::BuildEnvironmentBlock(
    const std::vector<std::pair<std::wstring, std::wstring>>& overrides) {
    std::unordered_map<std::wstring, std::wstring> values;

    wchar_t* environment = GetEnvironmentStringsW();
    if (environment) {
        for (wchar_t* cursor = environment; *cursor != L'\0';) {
            const std::wstring entry = cursor;
            const size_t separator = entry.find(L'=');
            if (separator != std::wstring::npos) {
                values.emplace(entry.substr(0, separator), entry.substr(separator + 1));
            }
            cursor += entry.size() + 1;
        }
        FreeEnvironmentStringsW(environment);
    }

    for (const auto& overrideEntry : overrides) {
        values[overrideEntry.first] = overrideEntry.second;
    }

    size_t totalCharacters = 1;
    for (const auto& entry : values) {
        totalCharacters += entry.first.size() + 1 + entry.second.size() + 1;
    }

    auto block = std::make_unique<wchar_t[]>(totalCharacters);
    wchar_t* cursor = block.get();
    for (const auto& entry : values) {
        const std::wstring line = entry.first + L'=' + entry.second;
        std::copy(line.begin(), line.end(), cursor);
        cursor[line.size()] = L'\0';
        cursor += line.size() + 1;
    }
    *cursor = L'\0';
    return block;
}

}  // namespace gamecrate
