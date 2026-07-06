#include "gamecrate/ProfileStore.hpp"

#include <ShlObj.h>
#include <UserEnv.h>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace gamecrate {

namespace {

std::wstring ReadValue(const std::wstring& json, const std::wstring& key) {
    const std::wstring pattern = L"\"" + key + L"\"";
    const size_t keyPos = json.find(pattern);
    if (keyPos == std::wstring::npos) {
        return L"";
    }

    const size_t colonPos = json.find(L':', keyPos + pattern.size());
    if (colonPos == std::wstring::npos) {
        return L"";
    }

    const size_t quoteStart = json.find(L'"', colonPos + 1);
    if (quoteStart == std::wstring::npos) {
        return L"";
    }

    const size_t quoteEnd = json.find(L'"', quoteStart + 1);
    if (quoteEnd == std::wstring::npos) {
        return L"";
    }

    return json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
}

bool ReadBool(const std::wstring& json, const std::wstring& key, bool defaultValue) {
    const std::wstring pattern = L"\"" + key + L"\"";
    const size_t keyPos = json.find(pattern);
    if (keyPos == std::wstring::npos) {
        return defaultValue;
    }

    const size_t colonPos = json.find(L':', keyPos + pattern.size());
    if (colonPos == std::wstring::npos) {
        return defaultValue;
    }

    const std::wstring tail = json.substr(colonPos + 1, 12);
    if (tail.find(L"true") != std::wstring::npos) {
        return true;
    }
    if (tail.find(L"false") != std::wstring::npos) {
        return false;
    }
    return defaultValue;
}

std::vector<std::wstring> ReadStringArray(const std::wstring& json, const std::wstring& key) {
    std::vector<std::wstring> values;
    const std::wstring pattern = L"\"" + key + L"\"";
    const size_t keyPos = json.find(pattern);
    if (keyPos == std::wstring::npos) {
        return values;
    }

    const size_t arrayStart = json.find(L'[', keyPos);
    const size_t arrayEnd = json.find(L']', arrayStart);
    if (arrayStart == std::wstring::npos || arrayEnd == std::wstring::npos) {
        return values;
    }

    size_t pos = arrayStart + 1;
    while (pos < arrayEnd) {
        const size_t quoteStart = json.find(L'"', pos);
        if (quoteStart == std::wstring::npos || quoteStart >= arrayEnd) {
            break;
        }
        const size_t quoteEnd = json.find(L'"', quoteStart + 1);
        if (quoteEnd == std::wstring::npos || quoteEnd > arrayEnd) {
            break;
        }
        values.push_back(json.substr(quoteStart + 1, quoteEnd - quoteStart - 1));
        pos = quoteEnd + 1;
    }

    return values;
}

std::wstring EscapeJson(const std::wstring& value) {
    std::wstring escaped;
    escaped.reserve(value.size());
    for (wchar_t ch : value) {
        if (ch == L'\\' || ch == L'"') {
            escaped.push_back(L'\\');
        }
        escaped.push_back(ch);
    }
    return escaped;
}

std::wstring WriteStringArray(const std::vector<std::wstring>& values) {
    std::wstringstream ss;
    ss << L"[";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            ss << L", ";
        }
        ss << L"\"" << EscapeJson(values[i]) << L"\"";
    }
    ss << L"]";
    return ss.str();
}

bool EnsureDirectory(const std::wstring& path) {
    return CreateDirectoryW(path.c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS;
}

}  // namespace

std::wstring ProfileStore::ProfilesRoot() {
    wchar_t programData[MAX_PATH] = {};
    if (FAILED(SHGetFolderPathW(nullptr, CSIDL_COMMON_APPDATA, nullptr, SHGFP_TYPE_CURRENT, programData))) {
        return L"C:\\ProgramData\\GameCrate\\profiles";
    }
    return std::wstring(programData) + L"\\GameCrate\\profiles";
}

std::wstring ProfileStore::ProfilePath(const std::wstring& id) {
    return ProfilesRoot() + L"\\" + id + L".json";
}

