#include "gamecrate/Win32Error.hpp"

namespace gamecrate {

std::wstring FormatWin32Error(DWORD error) {
    if (error == 0) {
        return L"";
    }

    wchar_t* buffer = nullptr;
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS;
    const DWORD length = FormatMessageW(flags, nullptr, error, 0, reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);
    if (length == 0 || buffer == nullptr) {
        return L"Win32 error " + std::to_wstring(error);
    }

    std::wstring message(buffer, length);
    LocalFree(buffer);

    while (!message.empty() && (message.back() == L'\r' || message.back() == L'\n')) {
        message.pop_back();
    }

    return message;
}

}  // namespace gamecrate
