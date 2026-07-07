#include "gamecrate/FootprintScanner.hpp"
#include "gamecrate/DataPaths.hpp"
#include "gamecrate/VirtualStorage.hpp"

#include <ShlObj.h>

#include <algorithm>
#include <filesystem>
#include <unordered_map>

namespace gamecrate {

namespace {

std::wstring KnownFolderPath(int csidl) {
    wchar_t buffer[MAX_PATH] = {};
    if (FAILED(SHGetFolderPathW(nullptr, csidl, nullptr, SHGFP_TYPE_CURRENT, buffer))) {
        return L"";
    }
    return buffer;
}

void ScanDirectoryRecursive(
    const std::filesystem::path& root,
    int maxDepth,
    int depth,
    FootprintSnapshot& snapshot) {
    if (maxDepth >= 0 && depth > maxDepth) {
        return;
    }

    std::error_code ec;
    if (!std::filesystem::exists(root, ec)) {
        return;
    }

    std::filesystem::directory_options options =
        std::filesystem::directory_options::skip_permission_denied;

    std::filesystem::recursive_directory_iterator it(root, options, ec);
    const std::filesystem::recursive_directory_iterator end;
    for (; it != end; it.increment(ec)) {
        if (ec) {
            break;
        }

        if (maxDepth >= 0 && it.depth() > maxDepth) {
            it.disable_recursion_pending();
            continue;
        }

        const auto& entry = *it;
        if (!entry.is_regular_file(ec)) {
            continue;
        }

        FileEntry fileEntry;
        fileEntry.path = PathUtils::Normalize(entry.path().wstring());
        fileEntry.size = entry.file_size(ec);

        WIN32_FILE_ATTRIBUTE_DATA attributes{};
        if (GetFileAttributesExW(fileEntry.path.c_str(), GetFileExInfoStandard, &attributes)) {
            ULARGE_INTEGER writeTime{};
            writeTime.LowPart = attributes.ftLastWriteTime.dwLowDateTime;
            writeTime.HighPart = attributes.ftLastWriteTime.dwHighDateTime;
            fileEntry.lastWriteUtc = writeTime.QuadPart;
        }

        snapshot.files.push_back(std::move(fileEntry));
    }
}

}  // namespace

std::wstring PathUtils::ToLower(const std::wstring& value) {
    std::wstring lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](wchar_t ch) {
        if (ch >= L'A' && ch <= L'Z') {
            return static_cast<wchar_t>(ch - L'A' + L'a');
        }
        return ch;
    });
    return lower;
}

std::wstring PathUtils::Normalize(const std::wstring& path) {
    if (path.empty()) {
        return L"";
    }

    wchar_t fullPath[MAX_PATH] = {};
    const DWORD length = GetFullPathNameW(path.c_str(), MAX_PATH, fullPath, nullptr);
    if (length == 0 || length >= MAX_PATH) {
        return ToLower(path);
    }

    std::wstring normalized = fullPath;
    while (!normalized.empty() && (normalized.back() == L'\\' || normalized.back() == L'/')) {
        normalized.pop_back();
    }
    return ToLower(normalized);
}

bool PathUtils::IsUnderRoot(const std::wstring& path, const std::wstring& root) {
    const std::wstring normalizedPath = Normalize(path);
    const std::wstring normalizedRoot = Normalize(root);
    if (normalizedRoot.empty() || normalizedPath.empty()) {
        return false;
    }
    if (normalizedPath == normalizedRoot) {
        return true;
    }
    return normalizedPath.rfind(normalizedRoot + L"\\", 0) == 0;
}

bool PathUtils::IsUnderAnyRoot(const std::wstring& path, const std::vector<std::wstring>& roots) {
    for (const auto& root : roots) {
        if (IsUnderRoot(path, root)) {
            return true;
        }
    }
    return false;
}

FootprintSnapshot FootprintScanner::ScanRoot(const std::wstring& root, int maxDepth) {
    FootprintSnapshot snapshot;
    if (root.empty()) {
        return snapshot;
    }
    ScanDirectoryRecursive(std::filesystem::path(root), maxDepth, 0, snapshot);
    return snapshot;
}

