# Game Compatibility Guide

WinDoze uses LPAC sandboxing. Not every game will work without profile tuning. Use this guide to classify titles and adjust profiles.

## Compatibility tiers

### Tier A — Likely works out of the box

- Offline indie games (single EXE or small folder)
- Older Win32 titles without kernel anti-cheat
- Games installed to a dedicated directory with saves beside the EXE

**Profile template:** `profiles/examples/offline-indie.json`

### Tier B — Needs capability tuning

- Games that read registry for settings (enable `registryRead`)
- LAN multiplayer (add `privateNetworkClientServer`)
- Online multiplayer (add `internetClient`)
- Games using COM codecs or shell integration (add `lpacCom`)

### Tier C — Problematic

- Steam/Epic/GOG launchers — need launcher path grants + network + often COM
- Games with self-updaters — need write access to install dir
- Titles using EasyAntiCheat, BattlEye, Vanguard — **will not work** in v0.1

### Tier D — Unsupported in v0.1

- Kernel anti-cheat (see above)
- Games requiring kernel drivers (some VR, some DRM)
- Kernel-level mod loaders

## Common failure symptoms

| Symptom | Likely fix |
|---|---|
| Instant exit, no window | Missing `registryRead` or blocked DLL path |
| "Cannot connect" / online fails | Enable `network` in profile |
| Cannot write save | Add save path to `writablePaths` |
| Anti-cheat error on launch | Not compatible — run outside WinDoze or wait for hypervisor-based approach |
| Missing DLL | Game may expect redistributables outside install dir — install VC++ runtimes on host (they load from System32) |

## Launcher integration (manual v0.1)

For Steam games, a typical flow:

1. Create profile with `installDir` = `...\steamapps\common\GameName`
2. Add read grant for `...\steamapps\appmanifest_*.acf` parent if needed
3. Enable `network` and `registryRead`
4. Launch `Game.exe` directly, not `steam.exe` (avoids Steam singleton issues)

Future WinDoze versions will automate launcher detection.

## Testing checklist

1. Launch game — main menu loads
2. Start new game / load save — write path works
3. Change settings — persists after restart
4. (If online) Connect to matchmaking
5. Confirm isolation: while game runs, it cannot open `\\?\C:\Users\<you>\Documents\test.txt` (use Process Monitor filtered to game PID)

## Reporting issues

When filing compatibility reports, include:

- Game name and store (Steam/Epic/standalone)
- Profile JSON used
- Anti-cheat / DRM if any
- Procmon log snippet of first `ACCESS DENIED` on file or registry
