# Install GameCrate with winget

GameCrate ships as a **portable zip** (`GameCrate-windows-x64.zip`). The winget manifest in this repo extracts the archive and registers:

| Command | Executable |
|---------|------------|
| `gamecrate` | CLI (`gamecrate.exe`) |
| `gamecrate-gui` | Tray GUI (`GameCrate.Gui.exe`) |

Both stay in the same folder (profiles, tools, and .NET runtime DLLs must remain alongside the exes).

## Install

Manifests live under [`winget/manifests/r/RenegadeRadio/GameCrate/`](../winget/manifests/r/RenegadeRadio/GameCrate/).

```powershell
git clone https://github.com/RenegadeRadio/GameCrate.git
cd GameCrate
winget install --manifest .\winget\manifests\r\RenegadeRadio\GameCrate\0.4.8
```

You can also download only the manifest folder from GitHub and point winget at the local path.

## New releases

After `GameCrate-windows-x64.zip` is published to GitHub Releases:

```powershell
.\tools\update-winget-manifest.ps1 -Version 0.4.8
# or with a local zip from CI:
.\tools\update-winget-manifest.ps1 -Version 0.4.9 -ZipPath .\GameCrate-windows-x64.zip
winget validate --manifest .\winget\manifests\r\RenegadeRadio\GameCrate\0.4.9
```

Commit the new version folder and share that path with testers.

## Requirements

- Windows 10 version 1809+ (build 17763) or Windows 11 x64
- [App Installer](https://apps.microsoft.com/detail/9NBLGGH4NNS1) / winget 1.5+ (zip portable support)
- No code signing required (unsigned binaries may show SmartScreen on first run — same as manual zip download)

## Uninstall

```powershell
winget uninstall RenegadeRadio.GameCrate
```

Game profiles and data under `%LOCALAPPDATA%\GameCrate\` are **not** removed automatically.

## Related

- [TESTING.md](TESTING.md) — manual zip install and tester guide
- [SCOPE.md](SCOPE.md) — standalone games only
