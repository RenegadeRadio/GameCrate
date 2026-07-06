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
  from `std::wstring`), which libstdc++/mingw does not support — so a mingw-w64
  cross-compile fails without source changes and is not a supported path.
- The GUI (`src/gui/GameCrate.Gui`) is WPF (`net8.0-windows`, `UseWPF`), which only
  builds on Windows.

### Supported build/run (Windows only — reference, not runnable here)
- CLI: Visual Studio 2022 "Desktop development with C++" + CMake 3.20+. Build via
  `tools/build.ps1` or `cmake -B build -G "Visual Studio 17 2022" -A x64 && cmake --build build --config Release`. See `README.md`.
- GUI: .NET 8 SDK. `dotnet publish src/gui/GameCrate.Gui/GameCrate.Gui.csproj -c Release -r win-x64`.
- CLI smoke test ("hello world"): `gamecrate.exe list-profiles`.
- The real build/run verification is GitHub Actions `windows-latest`
  (`.github/workflows/build.yml`). Use `gh run list` / `gh run view` to inspect it.

### Known pre-existing issue (do not fix as part of env setup)
- The GUI currently fails to compile: `src/gui/GameCrate.Gui/App.xaml.cs` →
  `CS0103: The name 'GameCrateService' does not exist`. On CI this fails the
  `Publish GUI` step and skips `Verify binary`. The C++ `Build Release` step
  (the CLI) passes.

### What CAN be checked on this Linux VM
- The profile JSON data contract in `profiles/` (the config that both the CLI and
  GUI consume) can be validated with `python3-jsonschema`. Example:
  validate `profiles/examples/offline-indie.json` (minus the doc-only
  `$schema`/`title`/`description` keys) against `profiles/schema.json`.
