#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace gamecrate {

struct RegistryEntry {
    std::wstring keyPath;
    std::wstring valueName;
    uint32_t valueType = 0;
    std::vector<uint8_t> data;
};

struct RegistrySnapshot {
    std::vector<RegistryEntry> entries;
};

struct RegistryDiff {
    std::vector<RegistryEntry> added;
    std::vector<RegistryEntry> modified;
    std::vector<RegistryEntry> removed;
};

class RegistryScanner {
public:
    static RegistrySnapshot CaptureWatchKeys();
    static RegistryDiff Compare(const RegistrySnapshot& before, const RegistrySnapshot& after);
    static bool IsSuspiciousKey(const std::wstring& keyPath);
    static std::wstring FormatEntry(const RegistryEntry& entry);
};

}  // namespace gamecrate
