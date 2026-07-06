#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace gamecrate {

struct VirtualStorageLayout {
    std::wstring root;
    std::wstring appDataRoaming;
    std::wstring appDataLocal;
    std::wstring temp;
};

class VirtualStorage {
public:
    static std::wstring ProfileRoot(const std::wstring& profileId);
    static VirtualStorageLayout Ensure(const std::wstring& profileId);
    static std::vector<std::wstring> WritableRoots(const std::wstring& profileId);
    static std::vector<std::pair<std::wstring, std::wstring>> EnvironmentOverrides(
        const VirtualStorageLayout& layout);
    static std::unique_ptr<wchar_t[]> BuildEnvironmentBlock(
        const std::vector<std::pair<std::wstring, std::wstring>>& overrides);
};

}  // namespace gamecrate
