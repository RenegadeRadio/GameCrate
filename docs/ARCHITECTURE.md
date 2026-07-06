# GameCrate Architecture

GameCrate is a Windows game sandbox that runs each title inside a **Less-Privileged AppContainer (LPAC)** with explicit filesystem grants. The goal is simple: a game can only touch files you installed for it, plus its own isolated save/profile storage.

## Design goals

1. **Filesystem isolation** — deny read/write outside granted paths by default.
2. **Per-game profiles** — each game gets its own AppContainer identity and ACL set.
3. **Install-time sandboxing** — installers run inside the same box so stray files never land on the host.
4. **Practical game support** — optional network, registry, and shared-library read access where needed.
5. **No kernel driver for v1** — use built-in Windows security (AppContainer + ACLs) before investing in a minifilter.

## Why not Docker / Windows Containers / Hyper-V?

| Approach | Problem for games |
|---|---|
| Windows Server containers | No GUI, no DirectX stack |
| Docker Desktop on Windows | Same — server-oriented, not for interactive 3D apps |
| Windows Sandbox (VM) | Full VM per session; poor GPU passthrough; slow startup |
| WSL2 | Linux only |

Games need low-latency GPU access, raw input, audio, and often kernel-mode anti-cheat. A **process-level sandbox** with explicit capability grants is the practical starting point.

## Isolation stack

```
┌─────────────────────────────────────────────────────────────┐
│  gamecrate.exe CLI + GameCrate.Gui (WPF tray)                 │
├─────────────────────────────────────────────────────────────┤
│  Profile manager (JSON) — paths, capabilities, moniker      │
├─────────────────────────────────────────────────────────────┤
│  ACL manager — grants R/W/RX on install + save dirs only  │
├─────────────────────────────────────────────────────────────┤
│  AppContainer launcher — LPAC + capability SIDs           │
├─────────────────────────────────────────────────────────────┤
│  Windows kernel — token, DACL, object namespace isolation │
└─────────────────────────────────────────────────────────────┘
```

### Layer 1: AppContainer / LPAC

[AppContainers](https://learn.microsoft.com/en-us/windows/win32/secauthz/appcontainer-isolation) are the same mechanism UWP apps use. LPAC (`PROCESS_CREATION_ALL_APPLICATION_PACKAGES_OPT_OUT`) is stricter: no registry, COM, or filesystem access unless explicitly granted via capability SIDs or ACLs.

Each game profile maps to one AppContainer moniker (e.g. `GameCrate.MyGame.ABC123`).

### Layer 2: Filesystem ACLs

AppContainer processes are **denied by default** on paths that lack an ACE for their SID. GameCrate:

1. Derives the AppContainer SID from the profile moniker.
2. Applies **allow** ACEs on:
   - Game install root (read + execute; write only if the game patches itself)
   - Dedicated save/data directory under `%ProgramData%\GameCrate\<profile>\`
   - Optional extra paths declared in the profile (mods, shared assets)
3. Does **not** grant access to user Documents, Desktop, other game folders, etc.

System directories (`C:\Windows`, `C:\Program Files`) remain readable so games can load DLLs and shaders — same as a normal LPAC with default capabilities.

### Layer 3: Capability SIDs (optional per profile)

| Capability | When needed |
|---|---|
| `internetClient` | Online games, launchers |
| `privateNetworkClientServer` | LAN multiplayer |
| `registryRead` | Most Win32 games (LPAC blocks registry otherwise) |
| `lpacPnpNotifications` | GPU / DirectX 12 adapter enumeration (default for game profiles) |
| `lpacMedia` | Media and GPU-related LPAC paths (default for game profiles) |
| `lpacCom` | Games using COM (some launchers, codecs) |

Capabilities are declared in the profile JSON and passed to `CreateAppContainerProfile`.

### Layer 4: Virtualized storage (v0.3)

GameCrate redirects `%APPDATA%`, `%LOCALAPPDATA%`, and `%TEMP%` into `%ProgramData%\GameCrate\<id>\virtual\` via a custom environment block at launch/install. Install-time registry snapshots detect persistence keys in `HKCU`. See [VIRTUAL_STORAGE.md](VIRTUAL_STORAGE.md).

## Component layout

```
gamecrate.exe          CLI entry point
GameCrate.Gui.exe      WPF tray app — wraps CLI for install/play/report
├── ProfileStore     Load/save JSON profiles under %ProgramData%\GameCrate\profiles\
├── AclManager       Apply/remove filesystem ACEs for AppContainer SID
└── AppContainerLauncher
                     CreateAppContainerProfile → CreateProcess with LPAC attributes
```

PowerShell helpers in `tools/` handle bulk ACL setup and guided game installation.

## Lifecycle

### 1. Create profile

```powershell
gamecrate create-profile `
  --id hollow-knight `
  --name "Hollow Knight" `
  --install-dir "D:\Games\HollowKnight" `
  --executable "D:\Games\HollowKnight\hollow_knight.exe"
```

Creates `%ProgramData%\GameCrate\profiles\hollow-knight.json` and registers the AppContainer moniker.

### 2. Install game (sandboxed)

```powershell
gamecrate install `
  --id my-game `
  --name "My Game" `
  --install-dir "D:\Sandbox\MyGame" `
  --installer "D:\Downloads\setup.exe"
```

Runs the installer inside the LPAC. Files land only where the installer can write — which is only granted paths. After install, GameCrate scans the install tree and locks ACLs to that footprint. See [SANDBOXED_INSTALL.md](SANDBOXED_INSTALL.md).

### 3. Launch

```powershell
gamecrate launch --profile hollow-knight
```

Spawns the game's executable with LPAC + profile capabilities. Child processes inherit the container.

### 4. Teardown

```powershell
gamecrate destroy-profile --profile hollow-knight --wipe-data
```

Removes AppContainer registration, revokes ACL ACEs on granted paths, deletes the profile JSON, and optionally wipes `%ProgramData%\GameCrate\<id>\`. Does **not** delete game files in `installDir`.

## Threat model (summary)

**In scope — what GameCrate v1 protects against:**

- Game reading your Documents, SSH keys, browser profiles
- Game modifying files outside its install/save dirs
- Compromised game process accessing other users' data (AppContainer is per-user)

**Out of scope / known limits:**

- Kernel-mode anti-cheat (EAC, BattlEye, Vanguard) — requires ring-0 drivers incompatible with LPAC
- GPU driver attack surface — same as any Windows process
- Malicious games with granted `internetClient` can exfiltrate data they *can* read
- Determined escape via unpatched Windows sandbox bugs

See [THREAT_MODEL.md](THREAT_MODEL.md) for detail.

## Roadmap

| Phase | Deliverable |
|---|---|
| **v0.1** | LPAC launcher, JSON profiles, ACL grants, CLI |
| **v0.2** | Sandboxed installer + install footprint scanner |
| **v0.3** | AppData redirection + registry install scan + destroy-profile |
| **v0.4** | WPF tray GUI wrapping the CLI |
| v0.5 | Steam/Epic launcher integration |
| v1.0 | Optional kernel minifilter for deny-by-default on all volumes |

## References

- [Microsoft LaunchAppContainer](https://github.com/microsoft/SandboxSecurityTools/tree/main/LaunchAppContainer)
- [AppContainer isolation](https://learn.microsoft.com/en-us/windows/win32/secauthz/appcontainer-isolation)
- [Sandboxie-Plus](https://github.com/sandboxie-plus/Sandboxie) — mature driver-based alternative
