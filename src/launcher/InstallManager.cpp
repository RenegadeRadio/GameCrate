#include "windoze/InstallManager.hpp"

#include "windoze/AclManager.hpp"
#include "windoze/AppContainerLauncher.hpp"

#include <ShlObj.h>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace windoze {

namespace {

std::wstring ProfileDataRoot(const std::wstring& profileId) {
    wchar_t programData[MAX_PATH] = {};
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_COMMON_APPDATA, nullptr, SHGFP_TYPE_CURRENT, programData))) {
        return std::wstring(programData) + L"\\WinDoze\\" + profileId;
    }
    return L"C:\\ProgramData\\WinDoze\\" + profileId;
}

std::wstring DefaultSaveDir(const std::wstring& id) {
    return ProfileDataRoot(id) + L"\\saves";
}

bool EnsureDirectory(const std::wstring& path) {
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    return std::filesystem::exists(path, ec);
}

bool IsInstallerLikeName(const std::wstring& fileName) {
    const std::wstring lower = PathUtils::ToLower(fileName);
    if (lower == L"setup.exe" || lower == L"install.exe" || lower == L"launcher.exe") {
        return true;
    }
    return lower.find(L"unins") != std::wstring::npos || lower.find(L"installer") != std::wstring::npos;
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

std::wstring WriteStringArrayJson(const std::vector<std::wstring>& values) {
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

bool RegisterProfile(const SandboxProfile& profile) {
    std::vector<SID_AND_ATTRIBUTES> capabilityAttributes;
    std::vector<CapabilitySid> ownedCapabilities;
    for (const auto& capabilityName : ProfileStore::CapabilitiesFor(profile)) {
        CapabilitySid capability;
        if (AppContainerLauncher::ResolveCapability(capabilityName, capability)) {
            capabilityAttributes.push_back({capability.sid, capability.attributes});
            ownedCapabilities.push_back(std::move(capability));
        }
    }

    PSID packageSid = nullptr;
    if (!AppContainerLauncher::CreateOrResolveProfile(
            profile.moniker, profile.name, capabilityAttributes, &packageSid)) {
        return false;
    }
    if (packageSid) {
        FreeSid(packageSid);
    }
    return true;
}

}  // namespace

bool InstallManager::ApplyProfileAcls(const SandboxProfile& profile, AclMode mode) {
    PSID packageSid = nullptr;
    HRESULT hr = DeriveAppContainerSidFromAppContainerName(profile.moniker.c_str(), &packageSid);
    if (FAILED(hr)) {
        std::vector<SID_AND_ATTRIBUTES> capabilities;
        if (!AppContainerLauncher::CreateOrResolveProfile(
                profile.moniker, profile.name, capabilities, &packageSid)) {
            return false;
        }
    }

    const PathAccess installAccess =
        mode == AclMode::Install ? PathAccess::ReadWriteExecute : PathAccess::ReadExecute;

    std::vector<AclGrant> grants;
    auto addGrant = [&](const std::wstring& path, PathAccess access) {
        if (!path.empty()) {
            grants.push_back({path, access});
        }
    };

    addGrant(profile.installDir, installAccess);
    addGrant(profile.saveDir, PathAccess::ReadWrite);

    for (const auto& path : profile.writablePaths) {
        if (PathUtils::Normalize(path) == PathUtils::Normalize(profile.installDir)) {
            continue;
        }
        addGrant(path, PathAccess::ReadWrite);
    }
    for (const auto& path : profile.readablePaths) {
        addGrant(path, PathAccess::ReadExecute);
    }

    const bool ok = AclManager::GrantAppContainerAccess(packageSid, grants);
    if (packageSid) {
        FreeSid(packageSid);
    }
    return ok;
}

std::wstring InstallManager::DetectExecutable(const std::wstring& installDir) {
    struct Candidate {
        std::wstring path;
        uintmax_t size = 0;
        int depth = 0;
    };

    std::vector<Candidate> candidates;
    std::error_code ec;
    if (!std::filesystem::exists(installDir, ec)) {
        return L"";
    }

    const std::filesystem::directory_options options =
        std::filesystem::directory_options::skip_permission_denied;

    std::filesystem::recursive_directory_iterator it(installDir, options, ec);
    const std::filesystem::recursive_directory_iterator end;
    for (; it != end; it.increment(ec)) {
        if (ec) {
            break;
        }
        const auto& entry = *it;
        if (!entry.is_regular_file(ec)) {
            continue;
        }
        if (it.depth() > 3) {
            it.disable_recursion_pending();
            continue;
        }
        if (entry.path().extension() != L".exe") {
            continue;
        }

        const std::wstring fileName = entry.path().filename().wstring();
        if (IsInstallerLikeName(fileName)) {
            continue;
        }

        candidates.push_back(
            {entry.path().wstring(), entry.file_size(ec), static_cast<int>(it.depth())});
    }

    if (candidates.empty()) {
        return L"";
    }

    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
        if (a.depth != b.depth) {
            return a.depth < b.depth;
        }
        return a.size > b.size;
    });

    return candidates.front().path;
}

