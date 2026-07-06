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

## Requirements

- Windows 10 version 1809+ or Windows 11 (x64)
- Visual Studio 2022 with **Desktop development with C++** workload, or Build Tools equivalent
- CMake 3.20+

## Build

```powershell
git clone <repo-url>
cd WinDoze
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The launcher binary is `build\Release\windoze.exe`.

## Quick start

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

## CLI reference

| Command | Description |
|---|---|
| `create-profile` | Register AppContainer + save JSON profile + apply ACLs |
| `launch` | Start the game inside LPAC |
| `grant` | Re-apply filesystem ACL grants |
| `list-profiles` | List installed profiles |
| `show-profile` | Print profile details |

## How isolation works

1. **LPAC token** — the game process is denied registry, COM, network, and filesystem access by default.
2. **Capability SIDs** — profile opts into `registryRead`, `internetClient`, etc. when needed.
3. **Filesystem ACLs** — WinDoze adds allow ACEs for the AppContainer SID only on install + save paths.

Everything else on the system remains inaccessible unless it is world-readable (e.g. `C:\Windows\System32` for DLL loading).

## Project layout

```
docs/           Architecture, threat model, compatibility
profiles/       JSON schema + examples
src/launcher/   C++ LPAC launcher, ACL manager, profile store
tools/          PowerShell setup helpers
```

## Roadmap

- [x] v0.1 — LPAC launcher, profiles, ACL grants, CLI
- [ ] v0.2 — Sandboxed installer + install footprint scanner
- [ ] v0.3 — Registry / AppData redirection
- [ ] v0.4 — GUI + launcher integration

## License

MIT — see LICENSE.

## Acknowledgements

Built on Microsoft's [AppContainer](https://learn.microsoft.com/en-us/windows/win32/secauthz/implementing-an-appcontainer) APIs. Inspired by [Sandboxie-Plus](https://github.com/sandboxie-plus/Sandboxie) and [LaunchAppContainer](https://github.com/microsoft/SandboxSecurityTools/tree/main/LaunchAppContainer).
