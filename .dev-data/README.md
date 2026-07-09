# Dev data (local only)

This folder is **gitignored**. When you run `tools/Enter-GameCrateDev.ps1`, GameCrate stores profiles, saves, install reports, and virtual AppData here instead of `%LOCALAPPDATA%\GameCrate\`.

| Subfolder | Purpose |
|-----------|---------|
| `profiles/` | Profile JSON files |
| `<profile-id>/` | Saves, virtual AppData, install reports per game |
| `games/` | Suggested install target for test games during development |

Delete this entire `.dev-data` folder to reset local dev state.