bool ProfileStore::Save(const SandboxProfile& profile) {
    const std::wstring root = ProfilesRoot();
    if (!EnsureDirectory(root.substr(0, root.find_last_of(L'\\')))) {
        return false;
    }
    if (!EnsureDirectory(root)) {
        return false;
    }

    std::wstringstream json;
    json << L"{\n";
    json << L"  \"id\": \"" << EscapeJson(profile.id) << L"\",\n";
    json << L"  \"name\": \"" << EscapeJson(profile.name) << L"\",\n";
    json << L"  \"moniker\": \"" << EscapeJson(profile.moniker) << L"\",\n";
    json << L"  \"installDir\": \"" << EscapeJson(profile.installDir) << L"\",\n";
    json << L"  \"executable\": \"" << EscapeJson(profile.executable) << L"\",\n";
    json << L"  \"arguments\": \"" << EscapeJson(profile.arguments) << L"\",\n";
    json << L"  \"saveDir\": \"" << EscapeJson(profile.saveDir) << L"\",\n";
    json << L"  \"writablePaths\": " << WriteStringArray(profile.writablePaths) << L",\n";
    json << L"  \"readablePaths\": " << WriteStringArray(profile.readablePaths) << L",\n";
    json << L"  \"network\": " << (profile.network ? L"true" : L"false") << L",\n";
    json << L"  \"registryRead\": " << (profile.registryRead ? L"true" : L"false") << L",\n";
    json << L"  \"lpacCom\": " << (profile.lpacCom ? L"true" : L"false") << L",\n";
    json << L"  \"gpu\": " << (profile.gpu ? L"true" : L"false") << L",\n";
    json << L"  \"virtualizeAppData\": " << (profile.virtualizeAppData ? L"true" : L"false") << L"\n";
    json << L"}\n";

    std::wofstream out(ProfilePath(profile.id));
    if (!out.is_open()) {
        return false;
    }
    out << json.str();
    return true;
}

bool ProfileStore::Load(const std::wstring& id, SandboxProfile& out) {
    std::wifstream in(ProfilePath(id));
    if (!in.is_open()) {
        return false;
    }

    std::wstringstream buffer;
    buffer << in.rdbuf();
    const std::wstring json = buffer.str();

    out.id = ReadValue(json, L"id");
    out.name = ReadValue(json, L"name");
    out.moniker = ReadValue(json, L"moniker");
    out.installDir = ReadValue(json, L"installDir");
    out.executable = ReadValue(json, L"executable");
    out.arguments = ReadValue(json, L"arguments");
    out.saveDir = ReadValue(json, L"saveDir");
    out.writablePaths = ReadStringArray(json, L"writablePaths");
    out.readablePaths = ReadStringArray(json, L"readablePaths");
    out.network = ReadBool(json, L"network", false);
    out.registryRead = ReadBool(json, L"registryRead", true);
    out.lpacCom = ReadBool(json, L"lpacCom", false);
    out.gpu = ReadBool(json, L"gpu", true);
    out.virtualizeAppData = ReadBool(json, L"virtualizeAppData", true);

    if (out.id.empty()) {
        out.id = id;
    }
    if (out.moniker.empty()) {
        out.moniker = L"GameCrate." + out.id;
    }

    return !out.executable.empty() || !out.installDir.empty();
}

bool ProfileStore::Exists(const std::wstring& id) {
    const DWORD attrs = GetFileAttributesW(ProfilePath(id).c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
}

std::vector<std::wstring> ProfileStore::CapabilitiesFor(const SandboxProfile& profile) {
    std::vector<std::wstring> capabilities;
    if (profile.registryRead) {
        capabilities.push_back(L"registryRead");
    }
    if (profile.network) {
        capabilities.push_back(L"internetClient");
        capabilities.push_back(L"privateNetworkClientServer");
    }
    if (profile.lpacCom) {
        capabilities.push_back(L"lpacCom");
    }
    if (profile.gpu) {
        capabilities.push_back(L"lpacPnpNotifications");
        capabilities.push_back(L"lpacMedia");
    }
    return capabilities;
}

std::vector<std::wstring> ProfileStore::ListProfiles() {
    std::vector<std::wstring> profiles;
    const std::wstring root = ProfilesRoot();
    const std::wstring pattern = root + L"\\*.json";
    WIN32_FIND_DATAW findData{};
    HANDLE findHandle = FindFirstFileW(pattern.c_str(), &findData);
    if (findHandle == INVALID_HANDLE_VALUE) {
        return profiles;
    }

    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::wstring fileName = findData.cFileName;
            const size_t dot = fileName.find_last_of(L'.');
            if (dot != std::wstring::npos) {
                profiles.push_back(fileName.substr(0, dot));
            }
        }
    } while (FindNextFileW(findHandle, &findData));

    FindClose(findHandle);
    return profiles;
}

bool ProfileStore::Destroy(const std::wstring& id, bool wipeData) {
    SandboxProfile profile;
    if (Load(id, profile)) {
        DeleteAppContainerProfile(profile.moniker.c_str());
    }

    const std::wstring profileFile = ProfilePath(id);
    std::error_code ec;
    std::filesystem::remove(profileFile, ec);

    if (wipeData) {
        wchar_t programData[MAX_PATH] = {};
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_COMMON_APPDATA, nullptr, SHGFP_TYPE_CURRENT, programData))) {
            const std::wstring dataRoot = std::wstring(programData) + L"\\GameCrate\\" + id;
            std::filesystem::remove_all(dataRoot, ec);
        }
    }

    return !Exists(id);
}

}  // namespace gamecrate