FootprintSnapshot FootprintScanner::ScanRoots(const std::vector<std::wstring>& roots, int maxDepth) {
    FootprintSnapshot combined;
    for (const auto& root : roots) {
        const FootprintSnapshot part = ScanRoot(root, maxDepth);
        combined.files.insert(combined.files.end(), part.files.begin(), part.files.end());
    }
    return combined;
}

FootprintDiff FootprintScanner::Compare(const FootprintSnapshot& before, const FootprintSnapshot& after) {
    FootprintDiff diff;

    std::unordered_map<std::wstring, FileEntry> beforeMap;
    beforeMap.reserve(before.files.size());
    for (const auto& file : before.files) {
        beforeMap[file.path] = file;
    }

    std::unordered_map<std::wstring, FileEntry> afterMap;
    afterMap.reserve(after.files.size());
    for (const auto& file : after.files) {
        afterMap[file.path] = file;
    }

    for (const auto& [path, afterEntry] : afterMap) {
        const auto it = beforeMap.find(path);
        if (it == beforeMap.end()) {
            diff.added.push_back(afterEntry);
        } else if (it->second.size != afterEntry.size || it->second.lastWriteUtc != afterEntry.lastWriteUtc) {
            diff.modified.push_back(afterEntry);
        }
    }

    for (const auto& [path, beforeEntry] : beforeMap) {
        if (afterMap.find(path) == afterMap.end()) {
            diff.removed.push_back(beforeEntry);
        }
    }

    std::sort(diff.added.begin(), diff.added.end(), [](const FileEntry& a, const FileEntry& b) {
        return a.path < b.path;
    });
    std::sort(diff.modified.begin(), diff.modified.end(), [](const FileEntry& a, const FileEntry& b) {
        return a.path < b.path;
    });
    std::sort(diff.removed.begin(), diff.removed.end(), [](const FileEntry& a, const FileEntry& b) {
        return a.path < b.path;
    });

    return diff;
}

std::vector<std::wstring> FootprintScanner::SensitiveWatchPaths() {
    std::vector<std::wstring> paths;
    const int folders[] = {
        CSIDL_STARTUP,
        CSIDL_COMMON_STARTUP,
        CSIDL_APPDATA,
        CSIDL_LOCAL_APPDATA,
        CSIDL_DESKTOPDIRECTORY,
        CSIDL_MYDOCUMENTS,
        CSIDL_PROGRAMS,
        CSIDL_COMMON_PROGRAMS,
    };

    for (int csidl : folders) {
        const std::wstring path = KnownFolderPath(csidl);
        if (!path.empty()) {
            paths.push_back(path);
        }
    }

    const std::wstring programData = KnownFolderPath(CSIDL_COMMON_APPDATA);
    if (!programData.empty()) {
        paths.push_back(programData);
    }

    return paths;
}

std::vector<std::wstring> FootprintScanner::AllowedInstallRoots(
    const std::wstring& profileId,
    const std::wstring& installDir,
    const std::wstring& saveDir) {
    std::vector<std::wstring> roots = {installDir, saveDir};

    roots.push_back(DataPaths::ProfileDataRoot(profileId));

    for (const auto& virtualRoot : VirtualStorage::WritableRoots(profileId)) {
        roots.push_back(virtualRoot);
    }

    std::vector<std::wstring> normalized;
    normalized.reserve(roots.size());
    for (const auto& root : roots) {
        if (!root.empty()) {
            normalized.push_back(PathUtils::Normalize(root));
        }
    }
    return normalized;
}

bool FootprintScanner::IsSuspiciousOutsidePath(const std::wstring& path) {
    const std::wstring lower = PathUtils::ToLower(path);
    static const wchar_t* markers[] = {
        L"\\startup\\",
        L"\\start menu\\",
        L"\\microsoft\\windows\\start menu\\",
        L"\\tasks\\",
        L"\\system32\\",
        L"\\syswow64\\",
        L"\\windows\\temp\\",
    };

    for (const wchar_t* marker : markers) {
        if (lower.find(marker) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

}  // namespace gamecrate
