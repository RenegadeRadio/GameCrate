#include "gamecrate/RegistryScanner.hpp"

#include "gamecrate/FootprintScanner.hpp"

#include <algorithm>
#include <sstream>
#include <unordered_map>

namespace gamecrate {

namespace {

constexpr wchar_t kHivePrefix[] = L"HKCU\\";

void AppendValueEntries(HKEY key, const std::wstring& keyPath, RegistrySnapshot& snapshot) {
    DWORD valueCount = 0;
    DWORD maxValueName = 0;
    DWORD maxValueData = 0;
    if (RegQueryInfoKeyW(
            key,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            &valueCount,
            &maxValueName,
            &maxValueData,
            nullptr,
            nullptr) != ERROR_SUCCESS) {
        return;
    }

    std::wstring valueName(maxValueName + 1, L'\0');
    std::vector<uint8_t> data(maxValueData + sizeof(wchar_t));

    for (DWORD index = 0; index < valueCount; ++index) {
        DWORD nameLength = maxValueName + 1;
        DWORD dataLength = static_cast<DWORD>(data.size());
        DWORD type = 0;
        const LONG result = RegEnumValueW(
            key,
            index,
            valueName.data(),
            &nameLength,
            nullptr,
            &type,
            data.data(),
            &dataLength);
        if (result != ERROR_SUCCESS) {
            continue;
        }

        RegistryEntry entry;
        entry.keyPath = keyPath;
        entry.valueName.assign(valueName.c_str(), nameLength);
        entry.valueType = type;
        entry.data.assign(data.begin(), data.begin() + dataLength);
        snapshot.entries.push_back(std::move(entry));
    }
}

void CaptureKeyRecursive(
    HKEY root,
    const std::wstring& relativePath,
    int depth,
    int maxDepth,
    RegistrySnapshot& snapshot) {
    HKEY key = nullptr;
    const LONG openResult =
        relativePath.empty() ? RegOpenKeyExW(root, nullptr, 0, KEY_READ, &key)
                             : RegOpenKeyExW(root, relativePath.c_str(), 0, KEY_READ, &key);
    if (openResult != ERROR_SUCCESS) {
        return;
    }

    const std::wstring fullPath = relativePath.empty() ? std::wstring(kHivePrefix)
                                                       : kHivePrefix + relativePath;
    AppendValueEntries(key, fullPath, snapshot);

    if (depth >= maxDepth) {
        RegCloseKey(key);
        return;
    }

    DWORD subKeyCount = 0;
    DWORD maxSubKeyLength = 0;
    if (RegQueryInfoKeyW(
            key,
            nullptr,
            nullptr,
            nullptr,
            &subKeyCount,
            &maxSubKeyLength,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr) != ERROR_SUCCESS) {
        RegCloseKey(key);
        return;
    }

    std::wstring subKeyName(maxSubKeyLength + 1, L'\0');
    for (DWORD index = 0; index < subKeyCount; ++index) {
        DWORD nameLength = maxSubKeyLength + 1;
        if (RegEnumKeyExW(
                key,
                index,
                subKeyName.data(),
                &nameLength,
                nullptr,
                nullptr,
                nullptr,
                nullptr) != ERROR_SUCCESS) {
            continue;
        }

        std::wstring childRelative = relativePath.empty()
            ? std::wstring(subKeyName.c_str(), nameLength)
            : relativePath + L"\\" + std::wstring(subKeyName.c_str(), nameLength);
        CaptureKeyRecursive(root, childRelative, depth + 1, maxDepth, snapshot);
    }

    RegCloseKey(key);
}

std::wstring EntryKey(const RegistryEntry& entry) {
    return entry.keyPath + L"|" + entry.valueName + L"|" + std::to_wstring(entry.valueType);
}

bool DataEqual(const RegistryEntry& left, const RegistryEntry& right) {
    return left.data == right.data;
}

}  // namespace

RegistrySnapshot RegistryScanner::CaptureWatchKeys() {
    RegistrySnapshot snapshot;

    static const wchar_t* watchRoots[] = {
        L"Software",
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        L"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run",
        L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run",
    };

    for (const wchar_t* watchRoot : watchRoots) {
        const int maxDepth = (watchRoot == watchRoots[0]) ? 2 : 0;
        CaptureKeyRecursive(HKEY_CURRENT_USER, watchRoot, 0, maxDepth, snapshot);
    }

    std::sort(snapshot.entries.begin(), snapshot.entries.end(), [](const RegistryEntry& a, const RegistryEntry& b) {
        return EntryKey(a) < EntryKey(b);
    });

    return snapshot;
}

RegistryDiff RegistryScanner::Compare(const RegistrySnapshot& before, const RegistrySnapshot& after) {
    RegistryDiff diff;

    std::unordered_map<std::wstring, RegistryEntry> beforeMap;
    beforeMap.reserve(before.entries.size());
    for (const auto& entry : before.entries) {
        beforeMap.emplace(EntryKey(entry), entry);
    }

    std::unordered_map<std::wstring, RegistryEntry> afterMap;
    afterMap.reserve(after.entries.size());
    for (const auto& entry : after.entries) {
        afterMap.emplace(EntryKey(entry), entry);
    }

    for (const auto& [key, afterEntry] : afterMap) {
        const auto it = beforeMap.find(key);
        if (it == beforeMap.end()) {
            diff.added.push_back(afterEntry);
        } else if (!DataEqual(it->second, afterEntry)) {
            diff.modified.push_back(afterEntry);
        }
    }

    for (const auto& [key, beforeEntry] : beforeMap) {
        if (afterMap.find(key) == afterMap.end()) {
            diff.removed.push_back(beforeEntry);
        }
    }

    return diff;
}

bool RegistryScanner::IsSuspiciousKey(const std::wstring& keyPath) {
    const std::wstring lower = PathUtils::ToLower(keyPath);
    static const wchar_t* markers[] = {
        L"\\run",
        L"\\runonce",
        L"\\startupapproved\\run",
        L"\\shell\\open\\command",
        L"\\inifilemapping",
        L"\\image file execution options",
    };

    for (const wchar_t* marker : markers) {
        if (lower.find(marker) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

std::wstring RegistryScanner::FormatEntry(const RegistryEntry& entry) {
    if (entry.valueName.empty()) {
        return entry.keyPath;
    }
    return entry.keyPath + L" [" + entry.valueName + L"]";
}

}  // namespace gamecrate
