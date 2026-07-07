#include "gamecrate/ConsoleIo.hpp"

#include <Windows.h>

namespace gamecrate {

namespace {

void WriteUtf8Handle(HANDLE handle, const std::wstring& text) {
    if (text.empty() || handle == nullptr || handle == INVALID_HANDLE_VALUE) {
        return;
    }

    const int byteCount = WideCharToMultiByte(
        CP_UTF8,
        0,
        text.c_str(),
        static_cast<int>(text.size()),
        nullptr,
        0,
        nullptr,
        nullptr);
    if (byteCount <= 0) {
        return;
    }

    std::string utf8(static_cast<size_t>(byteCount), '\0');
    WideCharToMultiByte(
        CP_UTF8,
        0,
        text.c_str(),
        static_cast<int>(text.size()),
        utf8.data(),
        byteCount,
        nullptr,
        nullptr);

    DWORD written = 0;
    WriteFile(handle, utf8.data(), static_cast<DWORD>(utf8.size()), &written, nullptr);
}

}  // namespace

void WriteStdout(const std::wstring& text) {
    WriteUtf8Handle(GetStdHandle(STD_OUTPUT_HANDLE), text);
}

void WriteStderr(const std::wstring& text) {
    WriteUtf8Handle(GetStdHandle(STD_ERROR_HANDLE), text);
}

}  // namespace gamecrate
