#pragma once

#include <string>

namespace gamecrate {

// Writes UTF-8 to redirected stdout/stderr (GUI child-process pipes).
void WriteStdout(const std::wstring& text);
void WriteStderr(const std::wstring& text);

}  // namespace gamecrate
