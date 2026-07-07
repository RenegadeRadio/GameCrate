# GameCrate CLI reference

Binary: **`gamecrate.exe`**. Run `gamecrate` with no arguments to print usage.

Profiles are stored at `%LOCALAPPDATA%\GameCrate\profiles\<id>.json`. Per-profile data lives under `%LOCALAPPDATA%\GameCrate\<id>\`.

## Commands

### `install`

Run an installer inside LPAC, scan filesystem and registry footprint, create a profile.

```powershell
gamecrate install `
  --id my-game `
  --name "My Game" `
  --install-dir "D:\Sandbox\MyGame" `
  --installer "D:\Downloads\setup.exe"
```

| Flag | Default | Description |
|---|---|---|
| `--id` | *(required)* | Profile identifier (`[a-z0-9-]+`) |
| `--name` | *(required)* | Display name |
| `--install-dir` | *(required)* | Empty or new install folder |
| `--installer` | *(required)* | Path to installer `.exe` |
| `--installer-args` | empty | Arguments passed to the installer |
| `--executable` | auto-detect | Game `.exe` after install |
| `--save-dir` | `%LOCALAPPDATA%\GameCrate\<id>\saves` | Isolated save directory |
| `--allow-outside-writes` | on | Do not fail on benign outside writes (Start Menu shortcuts, icon caches) |
| `--strict-outside-writes` | off | Fail if installer writes outside install/save dirs |
| `--keep-install-writable` | off | Skip post-install ACL tightening |
| `--network` | off | Allow network during install |
| `--no-registry` | off | Do not grant `registryRead` |
| `--lpac-com` | off | Grant COM capability |
| `--no-gpu` | off | Disable GPU LPAC capabilities |
| `--no-virtual-app-data` | off | Do not redirect AppData (not recommended) |

See [SANDBOXED_INSTALL.md](SANDBOXED_INSTALL.md).

**Exit codes:** `0` = success; `1` = installer failed, outside writes detected, or no executable found.

---

### `create-profile`

Register a profile for an already-installed game.

```powershell
gamecrate create-profile `
  --id hollow-knight `
  --name "Hollow Knight" `
  --install-dir "D:\Games\HollowKnight" `
  --executable "D:\Games\HollowKnight\hollow_knight.exe"
```

| Flag | Default | Description |
|---|---|---|
| `--id`, `--name`, `--install-dir`, `--executable` | *(required)* | Profile identity and paths |
| `--save-dir` | `%LOCALAPPDATA%\GameCrate\<id>\saves` | Save directory |
| `--network` | off | `internetClient` + `privateNetworkClientServer` |
| `--no-registry` | off | Stricter; breaks many games |
| `--lpac-com` | off | COM capability |
| `--no-gpu` | off | Disable GPU capabilities |
| `--no-virtual-app-data` | off | Disable AppData redirection |

After editing profile JSON manually (e.g. `readablePaths`), run `gamecrate grant --profile <id>`.

---

### `launch`

```powershell
gamecrate launch --profile my-game [--no-wait]
```

| Flag | Description |
|---|---|
| `--profile` | Profile ID |
| `--no-wait` | Return immediately after spawning the game |

---

### `grant`

Re-apply filesystem ACL grants for a profile (after path or JSON changes).

```powershell
gamecrate grant --profile my-game
```

---

### `show-install-report`

Print install footprint JSON to stdout.

```powershell
gamecrate show-install-report --profile my-game
```

Report file: `%LOCALAPPDATA%\GameCrate\<id>\install-report.json`

#### Install report fields

| Field | Type | Meaning |
|---|---|---|
| `profileId` | string | Profile ID |
| `installer` | string | Installer path used |
| `installerExitCode` | number | Installer process exit code |
| `executable` | string | Detected or specified game `.exe` |
| `installedFileCount` | number | Files under allowed roots |
| `outsideWriteCount` | number | Writes outside sandbox |
| `suspiciousOutsideWriteCount` | number | Writes to high-risk paths |
| `registryChangeCount` | number | All registry diffs detected |
| `outsideRegistryChangeCount` | number | Registry changes outside virtual store |
| `suspiciousRegistryChangeCount` | number | Run/RunOnce persistence keys |
| `installedFiles` | string[] | Paths under allowed roots |
| `outsideWrites` | string[] | Flagged filesystem paths |
| `suspiciousOutsideWrites` | string[] | Startup, System32, etc. |
| `registryChanges` | string[] | All registry value changes |
| `outsideRegistryChanges` | string[] | Changes treated as outside sandbox |
| `suspiciousRegistryChanges` | string[] | Persistence-related keys |

For malware isolation, `outsideWrites` and `outsideRegistryChanges` should be empty.

---

### `set-executable`

Set or fix the game executable after install when auto-detection fails.

```powershell
gamecrate set-executable --profile my-game --executable D:\Games\my-game\Game.exe
```

The GUI install wizard offers a file picker automatically when install reports *no game executable was detected*.

---

### `destroy-profile`

```powershell
gamecrate destroy-profile --profile my-game [--wipe-data]
```

| Flag | Description |
|---|---|
| `--profile` | Profile ID to remove |
| `--wipe-data` | Delete `%LOCALAPPDATA%\GameCrate\<id>\` (saves, virtual AppData, reports) |

**What is removed:** AppContainer registration, profile JSON, ACL ACEs on granted paths (when the moniker resolves).

**What is not removed:** Game files in `--install-dir`. Delete those manually if desired.

---

### `list-profiles`

```powershell
gamecrate list-profiles [--json]
```

Human output: one line per profile (`id — name (installDir)`).

#### `--json` output

Array of profile objects (same fields as `profiles/schema.json`):

```json
[
  {
    "id": "my-game",
    "name": "My Game",
    "moniker": "GameCrate.my-game",
    "installDir": "D:\\Sandbox\\MyGame",
    "executable": "D:\\Sandbox\\MyGame\\game.exe",
    "arguments": "",
    "saveDir": "C:\\Users\\you\\AppData\\Local\\GameCrate\\my-game\\saves",
    "writablePaths": ["D:\\Sandbox\\MyGame", "C:\\Users\\you\\AppData\\Local\\GameCrate\\my-game\\saves"],
    "readablePaths": [],
    "network": false,
    "registryRead": true,
    "lpacCom": false,
    "gpu": true,
    "virtualizeAppData": true
  }
]
```

Used by `GameCrate.Gui.exe` to populate the profile list.

---

### `show-profile`

```powershell
gamecrate show-profile --profile my-game
```

Prints key profile fields to stdout. Does not list `writablePaths`, `readablePaths`, or `arguments` — read the JSON file directly for full detail.

---

## PowerShell helpers

| Script | Purpose |
|---|---|
| `tools\build.ps1` | Build `gamecrate.exe` with CMake + VS 2022 |
| `tools\New-GameSandbox.ps1` | `create-profile` + `grant` (+ optional launch) |
| `tools\Install-GameSandbox.ps1` | Sandboxed `install` + show report |
| `tools\Grant-ProfileAcls.ps1` | Re-run `grant` for one profile |

## Build output paths

| Generator | CLI path |
|---|---|
| Visual Studio (`tools\build.ps1`) | `build\Release\gamecrate.exe` |
| Ninja (CI, manual) | `build\gamecrate.exe` |
| CI GUI package | `build\package\GameCrate.Gui.exe` + `build\package\gamecrate.exe` |