bool InstallManager::WriteReport(const InstallResult& result, const InstallOptions& options) {
    const std::wstring reportDir = ProfileDataRoot(options.id);
    if (!EnsureDirectory(reportDir)) {
        return false;
    }

    const std::wstring reportPath = reportDir + L"\\install-report.json";
    std::wstringstream json;
    json << L"{\n";
    json << L"  \"profileId\": \"" << EscapeJson(options.id) << L"\",\n";
    json << L"  \"installer\": \"" << EscapeJson(options.installer) << L"\",\n";
    json << L"  \"installerExitCode\": " << result.installerExitCode << L",\n";
    json << L"  \"executable\": \"" << EscapeJson(result.executable) << L"\",\n";
    json << L"  \"installedFileCount\": " << result.installedFiles.size() << L",\n";
    json << L"  \"outsideWriteCount\": " << result.outsideWrites.size() << L",\n";
    json << L"  \"suspiciousOutsideWriteCount\": " << result.suspiciousOutsideWrites.size() << L",\n";
    json << L"  \"installedFiles\": " << WriteStringArrayJson(result.installedFiles) << L",\n";
    json << L"  \"outsideWrites\": " << WriteStringArrayJson(result.outsideWrites) << L",\n";
    json << L"  \"suspiciousOutsideWrites\": " << WriteStringArrayJson(result.suspiciousOutsideWrites)
         << L"\n";
    json << L"}\n";

    std::wofstream out(reportPath);
    if (!out.is_open()) {
        return false;
    }
    out << json.str();
    return true;
}

InstallResult InstallManager::Run(const InstallOptions& options) {
    InstallResult result;

    if (options.id.empty() || options.name.empty() || options.installDir.empty() ||
        options.installer.empty()) {
        result.message = L"install requires --id, --name, --install-dir, and --installer.";
        return result;
    }

    if (GetFileAttributesW(options.installer.c_str()) == INVALID_FILE_ATTRIBUTES) {
        result.message = L"Installer not found: " + options.installer;
        return result;
    }

    InstallOptions normalized = options;
    if (normalized.saveDir.empty()) {
        normalized.saveDir = DefaultSaveDir(normalized.id);
    }
    if (!EnsureDirectory(normalized.installDir) || !EnsureDirectory(normalized.saveDir)) {
        result.message = L"Failed to create install or save directory.";
        return result;
    }

    SandboxProfile profile;
    profile.id = normalized.id;
    profile.name = normalized.name;
    profile.moniker = L"WinDoze." + normalized.id;
    profile.installDir = normalized.installDir;
    profile.saveDir = normalized.saveDir;
    profile.executable = normalized.executable;
    profile.network = normalized.network;
    profile.registryRead = normalized.registryRead;
    profile.lpacCom = normalized.lpacCom;
    profile.gpu = normalized.gpu;
    profile.writablePaths = {profile.installDir, profile.saveDir};

    if (!RegisterProfile(profile)) {
        result.message = L"Failed to register AppContainer profile.";
        return result;
    }

    if (!ApplyProfileAcls(profile, AclMode::Install)) {
        result.message = L"Failed to apply install-phase ACL grants.";
        return result;
    }

    const auto allowedRoots =
        FootprintScanner::AllowedInstallRoots(profile.id, profile.installDir, profile.saveDir);
    const auto watchPaths = FootprintScanner::SensitiveWatchPaths();

    const FootprintSnapshot beforeAllowed = FootprintScanner::ScanRoots(allowedRoots);
    const FootprintSnapshot beforeWatch = FootprintScanner::ScanRoots(watchPaths);

    LaunchOptions launchOptions;
    launchOptions.moniker = profile.moniker;
    launchOptions.displayName = profile.name;
    launchOptions.executable = normalized.installer;
    launchOptions.arguments = normalized.installerArgs;
    launchOptions.capabilities = ProfileStore::CapabilitiesFor(profile);
    launchOptions.lessPrivileged = true;
    launchOptions.waitForExit = true;
    launchOptions.retainProfile = true;
    launchOptions.workingDirectory = normalized.installDir;

    const LaunchResult launchResult = AppContainerLauncher::Launch(launchOptions);
    result.installerExitCode = launchResult.exitCode;

    if (!launchResult.success) {
        result.message = L"Installer failed: " + launchResult.message;
        return result;
    }

    const FootprintSnapshot afterAllowed = FootprintScanner::ScanRoots(allowedRoots);
    const FootprintSnapshot afterWatch = FootprintScanner::ScanRoots(watchPaths);

    const FootprintDiff allowedDiff = FootprintScanner::Compare(beforeAllowed, afterAllowed);
    const FootprintDiff watchDiff = FootprintScanner::Compare(beforeWatch, afterWatch);

    for (const auto& file : allowedDiff.added) {
        result.installedFiles.push_back(file.path);
    }
    for (const auto& file : allowedDiff.modified) {
        result.installedFiles.push_back(file.path);
    }

    for (const auto& file : watchDiff.added) {
        if (PathUtils::IsUnderAnyRoot(file.path, allowedRoots)) {
            continue;
        }
        result.outsideWrites.push_back(file.path);
        if (FootprintScanner::IsSuspiciousOutsidePath(file.path)) {
            result.suspiciousOutsideWrites.push_back(file.path);
        }
    }

    if (profile.executable.empty()) {
        profile.executable = DetectExecutable(profile.installDir);
    }
    result.executable = profile.executable;

    if (profile.executable.empty()) {
        result.message =
            L"Install finished but no game executable was detected. Set --executable manually.";
        result.success = false;
    } else {
        result.success = true;
    }

    if (!result.outsideWrites.empty()) {
        result.success = false;
        result.message = L"Detected file writes outside the sandbox footprint.";
        if (!normalized.failOnOutsideWrites) {
            result.success = profile.executable.empty() ? false : true;
        }
    }

    if (!ProfileStore::Save(profile)) {
        result.success = false;
        result.message = L"Failed to save profile after install.";
        return result;
    }

    if (normalized.tightenAclsAfter) {
        ApplyProfileAcls(profile, AclMode::Run);
    }

    result.reportPath = ProfileDataRoot(profile.id) + L"\\install-report.json";
    WriteReport(result, normalized);

    if (result.success) {
        result.message = L"Install completed successfully.";
    }

    return result;
}

}  // namespace windoze
