#pragma once

#include "gamecrate/FootprintScanner.hpp"
#include "gamecrate/ProfileStore.hpp"

#include <memory>
#include <string>
#include <vector>

namespace gamecrate {

enum class AclMode {
    Install,
    Run,
};

struct InstallOptions {
    std::wstring id;
    std::wstring name;
    std::wstring installDir;
    std::wstring installer;
    std::wstring installerArgs;
    std::wstring executable;
    std::wstring saveDir;
    bool network = false;
    bool registryRead = true;
    bool lpacCom = false;
    bool gpu = true;
    bool virtualizeAppData = true;
    bool failOnOutsideWrites = true;
    bool tightenAclsAfter = true;
};

struct InstallResult {
    bool success = false;
    DWORD installerExitCode = 0;
    std::wstring executable;
    std::vector<std::wstring> installedFiles;
    std::vector<std::wstring> outsideWrites;
    std::vector<std::wstring> suspiciousOutsideWrites;
    std::vector<std::wstring> registryChanges;
    std::vector<std::wstring> outsideRegistryChanges;
    std::vector<std::wstring> suspiciousRegistryChanges;
    std::wstring reportPath;
    std::wstring message;
};

class InstallManager {
public:
    static void ApplyVirtualStorage(SandboxProfile& profile);
    static std::unique_ptr<wchar_t[]> BuildEnvironmentForProfile(const SandboxProfile& profile);
    static bool ApplyProfileAcls(
        const SandboxProfile& profile,
        AclMode mode,
        std::wstring* errorOut = nullptr);
    static bool RemoveProfileAcls(const SandboxProfile& profile);
    static InstallResult Run(const InstallOptions& options);
    static std::wstring DetectExecutable(const std::wstring& installDir);
    static bool WriteReport(const InstallResult& result, const InstallOptions& options);
};

}  // namespace gamecrate
