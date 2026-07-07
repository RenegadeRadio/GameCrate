# Game Compatibility Guide

GameCrate uses LPAC sandboxing. Not every game will work without profile tuning. Use this guide to classify titles and adjust profiles.

## Compatibility tiers

### Tier A — Likely works out of the box

- Offline indie games (single EXE or small folder)
- Older Win32 titles without kernel anti-cheat
- Games installed to a dedicated directory with saves beside the EXE

**Profile template:** `profiles/examples/offline-indie.json`

### Tier B — Needs capability tuning

- Games that read registry for settings (enable `registryRead` — default on)
- DirectX 12 / black screen on launch (`gpu: true` — default on; grants `lpacPnpNotifications` + `lpacMedia`)
- LAN multiplayer (add `privateNetworkClientServer` via `--network`)
- Online multiplayer (add `internetClient`)
- Games using COM codecs or shell integration (add `lpacCom`)

### Tier C — Problematic / out of scope

- **Steam / Epic / GOG / Xbox** — use those platform clients; GameCrate does not integrate with store launchers ([SCOPE.md](SCOPE.md))
- Games with self-updaters — need write access to install dir
- Titles using EasyAntiCheat, BattlEye, Vanguard — **will not work** in GameCrate

### Tier D — Unsupported

- Kernel anti-cheat (see above)
- Games requiring kernel drivers (some VR, some DRM)
- Kernel-level mod loaders

## Common failure symptoms

| Symptom | Likely fix |
|---|---|
| Instant exit, no window | Missing `registryRead` or blocked DLL path |
| Black screen / no GPU | Ensure `gpu: true` in profile (default); try updating graphics drivers |
| "Cannot connect" / online fails | Enable `network` in profile |
| Cannot write save | Add save path to `writablePaths` |
| Anti-cheat error on launch | Not compatible — run outside GameCrate or wait for hypervisor-based approach |
| Missing DLL | Game may expect redistributables outside install dir — install VC++ runtimes on host (they load from System32) |

## Platform store launchers (out of scope)

GameCrate targets **standalone installers and direct `game.exe` launch**. Steam, Epic, GOG, and Xbox manage their own distribution, updates, DRM, and cloud saves — use those clients instead.

Advanced users may point a profile at a folder under `steamapps\common\` and launch the game `.exe` directly, but this is unsupported and GameCrate will not automate or integrate with store apps.

## Testing checklist

1. Launch game — main menu loads
2. Start new game / load save — write path works
3. Change settings — persists after restart
4. (If online) Connect to matchmaking
5. Confirm isolation: while game runs, it cannot open `\\?\C:\Users\<you>\Documents\test.txt` (use Process Monitor filtered to game PID)

## Reporting issues

When filing compatibility reports, include:

- Game name and source (**standalone** / repack / itch.io — not store launcher integration)
- Profile JSON used
- Anti-cheat / DRM if any
- Procmon log snippet of first `ACCESS DENIED` on file or registry
