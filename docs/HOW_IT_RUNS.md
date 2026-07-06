# How WinDoze Runs

WinDoze is **not** a background service, a VM, or a driver that wraps every app on your PC. It is a **launcher layer** — a small orchestrator that tells Windows to start a game inside an existing kernel security feature (LPAC).

## v0.1: single CLI executable

Today WinDoze ships as one file:

```
windoze.exe
```

There is no installer wizard, no system tray icon, and no always-on daemon. You run it from PowerShell or Command Prompt when you want to create a profile or launch a game.

```
You  →  windoze.exe launch --profile my-game  →  game.exe (sandboxed)
         ↑ exits when game exits (default)        ↑ runs until you close it
```

### What happens on `launch`

1. **Read profile** — JSON from `%ProgramData%\WinDoze\profiles\<id>.json`
2. **Apply ACLs** — add filesystem allow rules for the game's AppContainer SID
3. **CreateProcess (LPAC)** — spawn `game.exe` with a restricted token + capability SIDs
4. **Wait** — `windoze.exe` blocks until the game exits (unless `--no-wait`)
5. **Exit** — launcher returns; no residue except profile data and save files

The game window you see **is** the real game — WinDoze does not embed or stream it. It only sets up the sandbox, then gets out of the way.

## Application layer vs. container runtime

Think of the split like this:

| Piece | Role |
|---|---|
| **windoze.exe** | User-facing launcher — profiles, ACL setup, spawn game |
| **Windows LPAC** | Kernel-enforced sandbox (token, capabilities, object namespace) |
| **NTFS ACLs** | Filesystem allow-list for install + save paths |
| **game.exe** | The actual application you interact with |

WinDoze is the **control plane**. Windows is the **runtime**.

```
┌─────────────────────────────────────────┐
│  You (PowerShell, shortcuts, future GUI) │
└──────────────────┬──────────────────────┘
                   │ windoze.exe commands
┌──────────────────▼──────────────────────┐
│  WinDoze launcher (control plane)        │
│  • profiles  • ACL grants  • spawn       │
└──────────────────┬──────────────────────┘
                   │ CreateProcess + LPAC
┌──────────────────▼──────────────────────┐
│  Windows security (runtime)              │
│  • AppContainer token  • capabilities    │
│  • DACL enforcement  • object isolation  │
└──────────────────┬──────────────────────┘
                   │
            ┌──────▼──────┐
            │  game.exe   │  ← you play here
            └─────────────┘
```

This is closer to **"run this exe in a sandbox"** than to Docker Desktop running in the background.

## What gets installed on disk

| Location | Contents |
|---|---|
| `windoze.exe` | The launcher (you choose where — e.g. `C:\Tools\WinDoze\`) |
| `%ProgramData%\WinDoze\profiles\` | One JSON file per game profile |
| `%ProgramData%\WinDoze\<id>\saves\` | Isolated save data per profile |
| Your `installDir` | Game files (e.g. `D:\Sandbox\MyGame\`) |

No kernel driver. No Windows service. Uninstall = delete those folders + `windoze.exe`.

## Typical workflows

### One-off launch (manual)

```powershell
windoze launch --profile hollow-knight
```

### Shortcut (recommended for daily use)

Create a desktop shortcut:

- **Target:** `C:\Tools\WinDoze\windoze.exe launch --profile hollow-knight --no-wait`
- **Start in:** `C:\Tools\WinDoze`

Double-clicking the shortcut opens the game sandboxed. WinDoze exits immediately (`--no-wait`); the game keeps running.

### Helper script

```powershell
.\tools\New-GameSandbox.ps1 -Id my-game -Name "My Game" -InstallDir D:\Sandbox\MyGame -Launch
```

## What WinDoze is **not** (yet)

| Future piece | Purpose |
|---|---|
| **GUI app** (v0.4) | Point-and-click profile manager, "Play" buttons |
| **Windows service** | Auto-apply ACLs on boot, central policy |
| **Shell integration** | Right-click → "Run with WinDoze" |
| **Kernel minifilter** (v1.0) | Deny-by-default on all volumes (Sandboxie-style) |

v0.1 is intentionally minimal: one executable, explicit commands, no hidden background layer.

## GPU and capabilities

Game profiles default to `gpu: true`, which grants:

- `lpacPnpNotifications` — DXGI adapter enumeration (needed for many DirectX 12 titles)
- `lpacMedia` — media/GPU-related LPAC access

These are capability SIDs on the sandbox **token**, not a separate GPU driver. The game still talks to your normal graphics stack.

Disable with `--no-gpu` on `create-profile` if you are testing a non-graphical tool.
