#include "gamecrate/AclManager.hpp"

#include <filesystem>

namespace gamecrate {

DWORD AclManager::AccessMaskFor(PathAccess access) {
    switch (access) {
        case PathAccess::Read:
            return FILE_GENERIC_READ;
        case PathAccess::ReadExecute:
            return FILE_GENERIC_READ | FILE_GENERIC_EXECUTE;
        case PathAccess::ReadWrite:
            return FILE_GENERIC_READ | FILE_GENERIC_WRITE | DELETE;
        case PathAccess::ReadWriteExecute:
        default:
            return FILE_GENERIC_READ | FILE_GENERIC_WRITE | FILE_GENERIC_EXECUTE | DELETE;
    }
}

bool AclManager::GrantAppContainerAccess(PSID appContainerSid, const AclGrant& grant) {
    if (!appContainerSid || grant.path.empty()) {
        return false;
    }

    std::error_code ec;
    std::filesystem::path target(grant.path);
    if (!std::filesystem::exists(target, ec)) {
        std::filesystem::create_directories(target, ec);
    }

    PSECURITY_DESCRIPTOR securityDescriptor = nullptr;
    PACL oldDacl = nullptr;
    PACL newDacl = nullptr;

    const DWORD result = GetNamedSecurityInfoW(
        grant.path.c_str(),
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        nullptr,
        nullptr,
        &oldDacl,
        nullptr,
        &securityDescriptor);

    if (result != ERROR_SUCCESS) {
        return false;
    }

    EXPLICIT_ACCESSW explicitAccess{};
    explicitAccess.grfAccessPermissions = AccessMaskFor(grant.access);
    explicitAccess.grfAccessMode = GRANT_ACCESS;
    explicitAccess.grfInheritance = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
    explicitAccess.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    explicitAccess.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    explicitAccess.Trustee.ptstrName = static_cast<LPWSTR>(appContainerSid);

    if (SetEntriesInAclW(1, &explicitAccess, oldDacl, &newDacl) != ERROR_SUCCESS) {
        if (securityDescriptor) {
            LocalFree(securityDescriptor);
        }
        return false;
    }

    const DWORD setResult = SetNamedSecurityInfoW(
        const_cast<LPWSTR>(grant.path.c_str()),
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        nullptr,
        nullptr,
        newDacl,
        nullptr);

    if (newDacl) {
        LocalFree(newDacl);
    }
    if (securityDescriptor) {
        LocalFree(securityDescriptor);
    }

    return setResult == ERROR_SUCCESS;
}

bool AclManager::GrantAppContainerAccess(PSID appContainerSid, const std::vector<AclGrant>& grants) {
    bool ok = true;
    for (const auto& grant : grants) {
        ok = GrantAppContainerAccess(appContainerSid, grant) && ok;
    }
    return ok;
}

bool AclManager::RemoveAppContainerAccess(PSID appContainerSid, const std::wstring& path) {
    if (!appContainerSid || path.empty()) {
        return false;
    }

    PSECURITY_DESCRIPTOR securityDescriptor = nullptr;
    PACL oldDacl = nullptr;
    PACL newDacl = nullptr;

    const DWORD result = GetNamedSecurityInfoW(
        path.c_str(),
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        nullptr,
        nullptr,
        &oldDacl,
        nullptr,
        &securityDescriptor);

    if (result != ERROR_SUCCESS) {
        return false;
    }

    EXPLICIT_ACCESSW explicitAccess{};
    explicitAccess.grfAccessPermissions = 0;
    explicitAccess.grfAccessMode = REVOKE_ACCESS;
    explicitAccess.grfInheritance = NO_INHERITANCE;
    explicitAccess.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    explicitAccess.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    explicitAccess.Trustee.ptstrName = static_cast<LPWSTR>(appContainerSid);

    if (SetEntriesInAclW(1, &explicitAccess, oldDacl, &newDacl) != ERROR_SUCCESS) {
        if (securityDescriptor) {
            LocalFree(securityDescriptor);
        }
        return false;
    }

    const DWORD setResult = SetNamedSecurityInfoW(
        const_cast<LPWSTR>(path.c_str()),
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        nullptr,
        nullptr,
        newDacl,
        nullptr);

    if (newDacl) {
        LocalFree(newDacl);
    }
    if (securityDescriptor) {
        LocalFree(securityDescriptor);
    }

    return setResult == ERROR_SUCCESS;
}

}  // namespace gamecrate
