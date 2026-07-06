#include "windoze/AclManager.hpp"
#include "windoze/AppContainerLauncher.hpp"
#include "windoze/ProfileStore.hpp"

#include <ShlObj.h>

#include <iostream>
#include <sstream>

namespace {

void PrintUsage() {
    std::wcerr
        << L"WinDoze — sandboxed game launcher for Windows\n\n"
        << L"Usage:\n"
        << L"  windoze create-profile --id <id> --name <name> --install-dir <path> --executable <path>\n"
        << L"  windoze launch --profile <id> [--wait]\n"
        << L"  windoze grant --profile <id>\n"
        << L"  windoze list-profiles\n"
        << L"  windoze show-profile --profile <id>\n\n"
        << L"Options for create-profile:\n"
        << L"  --save-dir <path>       Isolated save directory (default: %ProgramData%\\WinDoze\\<id>\\saves)\n"
        << L"  --network               Grant internet and LAN capabilities\n"
        << L"  --no-registry           Do not grant registryRead (stricter, breaks many games)\n"
        << L"  --lpac-com              Grant COM capability for launcher-heavy titles\n";
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
        return std::wstring(programData) + L"\\WinDoze\\" + id + L"\\saves";
    }
    return L"C:\\ProgramData\\WinDoze\\" + id + L"\\saves";
}

bool ApplyProfileAcls(const windoze::SandboxProfile& profile) {
    PSID packageSid = nullptr;
    HRESULT hr = DeriveAppContainerSidFromAppContainerName(profile.moniker.c_str(), &packageSid);
    if (FAILED(hr)) {
        std::vector<SID_AND_ATTRIBUTES> capabilities;
        if (!windoze::AppContainerLauncher::CreateOrResolveProfile(
                profile.moniker,
                profile.name,
                capabilities,
                &packageSid)) {
            std::wcerr << L"Failed to resolve AppContainer SID for profile.\n";
            return false;
        }
    }

    std::vector<windoze::AclGrant> grants;

    auto addGrant = [&](const std::wstring& path, windoze::PathAccess access) {
        if (!path.empty()) {
            grants.push_back({path, access});
        }
    };

    addGrant(profile.installDir, windoze::PathAccess::ReadWriteExecute);
    addGrant(profile.saveDir, windoze::PathAccess::ReadWrite);

    for (const auto& path : profile.writablePaths) {
        addGrant(path, windoze::PathAccess::ReadWrite);
    }
    for (const auto& path : profile.readablePaths) {
        addGrant(path, windoze::PathAccess::ReadExecute);
    }

    const bool ok = windoze::AclManager::GrantAppContainerAccess(packageSid, grants);
    if (packageSid) {
        FreeSid(packageSid);
    }
    return ok;
}

int CreateProfile(int argc, wchar_t** argv) {
    windoze::SandboxProfile profile;
    profile.registryRead = true;

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
        }
    }

    if (profile.id.empty() || profile.name.empty() || profile.installDir.empty()) {
        std::wcerr << L"create-profile requires --id, --name, and --install-dir.\n";
        return 1;
    }

    if (profile.moniker.empty()) {
        profile.moniker = L"WinDoze." + profile.id;
    }
    if (profile.saveDir.empty()) {
        profile.saveDir = DefaultSaveDir(profile.id);
    }
    if (profile.executable.empty()) {
        profile.executable = profile.installDir;
    }

    profile.writablePaths = {profile.installDir, profile.saveDir};

    if (!windoze::ProfileStore::Save(profile)) {
        std::wcerr << L"Failed to save profile.\n";
        return 1;
    }

    std::vector<SID_AND_ATTRIBUTES> capabilityAttributes;
    std::vector<windoze::CapabilitySid> ownedCapabilities;
    for (const auto& capabilityName : windoze::ProfileStore::CapabilitiesFor(profile)) {
        windoze::CapabilitySid capability;
        if (windoze::AppContainerLauncher::ResolveCapability(capabilityName, capability)) {
            capabilityAttributes.push_back({capability.sid, capability.attributes});
            ownedCapabilities.push_back(std::move(capability));
        }
    }

    PSID packageSid = nullptr;
    if (!windoze::AppContainerLauncher::CreateOrResolveProfile(
            profile.moniker, profile.name, capabilityAttributes, &packageSid)) {
        std::wcerr << L"Failed to register AppContainer profile.\n";
        return 1;
    }
    if (packageSid) {
        FreeSid(packageSid);
    }

    if (!ApplyProfileAcls(profile)) {
        std::wcerr << L"Profile saved but ACL grants failed. Re-run 'windoze grant --profile "
                   << profile.id << L"'.\n";
        return 1;
    }

    std::wcout << L"Created profile '" << profile.id << L"' with moniker " << profile.moniker << L"\n";
    return 0;
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

    windoze::SandboxProfile profile;
    if (!windoze::ProfileStore::Load(profileId, profile)) {
        std::wcerr << L"Profile not found: " << profileId << L"\n";
        return 1;
    }

    if (!ApplyProfileAcls(profile)) {
        std::wcerr << L"Warning: ACL grants may be incomplete.\n";
    }

    windoze::LaunchOptions options;
    options.moniker = profile.moniker;
    options.displayName = profile.name;
    options.executable = profile.executable;
    options.arguments = profile.arguments;
    options.capabilities = windoze::ProfileStore::CapabilitiesFor(profile);
    options.lessPrivileged = true;
    options.waitForExit = waitForExit;
    options.retainProfile = true;
    options.workingDirectory = profile.installDir;

    const windoze::LaunchResult result = windoze::AppContainerLauncher::Launch(options);
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

    windoze::SandboxProfile profile;
    if (!windoze::ProfileStore::Load(profileId, profile)) {
        std::wcerr << L"Profile not found: " << profileId << L"\n";
        return 1;
    }

    if (!ApplyProfileAcls(profile)) {
        std::wcerr << L"Failed to apply ACL grants.\n";
        return 1;
    }

    std::wcout << L"ACL grants applied for profile '" << profileId << L"'.\n";
    return 0;
}

int ListProfiles() {
    const auto profiles = windoze::ProfileStore::ListProfiles();
    if (profiles.empty()) {
        std::wcout << L"No profiles found.\n";
        return 0;
    }

    for (const auto& id : profiles) {
        windoze::SandboxProfile profile;
        if (windoze::ProfileStore::Load(id, profile)) {
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

    windoze::SandboxProfile profile;
    if (!windoze::ProfileStore::Load(profileId, profile)) {
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
               << L"registryRead: " << (profile.registryRead ? L"true" : L"false") << L"\n";
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
        if (command == L"create-profile") {
            return CreateProfile(argc, argv);
        }
        if (command == L"launch") {
            return LaunchProfile(argc, argv);
        }
        if (command == L"grant") {
            return GrantProfile(argc, argv);
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
