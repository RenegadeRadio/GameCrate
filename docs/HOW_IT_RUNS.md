# How GameCrate Runs

GameCrate is **not** a background service, a VM, or a driver that wraps every app on your PC. It is a **launcher layer** — a small orchestrator that tells Windows to start a game inside an existing kernel security feature (LPAC).

## v0.4: CLI + tray GUI

GameCrate ships two user-facing entry points:

```
gamecrate.exe          CLI — scripting, automation, power users
GameCrate.Gui.exe      WPF tray app — install wizard, Play, install reports
```

The GUI wraps the CLI. It does not duplicate sandbox logic. See [GUI.md](GUI.md).

```
You  →  GameCrate.Gui.exe  →  gamecrate.exe launch --profile my-game  →  game.exe (sandboxed)
```

There is no always-on daemon. The tray icon is available while the GUI process is running. Closing the main window or choosing **Exit** from the tray menu quits the app.

### CLI-only usage

You can still run everything from PowerShell or Command Prompt:

```
You  →  gamecrate.exe launch --profile my-game  →  game.exe (sandboxed)
         ↑ exits when game exits (default)        ↑ runs until you close it
```

### What happens on `launch`

1. **Read profile** — JSON from `%ProgramData%\GameCrate\profiles\<id>.json`
2. **Apply ACLs** — add filesystem allow rules for the game's AppContainer SID
3. **CreateProcess (LPAC)** — spawn `game.exe` with a restricted token + capability SIDs
4. **Wait** — `gamecrate.exe` blocks until the game exits (unless `--no-wait`)
5. **Exit** — launcher returns; no residue except profile data and save files

The game window you see **is** the real game — GameCrate does not embed or stream it. It only sets up the sandbox, then gets out of the way.

## Application layer vs. container runtime

Think of the split like this:

| Piece | Role |
|---|---|
| **gamecrate.exe** | CLI launcher — profiles, ACL setup, spawn game |
| **GameCrate.Gui.exe** | Tray GUI — wraps the CLI for install/play/report |
| **Windows LPAC** | Kernel-enforced sandbox (token, capabilities, object namespace) |
| **NTFS ACLs** | Filesystem allow-list for install + save paths |
| **game.exe** | The actual application you interact with |

GameCrate is the **control plane**. Windows is the **runtime**.

```
┌─────────────────────────────────────────┐
│  You (GUI, PowerShell, shortcuts)        │
└──────────────────┬──────────────────────┘
                   │ gamecrate.exe commands
┌──────────────────▼──────────────────────┐
│  GameCrate launcher (control plane)        │
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
| `gamecrate.exe` | CLI launcher — profiles, ACL setup, spawn game |
| `GameCrate.Gui.exe` | Tray GUI — wraps CLI for install/play/report |
| `%ProgramData%\GameCrate\profiles\` | One JSON file per game profile |
| `%ProgramData%\GameCrate\<id>\saves\` | Isolated save data per profile |
| Your `installDir` | Game files (e.g. `D:\Sandbox\MyGame\`) |

No kernel driver. No Windows service. Uninstall = delete those folders + `gamecrate.exe`.

## Typical workflows

### One-off launch (manual)

```powershell
gamecrate launch --profile hollow-knight
```

### Shortcut (recommended for daily use)

Create a desktop shortcut:

- **Target:** `C:\Tools\GameCrate\gamecrate.exe launch --profile hollow-knight --no-wait`
- **Start in:** `C:\Tools\GameCrate`

Double-clicking the shortcut opens the game sandboxed. GameCrate exits immediately (`--no-wait`); the game keeps running.

### Helper script

```powershell
.\tools\New-GameSandbox.ps1 -Id my-game -Name "My Game" -InstallDir D:\Sandbox\MyGame -Launch
```

## What GameCrate is **not** (yet)

| Future piece | Purpose |
|---|---|
| **Windows service** | Auto-apply ACLs on boot, central policy |
| **Shell integration** | Right-click → "Run with GameCrate" |
| **Steam/Epic integration** (v0.5) | Launcher-aware profile creation |
| **Kernel minifilter** (v1.0) | Deny-by-default on all volumes (Sandboxie-style) |

v0.4 keeps the control plane explicit: CLI + optional GUI tray, no hidden background layer.

## GPU and capabilities

Game profiles default to `gpu: true`, which grants:

- `lpacPnpNotifications` — DXGI adapter enumeration (needed for many DirectX 12 titles)
- `lpacMedia` — media/GPU-related LPAC access

These are capability SIDs on the sandbox **token**, not a separate GPU driver. The game still talks to your normal graphics stack.

Disable with `--no-gpu` on `create-profile` if you are testing a non-graphical tool.
