# GameCrate scope

GameCrate is a **Windows LPAC sandbox for standalone games** — installers and executables you manage yourself. It is **not** a replacement for Steam, Epic Games Store, GOG Galaxy, Xbox, or other platform launchers.

## In scope

| Use case | Example |
|----------|---------|
| Standalone `.exe` installers | Inno Setup, NSIS, repacks, itch.io downloads |
| Portable / folder-based games | Unzip and point a profile at `game.exe` |
| Mod-heavy or untrusted titles | Install monitored, play sandboxed |
| Per-game save isolation | Saves under `%LOCALAPPDATA%\GameCrate\<profile>\` |

**Typical flow:** download installer → GameCrate **Install** (monitored) → GameCrate **Play** (LPAC sandbox).

## Out of scope (by design)

| Use case | Why |
|----------|-----|
| **Steam / Epic / GOG / Xbox apps** | Those platforms own distribution, updates, DRM, cloud saves, friends, and anti-cheat policy. Use their clients. |
| **Launching `steam.exe` / `EpicGamesLauncher.exe`** | Singleton services, login, and store APIs are outside GameCrate’s model. |
| **Kernel anti-cheat** | EAC, BattlEye, Vanguard — incompatible with LPAC. |
| **DRM that requires store runtime** | Same as above — use the store. |

You *can* manually create a profile whose `installDir` points at a game folder under `steamapps\common\` and launch `Game.exe` directly, but GameCrate will **not** integrate with or automate store launchers. That remains an advanced, unsupported edge case.

## Two phases (important)

| Phase | How it runs | Purpose |
|-------|-------------|---------|
| **Install** | Monitored child process (UAC allowed) | Real installers (Inno, etc.) can run |
| **Play** | AppContainer LPAC sandbox | Game only sees granted install + save paths |

Install is **not** fully LPAC-sandboxed — footprint scanning and install reports catch bad behavior instead.

## Data locations

| What | Where |
|------|--------|
| Game files | **You choose** — e.g. `D:\Games\my-game` (not limited to AppData) |
| GameCrate profiles, saves, reports | `%LOCALAPPDATA%\GameCrate\` (automatic) |

## Who should use GameCrate

- Testers evaluating sandboxed installs on Windows VMs
- Players with standalone/repack/offline installers
- Anyone who wants one game isolated from the rest of the profile

## Who should not expect GameCrate to help

- “Sandbox my entire Steam library” — use Steam + its own tooling, or a driver sandbox (e.g. Sandboxie)
- Online competitive titles with kernel anti-cheat
- GPU-less VMs (install may work; **Play** often will not)

## Related docs

- [TESTING.md](TESTING.md) — hand to testers
- [GAME_COMPATIBILITY.md](GAME_COMPATIBILITY.md) — tiers and failure symptoms
- [SANDBOXED_INSTALL.md](SANDBOXED_INSTALL.md) — install monitoring details
- [ARCHITECTURE.md](ARCHITECTURE.md) — technical design
