#pragma once

#include <Windows.h>
#include <Aclapi.h>
#include <sddl.h>

#include <string>
#include <vector>

namespace gamecrate {

enum class PathAccess {
    Read,
    ReadExecute,
    ReadWrite,
    ReadWriteExecute,
};

struct AclGrant {
    std::wstring path;
    PathAccess access = PathAccess::ReadWriteExecute;
};

class AclManager {
public:
    static DWORD GrantAppContainerAccess(PSID appContainerSid, const AclGrant& grant);
    static bool GrantAppContainerAccess(PSID appContainerSid, const std::vector<AclGrant>& grants);
    static bool GrantAppContainerAccess(
        PSID appContainerSid,
        const std::vector<AclGrant>& grants,
        std::wstring& failedPath,
        DWORD& failedError);
    static bool RemoveAppContainerAccess(PSID appContainerSid, const std::wstring& path);
    static DWORD AccessMaskFor(PathAccess access);
};

}  // namespace gamecrate
