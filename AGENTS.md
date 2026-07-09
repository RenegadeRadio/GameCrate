# AGENTS.md

## Cursor Cloud specific instructions

GameCrate is a **Windows-only** application. It **cannot be built or run on the
Linux Cloud Agent VM**, and this is an inherent platform limitation, not a fixable
setup gap. Do not attempt to "fix" the environment to build it on Linux.

### Why it is Windows-only

- The CLI (`gamecrate.exe`, `src/launcher/*.cpp`) uses AppContainer/LPAC/ACL APIs
  (`<Windows.h>`, `<Aclapi.h>`, `<sddl.h>`, `<UserEnv.h>`, `<ShlObj.h>`) and links
  Windows libs (`Userenv`, `onecoreuap`, `Advapi32`, `Shell32`). It also relies on
  MSVC-specific STL behaviour (e.g. `std::wifstream`/`std::wofstream` constructed
  from `std::wstring`), which libstdc++/mingw does not support.
- The GUI (`src/gui/GameCrate.Gui`) is WPF (`net8.0-windows`, `UseWPF`), which only
  builds on Windows.

### Supported build/run (Windows only — reference, not runnable here)

- **Pre-built download:** [GitHub Releases](https://github.com/RenegadeRadio/GameCrate/releases)
  (`GameCrate-windows-x64.zip`, self-contained, no .NET install needed).
- **CLI:** Visual Studio 2022 "Desktop development with C++" + CMake 3.20+.
  Build via `tools/build.ps1` or CMake + Ninja. See `README.md`.
- **GUI:** .NET 8 SDK. `dotnet publish src/gui/GameCrate.Gui/GameCrate.Gui.csproj -c Release -r win-x64`.
- **CLI smoke test:** `gamecrate.exe list-profiles`.
- **CI:** GitHub Actions `windows-latest` (`.github/workflows/build.yml`). Pushes to
  `main` publish the `v0.4-latest` release. Use `gh run list` / `gh run view` to inspect.

### What CAN be checked on this Linux VM

- Profile JSON in `profiles/` — validate examples against `profiles/schema.json`.
- Documentation, README, and workflow YAML consistency.
- Grep/review C# and C++ source (no compile).

### Repo conventions

- Product name: **GameCrate** (`gamecrate.exe`, `GameCrate.Gui.exe`). Formerly WinDoze.
- Product scope: **standalone/portable games only** — not Steam/Epic/GOG integration. See `docs/SCOPE.md`.
- Data paths: `%LOCALAPPDATA%\GameCrate\` (or `GAMECRATE_DATA_ROOT` for dev — see `docs/DEVELOPMENT.md`, `tools/Enter-GameCrateDev.ps1`).
- Feature branches: `cursor/<descriptive-name>-ae40`.
- Default branch: `main`.
