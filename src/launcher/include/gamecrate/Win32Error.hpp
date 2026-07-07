#pragma once

#include <Windows.h>

#include <string>

namespace gamecrate {

std::wstring FormatWin32Error(DWORD error);

}  // namespace gamecrate
