# WinDoze

**WinDoze** is a Windows game sandbox that runs titles inside a [Less-Privileged AppContainer (LPAC)](https://learn.microsoft.com/en-us/windows/win32/secauthz/appcontainer-isolation). Each game only gets filesystem access to its install directory and isolated save storage — not your Documents, other games, or arbitrary drives.

## What this solves

- Run untrusted or mod-heavy games without giving them the keys to your whole user profile
- Keep per-game save data isolated under `%ProgramData%\WinDoze\<profile>\`
- Install games into a sandbox so installers cannot spray files across the system

## What this does **not** solve (yet)

- Kernel anti-cheat (EAC, BattlEye, Vanguard) — incompatible with LPAC sandboxing
- Perfect DRM compatibility
- Steam/Epic launcher automation (planned v0.4)

See [docs/GAME_COMPATIBILITY.md](docs/GAME_COMPATIBILITY.md) for tiered compatibility guidance.

## Architecture

```
windoze.exe  →  JSON profile  →  ACL grants  →  LPAC CreateProcess  →  game.exe
```

Read the full design in [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## How WinDoze runs

**v0.1 is a single CLI executable** — not a background app or VM.

- You run `windoze.exe` when you want to create a profile or launch a game
- It configures Windows LPAC + filesystem ACLs, spawns `game.exe`, then exits (or waits for the game)
- The game window you see is the real game; WinDoze is the control plane, Windows is the sandbox runtime

See [docs/HOW_IT_RUNS.md](docs/HOW_IT_RUNS.md) for the full picture, including desktop shortcut setup.

```
You → windoze.exe launch --profile my-game → game.exe (sandboxed)
```

Future versions will add a GUI tray app (v0.4). There is no kernel driver and no always-on service in v0.1.

## Requirements

- Windows 10 version 1809+ or Windows 11 (x64)
- Visual Studio 2022 with **Desktop development with C++** workload, or Build Tools equivalent
- CMake 3.20+

## Build

WinDoze uses Windows-only APIs (AppContainer, LPAC, ACLs) and **must be built on Windows** with MSVC.

### Option A — build on your PC

Prerequisites: Visual Studio 2022 with **Desktop development with C++**, and CMake 3.20+.

```powershell
git clone <repo-url>
cd WinDoze
.\tools\build.ps1
```

Output: `build\Release\windoze.exe` (Visual Studio generator) or `build\windoze.exe` (Ninja).

Manual CMake:

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

### Option B — download CI build

Every push to `main` or `cursor/**` branches runs [GitHub Actions](.github/workflows/build.yml) on `windows-latest` and uploads `windoze.exe` as an artifact.

1. Open the repo on GitHub → **Actions** → **Build WinDoze** → latest green run
2. Download the **windoze-windows-x64** artifact
3. Unzip — `build\windoze.exe` is ready to use

### Cannot build on Linux/macOS

Cross-compiling is not supported — AppContainer APIs exist only on Windows.

## Quick start

### Sandboxed install (recommended for untrusted games)

```powershell
.\build\windoze.exe install `
  --id my-game `
  --name "My Game" `
  --install-dir "D:\Sandbox\MyGame" `
  --installer "D:\Downloads\setup.exe"

.\build\windoze.exe show-install-report --profile my-game
.\build\windoze.exe launch --profile my-game
```

See [docs/SANDBOXED_INSTALL.md](docs/SANDBOXED_INSTALL.md) for the full install workflow.

### Already-installed game

```powershell
# 1. Create a profile for an installed game
.\build\Release\windoze.exe create-profile `
  --id hollow-knight `
  --name "Hollow Knight" `
  --install-dir "D:\Games\HollowKnight" `
  --executable "D:\Games\HollowKnight\hollow_knight.exe"

# 2. Launch sandboxed
.\build\Release\windoze.exe launch --profile hollow-knight

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
| `list-profiles` | List installed profiles |
| `show-profile` | Print profile details |

## How isolation works

1. **LPAC token** — the game process is denied registry, COM, network, and filesystem access by default.
2. **Capability SIDs** — profile opts into `registryRead`, `internetClient`, etc. when needed.
3. **Filesystem ACLs** — WinDoze adds allow ACEs for the AppContainer SID only on install + save paths.

Everything else on the system remains inaccessible unless it is world-readable (e.g. `C:\Windows\System32` for DLL loading).

## Project layout

```
docs/           Architecture, threat model, compatibility, how it runs
profiles/       JSON schema + examples
src/launcher/   C++ LPAC launcher, ACL manager, profile store
tools/          PowerShell setup helpers
```

## Roadmap

- [x] v0.1 — LPAC launcher, profiles, ACL grants, CLI
- [x] v0.2 — Sandboxed installer + install footprint scanner
- [ ] v0.3 — Registry / AppData redirection
- [ ] v0.4 — GUI + launcher integration

## License

MIT — see LICENSE.

## Acknowledgements

Built on Microsoft's [AppContainer](https://learn.microsoft.com/en-us/windows/win32/secauthz/implementing-an-appcontainer) APIs. Inspired by [Sandboxie-Plus](https://github.com/sandboxie-plus/Sandboxie) and [LaunchAppContainer](https://github.com/microsoft/SandboxSecurityTools/tree/main/LaunchAppContainer).
