# Install GameCrate with winget

GameCrate ships as a **portable zip** (`GameCrate-windows-x64.zip`). The winget manifest extracts the archive and registers:

| Command | Executable |
|---------|------------|
| `gamecrate` | CLI (`gamecrate.exe`) |
| `gamecrate-gui` | Tray GUI (`GameCrate.Gui.exe`) |

Both stay in the same folder (profiles, tools, and .NET runtime DLLs must remain alongside the exes).

## Option A — Microsoft community repository (recommended once listed)

After [microsoft/winget-pkgs](https://github.com/microsoft/winget-pkgs) accepts the package:

```powershell
winget install RenegadeRadio.GameCrate
```

Search: `winget search gamecrate`

## Option B — Install from this repository (available now)

Manifests live under [`winget/manifests/r/RenegadeRadio/GameCrate/`](../winget/manifests/r/RenegadeRadio/GameCrate/).

```powershell
git clone https://github.com/RenegadeRadio/GameCrate.git
cd GameCrate
winget install --manifest .\winget\manifests\r\RenegadeRadio\GameCrate\0.4.8
```

Or download only the manifest folder from GitHub and point winget at the local path.

## Option C — Submit / update winget-pkgs

Maintainers: copy the version folder into a fork of [winget-pkgs](https://github.com/microsoft/winget-pkgs) at:

```
manifests/r/RenegadeRadio/GameCrate/<version>/
```

Open a PR using the [submission checklist](https://github.com/microsoft/winget-pkgs/blob/master/CONTRIBUTING.md).

### Regenerate manifests for a new release

On Windows, after `GameCrate-windows-x64.zip` is published:

```powershell
.\tools\update-winget-manifest.ps1 -Version 0.4.8
# or with a local zip from CI:
.\tools\update-winget-manifest.ps1 -Version 0.4.9 -ZipPath .\GameCrate-windows-x64.zip
winget validate --manifest .\winget\manifests\r\RenegadeRadio\GameCrate\0.4.9
```

Commit the new version folder and open a winget-pkgs PR.

### Optional: automated winget-pkgs PR on release

Add a repository secret `WINGET_TOKEN` (PAT with `public_repo` scope) and enable the workflow in `.github/workflows/winget-publish.yml`. It uses [winget-releaser](https://github.com/vedantmg0009/winget-releaser) to open a PR when a GitHub Release is published.

## Requirements

- Windows 10 version 1809+ (build 17763) or Windows 11 x64
- [App Installer](https://apps.microsoft.com/detail/9NBLGGH4NNS1) / winget 1.5+ (zip portable support)
- No code signing required for zip-based portable install (unsigned binaries may show SmartScreen on first run — same as manual zip download)

## Uninstall

```powershell
winget uninstall RenegadeRadio.GameCrate
```

Game profiles and data under `%LOCALAPPDATA%\GameCrate\` are **not** removed automatically.

## Related

- [TESTING.md](TESTING.md) — manual zip install and tester guide
- [SCOPE.md](SCOPE.md) — standalone games only
