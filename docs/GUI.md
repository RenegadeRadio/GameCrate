# GameCrate GUI (v0.4)

GameCrate ships a WPF tray application that wraps the `gamecrate.exe` CLI. Use it to install games in a sandbox, launch them, and review install reports without memorizing command-line flags.

## Requirements

- Windows 10 1809+ or Windows 11 (x64)
- [.NET 8 Desktop Runtime](https://dotnet.microsoft.com/download/dotnet/8.0) (framework-dependent build)
- `gamecrate.exe` in the same folder as `GameCrate.Gui.exe`, or on your `PATH`

## Running

From a CI artifact or local build:

```powershell
cd build\package
.\GameCrate.Gui.exe
```

The app minimizes to the system tray. Double-click the tray icon or choose **Open GameCrate** from the context menu.

## Features

| Action | Description |
|---|---|
| **Install game** | Wizard for sandboxed installer run + footprint scan |
| **Play** | Launch the selected profile (`gamecrate launch --no-wait`) |
| **Install report** | Opens the JSON report from the last sandboxed install |
| **Remove profile** | Deletes the profile and wipes sandbox data (`destroy-profile --wipe-data`) |
| **Refresh** | Reloads the profile list from disk |

## Install wizard options

- **Virtualize AppData (recommended)** — redirects `%APPDATA%`, `%LOCALAPPDATA%`, and `%TEMP%` into the sandbox. Uncheck only if an installer genuinely needs real AppData paths.
- **Allow network during install** — grants network to the installer process. Off by default for malware isolation.

## How it works

The GUI does not reimplement sandbox logic. `GameCrateService` spawns `gamecrate.exe` as a child process and parses JSON from `list-profiles --json`. Keep `gamecrate.exe` next to the GUI binary so both stay in sync.

## Build locally

```powershell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

dotnet publish src/gui/GameCrate.Gui/GameCrate.Gui.csproj -c Release -r win-x64 --self-contained false -o build/package
Copy-Item build\gamecrate.exe build\package\gamecrate.exe
```

## Troubleshooting

**"gamecrate.exe was not found"** — copy `gamecrate.exe` into the same directory as `GameCrate.Gui.exe`.

**Install hangs** — some installers need elevation or network. Check the install report after completion; re-run from the CLI with `--network` if the game requires online activation during setup.

**Game won't start from GUI** — verify the profile with `gamecrate show-profile --profile <id>`. Anti-cheat and some DRM titles cannot run inside LPAC.
