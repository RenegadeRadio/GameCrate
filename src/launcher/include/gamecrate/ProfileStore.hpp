#pragma once

#include <string>
#include <vector>

namespace gamecrate {

struct SandboxProfile {
    std::wstring id;
    std::wstring name;
    std::wstring moniker;
    std::wstring installDir;
    std::wstring executable;
    std::wstring arguments;
    std::wstring saveDir;
    std::vector<std::wstring> writablePaths;
    std::vector<std::wstring> readablePaths;
    bool network = false;
    bool registryRead = true;
    bool lpacCom = false;
    bool gpu = true;
};

class ProfileStore {
public:
    static std::wstring ProfilesRoot();
    static std::wstring ProfilePath(const std::wstring& id);
    static bool Save(const SandboxProfile& profile);
    static bool Load(const std::wstring& id, SandboxProfile& out);
    static bool Exists(const std::wstring& id);
    static std::vector<std::wstring> CapabilitiesFor(const SandboxProfile& profile);
    static std::vector<std::wstring> ListProfiles();
};

}  // namespace gamecrate
