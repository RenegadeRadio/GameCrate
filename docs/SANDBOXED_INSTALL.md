# Sandboxed Install (v0.2)

GameCrate v0.2 adds **`gamecrate install`** — run an untrusted installer inside LPAC, scan what it wrote, and block persistence outside the sandbox.

## Command

```powershell
gamecrate install `
  --id my-game `
  --name "My Game" `
  --install-dir "D:\Sandbox\MyGame" `
  --installer "D:\Downloads\setup.exe"
```

Or use the helper:

```powershell
.\tools\Install-GameSandbox.ps1 -Id my-game -Name "My Game" `
  -InstallDir "D:\Sandbox\MyGame" -Installer "D:\Downloads\setup.exe" -Launch
```

## What happens

```mermaid
sequenceDiagram
    participant User
    participant GameCrate
    participant LPAC as Windows LPAC
    participant FS as Filesystem

    User->>GameCrate: install --installer setup.exe
    GameCrate->>FS: Snapshot allowed + watch paths
    GameCrate->>LPAC: Launch installer (network off by default)
    LPAC->>FS: Writes only where ACLs allow
    LPAC-->>GameCrate: Installer exits
    GameCrate->>FS: Snapshot again + diff
    GameCrate->>GameCrate: Detect game .exe, tighten ACLs
    GameCrate-->>User: Report + profile ready
```

### 1. Install-phase ACLs

The install directory gets **read/write/execute** so the installer can extract files. Save dir and `%ProgramData%\GameCrate\<id>\` are also writable.

### 2. Footprint snapshots

**Allowed roots** (writes expected):

- `--install-dir`
- Save directory
- `%ProgramData%\GameCrate\<id>\`

**Watch paths** (writes flagged as violations):

- Startup folders
- `%APPDATA%`, `%LOCALAPPDATA%`
- Desktop, Documents
- Start Menu / Programs
- `%ProgramData%` (outside the GameCrate profile subtree)

### 3. Sandboxed installer launch

The installer runs in LPAC with **network disabled by default** to reduce install-time phone-home. Use `--network` only if the installer requires it.

### 4. Post-install analysis

- Lists new/modified files under allowed roots
- Flags any new files under watch paths
- Marks **suspicious** paths (Startup, System32, Tasks, etc.)
- Auto-detects the game `.exe` (skips `setup.exe`, `unins*.exe`, etc.)
- Writes `%ProgramData%\GameCrate\<id>\install-report.json`

### 5. ACL tightening

By default, install dir ACLs are tightened to **read/execute** after a successful install so the game can run but not self-modify. Use `--keep-install-writable` to skip this (e.g. for games that patch themselves).

## Options

| Flag | Default | Purpose |
|---|---|---|
| `--installer-args` | empty | Arguments for the installer |
| `--executable` | auto-detect | Skip executable detection |
| `--allow-outside-writes` | off | Warn but don't fail on outside writes |
| `--keep-install-writable` | off | Keep install dir writable after install |
| `--network` | off | Allow network during install |

## Exit codes

| Code | Meaning |
|---|---|
| `0` | Install succeeded, no outside writes, executable detected |
| `1` | Installer failed, outside writes detected, or no executable found |

## Viewing the report

```powershell
gamecrate show-install-report --profile my-game
```

## Malware-focused workflow

1. **Never** install untrusted games outside GameCrate first
2. Run `gamecrate install` with network **off**
3. Review `install-report.json` — `outsideWrites` should be empty
4. If clean, enable `--network` on the profile later if the game needs online play:
   ```powershell
   # Edit profile JSON: "network": true, then:
   gamecrate grant --profile my-game
   ```
5. Launch: `gamecrate launch --profile my-game`

## Limitations (v0.3)

- Registry writes are **detected**, not fully redirected — review `outsideRegistryChanges` in install reports
- Watch-path filesystem scans can take time on large `%LOCALAPPDATA%` trees
- Installers that require admin elevation outside the sandbox may fail
- Some installers spawn external processes that are not sandboxed

See [VIRTUAL_STORAGE.md](VIRTUAL_STORAGE.md) for AppData layout and registry scanning details.
