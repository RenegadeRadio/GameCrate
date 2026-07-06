#include "gamecrate/AclManager.hpp"
#include "gamecrate/AppContainerLauncher.hpp"
#include "gamecrate/InstallManager.hpp"
#include "gamecrate/ProfileStore.hpp"

#include <ShlObj.h>

#include <fstream>
#include <iostream>
#include <sstream>

namespace {

void PrintUsage() {
    std::wcerr
        << L"GameCrate — sandboxed game launcher for Windows\n\n"
        << L"Usage:\n"
        << L"  gamecrate install --id <id> --name <name> --install-dir <path> --installer <path>\n"
        << L"  gamecrate create-profile --id <id> --name <name> --install-dir <path> --executable <path>\n"
        << L"  gamecrate launch --profile <id> [--no-wait]\n"
        << L"  gamecrate grant --profile <id>\n"
        << L"  gamecrate show-install-report --profile <id>\n"
        << L"  gamecrate list-profiles\n"
        << L"  gamecrate show-profile --profile <id>\n\n"
        << L"Options for install:\n"
        << L"  --installer-args <args>  Arguments passed to the installer\n"
        << L"  --executable <path>      Skip auto-detection after install\n"
        << L"  --allow-outside-writes   Warn on outside writes but do not fail\n"
        << L"  --keep-install-writable  Do not tighten install dir ACLs after install\n"
        << L"  --network                Allow network during install (not recommended)\n"
        << L"  --no-registry / --lpac-com / --no-gpu\n\n"
        << L"Options for create-profile:\n"
        << L"  --save-dir <path>       Isolated save directory (default: %ProgramData%\\GameCrate\\<id>\\saves)\n"
        << L"  --network               Grant internet and LAN capabilities\n"
        << L"  --no-registry           Do not grant registryRead (stricter, breaks many games)\n"
        << L"  --lpac-com              Grant COM capability for launcher-heavy titles\n"
        << L"  --no-gpu                Disable GPU capabilities (lpacPnpNotifications, lpacMedia)\n";
}

std::wstring RequireArg(int argc, wchar_t** argv, int& index) {
    if (index + 1 >= argc) {
        throw std::runtime_error("missing argument value");
    }
    return argv[++index];
}

std::wstring DefaultSaveDir(const std::wstring& id) {
    wchar_t programData[MAX_PATH] = {};
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_COMMON_APPDATA, nullptr, SHGFP_TYPE_CURRENT, programData))) {
        return std::wstring(programData) + L"\\GameCrate\\" + id + L"\\saves";
    }
    return L"C:\\ProgramData\\GameCrate\\" + id + L"\\saves";
}

int CreateProfile(int argc, wchar_t** argv) {
    gamecrate::SandboxProfile profile;
    profile.registryRead = true;
    profile.gpu = true;

    for (int i = 2; i < argc; ++i) {
        const std::wstring arg = argv[i];
        if (arg == L"--id") {
            profile.id = RequireArg(argc, argv, i);
        } else if (arg == L"--name") {
            profile.name = RequireArg(argc, argv, i);
        } else if (arg == L"--install-dir") {
            profile.installDir = RequireArg(argc, argv, i);
        } else if (arg == L"--executable") {
            profile.executable = RequireArg(argc, argv, i);
        } else if (arg == L"--save-dir") {
            profile.saveDir = RequireArg(argc, argv, i);
        } else if (arg == L"--network") {
            profile.network = true;
        } else if (arg == L"--no-registry") {
            profile.registryRead = false;
        } else if (arg == L"--lpac-com") {
            profile.lpacCom = true;
        } else if (arg == L"--no-gpu") {
            profile.gpu = false;
        }
    }

    if (profile.id.empty() || profile.name.empty() || profile.installDir.empty()) {
        std::wcerr << L"create-profile requires --id, --name, and --install-dir.\n";
        return 1;
    }

    if (profile.moniker.empty()) {
        profile.moniker = L"GameCrate." + profile.id;
    }
    if (profile.saveDir.empty()) {
        profile.saveDir = DefaultSaveDir(profile.id);
    }
    if (profile.executable.empty()) {
        profile.executable = profile.installDir;
    }

    profile.writablePaths = {profile.installDir, profile.saveDir};

    if (!gamecrate::ProfileStore::Save(profile)) {
        std::wcerr << L"Failed to save profile.\n";
        return 1;
    }

    std::vector<SID_AND_ATTRIBUTES> capabilityAttributes;
    std::vector<gamecrate::CapabilitySid> ownedCapabilities;
    for (const auto& capabilityName : gamecrate::ProfileStore::CapabilitiesFor(profile)) {
        gamecrate::CapabilitySid capability;
        if (gamecrate::AppContainerLauncher::ResolveCapability(capabilityName, capability)) {
            capabilityAttributes.push_back({capability.sid, capability.attributes});
            ownedCapabilities.push_back(std::move(capability));
        }
    }

    PSID packageSid = nullptr;
    if (!gamecrate::AppContainerLauncher::CreateOrResolveProfile(
            profile.moniker, profile.name, capabilityAttributes, &packageSid)) {
        std::wcerr << L"Failed to register AppContainer profile.\n";
        return 1;
    }
    if (packageSid) {
        FreeSid(packageSid);
    }

    if (!gamecrate::InstallManager::ApplyProfileAcls(profile, gamecrate::AclMode::Run)) {
        std::wcerr << L"Profile saved but ACL grants failed. Re-run 'gamecrate grant --profile "
                   << profile.id << L"'.\n";
        return 1;
    }

    std::wcout << L"Created profile '" << profile.id << L"' with moniker " << profile.moniker << L"\n";
    return 0;
}

