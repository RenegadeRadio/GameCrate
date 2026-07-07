# GameCrate testing guide

Use this guide when handing GameCrate to testers. It covers download, install, play, path choices, and what feedback to collect.

## Download

1. Open [GitHub Releases](https://github.com/RenegadeRadio/GameCrate/releases)
2. Download **`GameCrate-windows-x64.zip`** from the latest release (currently **v0.4.7+**)
3. Extract the `GameCrate` folder anywhere (e.g. `C:\Tools\GameCrate`)
4. Run **`GameCrate.Gui.exe`** as a **normal user** — do not “Run as administrator” unless Windows/UAC asks during the game installer

## Where files go (two different things)

### Game install folder — **you choose this**

The game files can go in **any folder you own**, as long as it is **not** directly on the drive root and **not** under `Program Files` or `Windows`.

| Good examples | Avoid |
|---------------|--------|
| `D:\Games\my-game` | `C:\Install` (drive root) |
| `C:\Users\You\Games\my-game` | `C:\Program Files\...` |
| `%LOCALAPPDATA%\Games\my-game` | `C:\Windows\...` |

`%LOCALAPPDATA%\Games\...` is only a convenient suggestion — **`D:\Games\...` works the same.**

In the GUI install wizard, click **Browse** and pick or create an **empty** folder.

### GameCrate data — **automatic (not configurable in GUI)**

GameCrate always stores profiles, saves, virtual AppData, and install reports here:

```
%LOCALAPPDATA%\GameCrate\
├── profiles\              ← one JSON file per game profile
└── <profile-id>\
    ├── saves\
    ├── virtual\           ← redirected AppData during play
    └── install-report.json
```

Example: `C:\Users\You\AppData\Local\GameCrate\`

Advanced: override save location with CLI `--save-dir` on `install` / `create-profile`.

## Install a game (GUI)

1. Open **GameCrate.Gui.exe**
2. Click **Install game...**
3. Fill in:
   - **Display name** — e.g. `My Game`
   - **Profile ID** — lowercase, e.g. `my-game`
   - **Install directory** — empty folder you own (see table above)
   - **Installer** — path to the `.exe` setup file
4. Leave **Virtualize AppData** checked (recommended)
5. Click **Install**
6. **Approve UAC** if Windows prompts — many installers (Inno Setup, repacks) need this once
7. When finished, select the profile and click **Install report** to review what the installer touched

### Install notes

- Install runs **monitored** (not inside LPAC) so UAC and common installers work; **Play** still uses the LPAC sandbox
- Start Menu shortcuts and Windows icon caches are normal and should **not** fail install (v0.4.6+)
- Optional: check **Strict: fail if installer writes outside install folder** only if you want hard failures on any outside write

## Play a game

1. Select the profile in the main window
2. Click **Play** (or double-click the profile)

**Requirements:** a real GPU or working VM 3D acceleration. A VM with no GPU may **install successfully** but **fail on Play** — that is expected for 3D/DirectX games.

## CLI quick checks

```powershell
cd C:\Tools\GameCrate

.\gamecrate.exe list-profiles
.\gamecrate.exe show-profile --profile my-game
.\gamecrate.exe launch --profile my-game
```

## What to report back

Please include:

| Item | Example |
|------|---------|
| GameCrate version | v0.4.7 |
| Windows version | Windows 11 23H2 |
| Install folder used | `D:\Games\my-game` |
| Installer type | Inno / NSIS / portable / other (not Steam/Epic store clients) |
| Install result | Success / failed (screenshot or error text) |
| Play result | Success / failed (screenshot or error text) |
| GPU / VM | Physical PC / VM with 3D accel / VM no GPU |
| Install report | `%LOCALAPPDATA%\GameCrate\<profile-id>\install-report.json` |

## Common issues

| Symptom | Likely cause | What to try |
|---------|--------------|-------------|
| JSON error on startup | Old build | Use **v0.4.3+** |
| Access denied on install folder | `C:\Install`, Program Files | Use `D:\Games\...` or `%LOCALAPPDATA%\Games\...` |
| Inno / `C:\Windows\is-*.tmp` error | Old build or LPAC install | Use **v0.4.5+**; approve UAC |
| Install failed: outside writes | Old build | Use **v0.4.7+**; Start Menu shortcuts are benign |
| Play fails, install OK | No GPU in VM | Expected for many games; test on hardware with a GPU |
| UAC prompt during install | Normal for many installers | Click Yes |

## Related docs

- [SCOPE.md](SCOPE.md) — standalone games only; not store launchers
- [GUI.md](GUI.md) — tray app and troubleshooting
- [CLI.md](CLI.md) — full command reference
- [GAME_COMPATIBILITY.md](GAME_COMPATIBILITY.md) — which games work in LPAC
- [SANDBOXED_INSTALL.md](SANDBOXED_INSTALL.md) — install monitoring details
