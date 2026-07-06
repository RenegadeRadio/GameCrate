# GameCrate

**GameCrate** is a Windows game sandbox that runs titles inside a [Less-Privileged AppContainer (LPAC)](https://learn.microsoft.com/en-us/windows/win32/secauthz/appcontainer-isolation). Each game only gets filesystem access to its install directory and isolated save storage — not your Documents, other games, or arbitrary drives.

## What this solves

- Run untrusted or mod-heavy games without giving them the keys to your whole user profile
- Keep per-game save data isolated under `%ProgramData%\GameCrate\<profile>\`
- Install games into a sandbox so installers cannot spray files across the system

## What this does **not** solve (yet)

- Kernel anti-cheat (EAC, BattlEye, Vanguard) — incompatible with LPAC sandboxing
- Perfect DRM compatibility
- Steam/Epic launcher automation (planned post-v0.4)

See [docs/GAME_COMPATIBILITY.md](docs/GAME_COMPATIBILITY.md) for tiered compatibility guidance.

## Architecture

```
gamecrate.exe  →  JSON profile  →  ACL grants  →  LPAC CreateProcess  →  game.exe
```

Read the full design in [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## How GameCrate runs

**v0.4 adds a WPF tray GUI** alongside the CLI. Both use the same `gamecrate.exe` control plane.

- Run `GameCrate.Gui.exe` for point-and-click install, play, and profile management
- Or run `gamecrate.exe` from PowerShell for scripting and automation
- The CLI configures Windows LPAC + filesystem ACLs, spawns `game.exe`, then exits (or waits for the game)
- The game window you see is the real game; GameCrate is the control plane, Windows is the sandbox runtime

See [docs/HOW_IT_RUNS.md](docs/HOW_IT_RUNS.md) and [docs/GUI.md](docs/GUI.md).

```
You → GameCrate.Gui.exe  →  gamecrate.exe launch --profile my-game  →  game.exe (sandboxed)
```

There is no kernel driver and no always-on service.

## Requirements

- Windows 10 version 1809+ or Windows 11 (x64)
- Visual Studio 2022 with **Desktop development with C++** workload, or Build Tools equivalent
- CMake 3.20+

## Build

GameCrate uses Windows-only APIs (AppContainer, LPAC, ACLs) and **must be built on Windows** with MSVC.

### Option A — build on your PC

Prerequisites: Visual Studio 2022 with **Desktop development with C++**, and CMake 3.20+.

```powershell
git clone <repo-url>
cd GameCrate
.\tools\build.ps1
```

Output: `build\Release\gamecrate.exe` (Visual Studio generator) or `build\gamecrate.exe` (Ninja).

### GUI (optional)

Requires [.NET 8 SDK](https://dotnet.microsoft.com/download/dotnet/8.0):

```powershell
dotnet publish src/gui/GameCrate.Gui/GameCrate.Gui.csproj -c Release -r win-x64 --self-contained false -o build/package
Copy-Item build\gamecrate.exe build\package\gamecrate.exe
.\build\package\GameCrate.Gui.exe
```

See [docs/GUI.md](docs/GUI.md).

Manual CMake:

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

### Option B — download CI build

Every push to `main` or `cursor/**` branches runs [GitHub Actions](.github/workflows/build.yml) on `windows-latest` and uploads `gamecrate.exe` as an artifact.

1. Open the repo on GitHub → **Actions** → **Build GameCrate** → latest green run
2. Download the **gamecrate-windows-x64** artifact
3. Unzip — `build\gamecrate.exe` is ready to use

### Cannot build on Linux/macOS

Cross-compiling is not supported — AppContainer APIs exist only on Windows.

## Quick start

### Sandboxed install (recommended for untrusted games)

```powershell
.\build\gamecrate.exe install `
  --id my-game `
  --name "My Game" `
  --install-dir "D:\Sandbox\MyGame" `
  --installer "D:\Downloads\setup.exe"

.\build\gamecrate.exe show-install-report --profile my-game
.\build\gamecrate.exe launch --profile my-game
```

See [docs/SANDBOXED_INSTALL.md](docs/SANDBOXED_INSTALL.md) for the full install workflow.

### Already-installed game

```powershell
# 1. Create a profile for an installed game
.\build\Release\gamecrate.exe create-profile `
  --id hollow-knight `
  --name "Hollow Knight" `
  --install-dir "D:\Games\HollowKnight" `
  --executable "D:\Games\HollowKnight\hollow_knight.exe"

# 2. Launch sandboxed
.\build\Release\gamecrate.exe launch --profile hollow-knight

# Or use the helper script
.\tools\New-GameSandbox.ps1 -Id hollow-knight -Name "Hollow Knight" `
  -InstallDir "D:\Games\HollowKnight" -Executable "D:\Games\HollowKnight\hollow_knight.exe" -Launch
```

### Online games

Add `--network` when creating the profile (grants `internetClient` + `privateNetworkClientServer`).

### GPU (default on)

Game profiles enable GPU capabilities by default (`lpacPnpNotifications`, `lpacMedia`) for DirectX adapter access. Use `--no-gpu` only for non-graphical sandboxes.

## CLI reference

| Command | Description |
|---|---|
| `install` | Run installer in LPAC + footprint scan |
| `create-profile` | Register AppContainer + save JSON profile + apply ACLs |
| `launch` | Start the game inside LPAC |
| `grant` | Re-apply filesystem ACL grants |
| `show-install-report` | Print install footprint report |
| `destroy-profile` | Remove profile; use `--wipe-data` to delete sandbox files |
| `list-profiles` | List installed profiles (`--json` for machine-readable output) |
| `show-profile` | Print profile details |

## How isolation works

1. **LPAC token** — the game process is denied registry, COM, network, and filesystem access by default.
2. **Capability SIDs** — profile opts into `registryRead`, `internetClient`, etc. when needed.
3. **Filesystem ACLs** — GameCrate adds allow ACEs for the AppContainer SID only on install + save paths.

Everything else on the system remains inaccessible unless it is world-readable (e.g. `C:\Windows\System32` for DLL loading).

## Project layout

```
docs/           Architecture, threat model, compatibility, how it runs, GUI
profiles/       JSON schema + examples
src/launcher/   C++ LPAC launcher, ACL manager, profile store
src/gui/        WPF tray GUI (GameCrate.Gui)
tools/          PowerShell setup helpers
```

## Roadmap

- [x] v0.1 — LPAC launcher, profiles, ACL grants, CLI
- [x] v0.2 — Sandboxed installer + install footprint scanner
- [x] v0.3 — AppData redirection, registry install scan, destroy-profile
- [x] v0.4 — WPF tray GUI wrapping the CLI
- [ ] v0.5 — Steam/Epic launcher integration

## License

MIT — see LICENSE.

## Acknowledgements

Built on Microsoft's [AppContainer](https://learn.microsoft.com/en-us/windows/win32/secauthz/implementing-an-appcontainer) APIs. Inspired by [Sandboxie-Plus](https://github.com/sandboxie-plus/Sandboxie) and [LaunchAppContainer](https://github.com/microsoft/SandboxSecurityTools/tree/main/LaunchAppContainer).