int InstallProfile(int argc, wchar_t** argv) {
    gamecrate::InstallOptions options;
    options.registryRead = true;
    options.gpu = true;
    options.network = false;

    for (int i = 2; i < argc; ++i) {
        const std::wstring arg = argv[i];
        if (arg == L"--id") {
            options.id = RequireArg(argc, argv, i);
        } else if (arg == L"--name") {
            options.name = RequireArg(argc, argv, i);
        } else if (arg == L"--install-dir") {
            options.installDir = RequireArg(argc, argv, i);
        } else if (arg == L"--installer") {
            options.installer = RequireArg(argc, argv, i);
        } else if (arg == L"--installer-args") {
            options.installerArgs = RequireArg(argc, argv, i);
        } else if (arg == L"--executable") {
            options.executable = RequireArg(argc, argv, i);
        } else if (arg == L"--save-dir") {
            options.saveDir = RequireArg(argc, argv, i);
        } else if (arg == L"--network") {
            options.network = true;
        } else if (arg == L"--no-registry") {
            options.registryRead = false;
        } else if (arg == L"--lpac-com") {
            options.lpacCom = true;
        } else if (arg == L"--no-gpu") {
            options.gpu = false;
        } else if (arg == L"--allow-outside-writes") {
            options.failOnOutsideWrites = false;
        } else if (arg == L"--keep-install-writable") {
            options.tightenAclsAfter = false;
        }
    }

    const gamecrate::InstallResult result = gamecrate::InstallManager::Run(options);

    std::wcout << L"Installer exit code: " << result.installerExitCode << L"\n";
    std::wcout << L"Installed files: " << result.installedFiles.size() << L"\n";
    std::wcout << L"Outside writes: " << result.outsideWrites.size() << L"\n";

    if (!result.executable.empty()) {
        std::wcout << L"Executable: " << result.executable << L"\n";
    }
    if (!result.reportPath.empty()) {
        std::wcout << L"Report: " << result.reportPath << L"\n";
    }

    if (!result.outsideWrites.empty()) {
        std::wcerr << L"\nFiles written outside sandbox:\n";
        for (const auto& path : result.outsideWrites) {
            std::wcerr << L"  " << path << L"\n";
        }
    }

    if (!result.suspiciousOutsideWrites.empty()) {
        std::wcerr << L"\nSuspicious outside writes:\n";
        for (const auto& path : result.suspiciousOutsideWrites) {
            std::wcerr << L"  " << path << L"\n";
        }
    }

    if (!result.message.empty()) {
        if (result.success) {
            std::wcout << result.message << L"\n";
        } else {
            std::wcerr << result.message << L"\n";
        }
    }

    return result.success ? 0 : 1;
}

int LaunchProfile(int argc, wchar_t** argv) {
    std::wstring profileId;
    bool waitForExit = true;

    for (int i = 2; i < argc; ++i) {
        const std::wstring arg = argv[i];
        if (arg == L"--profile") {
            profileId = RequireArg(argc, argv, i);
        } else if (arg == L"--no-wait") {
            waitForExit = false;
        }
    }

    if (profileId.empty()) {
        std::wcerr << L"launch requires --profile <id>.\n";
        return 1;
    }

    gamecrate::SandboxProfile profile;
    if (!gamecrate::ProfileStore::Load(profileId, profile)) {
        std::wcerr << L"Profile not found: " << profileId << L"\n";
        return 1;
    }

    if (!gamecrate::InstallManager::ApplyProfileAcls(profile, gamecrate::AclMode::Run)) {
        std::wcerr << L"Warning: ACL grants may be incomplete.\n";
    }

    gamecrate::LaunchOptions options;
    options.moniker = profile.moniker;
    options.displayName = profile.name;
    options.executable = profile.executable;
    options.arguments = profile.arguments;
    options.capabilities = gamecrate::ProfileStore::CapabilitiesFor(profile);
    options.lessPrivileged = true;
    options.waitForExit = waitForExit;
    options.retainProfile = true;
    options.workingDirectory = profile.installDir;

    const gamecrate::LaunchResult result = gamecrate::AppContainerLauncher::Launch(options);
    if (!result.success) {
        std::wcerr << result.message << L"\n";
        return static_cast<int>(result.errorCode);
    }

    std::wcout << L"Launched PID " << result.processId;
    if (waitForExit) {
        std::wcout << L" (exit code " << result.exitCode << L")";
    }
    std::wcout << L"\n";
    return static_cast<int>(result.exitCode);
}

