# Windows development setup

GameCrate **must be built on Windows** (MSVC + WPF). A Linux Docker container cannot compile or run the LPAC launcher. For day-to-day work on a Windows PC, use the **project-local dev environment** so profiles, saves, and test installs stay inside the repo instead of `%LOCALAPPDATA%`.

## Quick start (Windows)

Prerequisites: Visual Studio 2022 (**Desktop development with C++**), CMake 3.20+, [.NET 8 SDK](https://dotnet.microsoft.com/download/dotnet/8.0).

```powershell
git clone https://github.com/RenegadeRadio/GameCrate.git
cd GameCrate

# Isolated dev shell — data goes to .dev-data\, not AppData
.\tools\Enter-GameCrateDev.ps1 -Build -PublishGui

# In the new shell:
gamecrate list-profiles
.\build\dev-package\GameCrate.Gui.exe
```

### What stays isolated

| Normal install | Dev environment |
|----------------|-----------------|
| `%LOCALAPPDATA%\GameCrate\` | `<repo>\.dev-data\` |
| Game installs anywhere you pick | Suggested: `<repo>\.dev-data\games\<name>\` |
| `build\` artifacts | Still `build\` (gitignored) |

Set `GAMECRATE_DATA_ROOT` manually if you prefer a different folder:

```powershell
$env:GAMECRATE_DATA_ROOT = "D:\GameCrateDev\data"
.\build\Release\gamecrate.exe list-profiles
```

The GUI inherits the same variable when it spawns `gamecrate.exe`.

### Reset dev state

```powershell
Remove-Item -Recurse -Force .dev-data\profiles, .dev-data\*
# Keep .dev-data\README.md — or delete the whole .dev-data folder
```

## Dev Container (Linux — validation only)

The [`.devcontainer/`](../.devcontainer/) config is for **profile JSON validation** and doc work in Cursor/GitHub Codespaces. It runs `tools/validate-profiles.py` on create.

It does **not** build `gamecrate.exe` or the WPF GUI. Open the repo on your **Windows host** for full compile/test.

## Why not Docker for the full app?

| Requirement | Linux container | Windows host |
|-------------|-----------------|--------------|
| AppContainer / LPAC APIs | No | Yes |
| MSVC + Ninja build | No (in practice) | Yes |
| WPF (`net8.0-windows`) | No | Yes |
| Run sandboxed games | No | Yes |

Optional: [Windows Sandbox](https://learn.microsoft.com/en-us/windows/security/application-security/application-isolation/windows-sandbox/windows-sandbox-overview) for disposable VM-style testing of installers — still use `Enter-GameCrateDev.ps1` inside the sandbox with the repo mapped in.

## Typical dev loop

```powershell
.\tools\Enter-GameCrateDev.ps1 -Build
gamecrate install --id my-test --name "My Test" `
  --install-dir "$env:GAMECRATE_DEV_GAMES_DIR\my-test" `
  --installer "$env:USERPROFILE\Downloads\setup.exe"
gamecrate launch --profile my-test
gamecrate show-install-report --profile my-test
gamecrate destroy-profile --profile my-test --wipe-data
```

## Related

- [CLI.md](CLI.md) — command reference
- [TESTING.md](TESTING.md) — handout for testers (production paths use real AppData)
- [WINGET.md](WINGET.md) — install release builds via winget manifest
