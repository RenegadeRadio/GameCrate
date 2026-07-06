# GameCrate GUI (v0.4)

GameCrate ships a WPF tray application that wraps the `gamecrate.exe` CLI. Use it to install games in a sandbox, launch them, and review install reports without memorizing command-line flags.

## Requirements

- Windows 10 1809+ or Windows 11 (x64)
- [.NET 8 Desktop Runtime](https://dotnet.microsoft.com/download/dotnet/8.0) — only if you build the GUI yourself with `--self-contained false`
- **GitHub Releases** builds are self-contained; no separate .NET install needed
- `gamecrate.exe` in the same folder as `GameCrate.Gui.exe`, or on your `PATH`

## Running

From a [GitHub Release](https://github.com/RenegadeRadio/GameCrate/releases) or local build:

```powershell
cd C:\Tools\GameCrate
.\GameCrate.Gui.exe
```

On startup the main window opens and a **system tray icon** appears. The app runs until you choose **Exit** from the tray menu or close the main window.

| Tray action | Behavior |
|---|---|
| Double-click icon | Open / focus main window |
| **Open GameCrate** | Open / focus main window |
| **Install game...** | Open main window and install wizard |
| **Exit** | Quit the application |

Closing the main window exits the app (there is no hide-to-tray-only mode in v0.4).

## Features

| Action | CLI equivalent |
|---|---|
| **Install game** | `gamecrate install ...` |
| **Play** | `gamecrate launch --profile <id> --no-wait` |
| **Install report** | Opens `%ProgramData%\GameCrate\<id>\install-report.json` |
| **Remove profile** | `gamecrate destroy-profile --profile <id> --wipe-data` |
| **Refresh** | `gamecrate list-profiles --json` |

**Not in the GUI (use CLI):** `create-profile` for already-installed games, `grant`, editing `readablePaths` / `writablePaths`.

## Install wizard options

- **Virtualize AppData (recommended)** — redirects `%APPDATA%`, `%LOCALAPPDATA%`, and `%TEMP%` into the sandbox. Uncheck only if an installer genuinely needs real AppData paths.
- **Allow network during install** — grants network to the installer process. Off by default for malware isolation.

Profile IDs must match the schema: lowercase letters, numbers, and hyphens (`[a-z0-9-]+`).

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

**Remove profile** — deletes the profile, revokes ACLs, and wipes `%ProgramData%\GameCrate\<id>\`. Game files in the install directory are **not** deleted.

See also [CLI.md](CLI.md) and [HOW_IT_RUNS.md](HOW_IT_RUNS.md).