int GrantProfile(int argc, wchar_t** argv) {
    std::wstring profileId;
    for (int i = 2; i < argc; ++i) {
        if (std::wstring(argv[i]) == L"--profile") {
            profileId = RequireArg(argc, argv, i);
        }
    }

    if (profileId.empty()) {
        std::wcerr << L"grant requires --profile <id>.\n";
        return 1;
    }

    gamecrate::SandboxProfile profile;
    if (!gamecrate::ProfileStore::Load(profileId, profile)) {
        std::wcerr << L"Profile not found: " << profileId << L"\n";
        return 1;
    }

    if (!gamecrate::InstallManager::ApplyProfileAcls(profile, gamecrate::AclMode::Run)) {
        std::wcerr << L"Failed to apply ACL grants.\n";
        return 1;
    }

    std::wcout << L"ACL grants applied for profile '" << profileId << L"'.\n";
    return 0;
}

int ShowInstallReport(int argc, wchar_t** argv) {
    std::wstring profileId;
    for (int i = 2; i < argc; ++i) {
        if (std::wstring(argv[i]) == L"--profile") {
            profileId = RequireArg(argc, argv, i);
        }
    }

    if (profileId.empty()) {
        std::wcerr << L"show-install-report requires --profile <id>.\n";
        return 1;
    }

    wchar_t programData[MAX_PATH] = {};
    if (FAILED(SHGetFolderPathW(nullptr, CSIDL_COMMON_APPDATA, nullptr, SHGFP_TYPE_CURRENT, programData))) {
        std::wcerr << L"Failed to resolve ProgramData path.\n";
        return 1;
    }

    const std::wstring reportPath =
        std::wstring(programData) + L"\\GameCrate\\" + profileId + L"\\install-report.json";

    std::wifstream in(reportPath);
    if (!in.is_open()) {
        std::wcerr << L"Install report not found: " << reportPath << L"\n";
        return 1;
    }

    std::wstringstream buffer;
    buffer << in.rdbuf();
    std::wcout << buffer.str();
    return 0;
}

int ListProfiles() {
    const auto profiles = gamecrate::ProfileStore::ListProfiles();
    if (profiles.empty()) {
        std::wcout << L"No profiles found.\n";
        return 0;
    }

    for (const auto& id : profiles) {
        gamecrate::SandboxProfile profile;
        if (gamecrate::ProfileStore::Load(id, profile)) {
            std::wcout << id << L" — " << profile.name << L" (" << profile.installDir << L")\n";
        } else {
            std::wcout << id << L"\n";
        }
    }
    return 0;
}

int ShowProfile(int argc, wchar_t** argv) {
    std::wstring profileId;
    for (int i = 2; i < argc; ++i) {
        if (std::wstring(argv[i]) == L"--profile") {
            profileId = RequireArg(argc, argv, i);
        }
    }

    if (profileId.empty()) {
        std::wcerr << L"show-profile requires --profile <id>.\n";
        return 1;
    }

    gamecrate::SandboxProfile profile;
    if (!gamecrate::ProfileStore::Load(profileId, profile)) {
        std::wcerr << L"Profile not found: " << profileId << L"\n";
        return 1;
    }

    std::wcout << L"id: " << profile.id << L"\n"
               << L"name: " << profile.name << L"\n"
               << L"moniker: " << profile.moniker << L"\n"
               << L"installDir: " << profile.installDir << L"\n"
               << L"executable: " << profile.executable << L"\n"
               << L"saveDir: " << profile.saveDir << L"\n"
               << L"network: " << (profile.network ? L"true" : L"false") << L"\n"
               << L"registryRead: " << (profile.registryRead ? L"true" : L"false") << L"\n"
               << L"gpu: " << (profile.gpu ? L"true" : L"false") << L"\n"
               << L"lpacCom: " << (profile.lpacCom ? L"true" : L"false") << L"\n";
    return 0;
}

}  // namespace

int wmain(int argc, wchar_t** argv) {
    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    const std::wstring command = argv[1];
    try {
        if (command == L"install") {
            return InstallProfile(argc, argv);
        }
        if (command == L"create-profile") {
            return CreateProfile(argc, argv);
        }
        if (command == L"launch") {
            return LaunchProfile(argc, argv);
        }
        if (command == L"grant") {
            return GrantProfile(argc, argv);
        }
        if (command == L"show-install-report") {
            return ShowInstallReport(argc, argv);
        }
        if (command == L"list-profiles") {
            return ListProfiles();
        }
        if (command == L"show-profile") {
            return ShowProfile(argc, argv);
        }
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }

    PrintUsage();
    return 1;
}
