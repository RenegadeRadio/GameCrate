#pragma once

#include <Windows.h>

#include <cstdint>
#include <string>
#include <vector>

namespace windoze {

struct FileEntry {
    std::wstring path;
    uint64_t size = 0;
    uint64_t lastWriteUtc = 0;
};

struct FootprintSnapshot {
    std::vector<FileEntry> files;
};

struct FootprintDiff {
    std::vector<FileEntry> added;
    std::vector<FileEntry> modified;
    std::vector<FileEntry> removed;
};

class PathUtils {
public:
    static std::wstring Normalize(const std::wstring& path);
    static bool IsUnderRoot(const std::wstring& path, const std::wstring& root);
    static bool IsUnderAnyRoot(const std::wstring& path, const std::vector<std::wstring>& roots);
    static std::wstring ToLower(const std::wstring& value);
};

class FootprintScanner {
public:
    static FootprintSnapshot ScanRoot(const std::wstring& root, int maxDepth = -1);
    static FootprintSnapshot ScanRoots(const std::vector<std::wstring>& roots, int maxDepth = -1);
    static FootprintDiff Compare(const FootprintSnapshot& before, const FootprintSnapshot& after);
    static std::vector<std::wstring> SensitiveWatchPaths();
    static std::vector<std::wstring> AllowedInstallRoots(
        const std::wstring& profileId,
        const std::wstring& installDir,
        const std::wstring& saveDir);
    static bool IsSuspiciousOutsidePath(const std::wstring& path);
};

}  // namespace windoze
