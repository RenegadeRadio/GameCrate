# GameCrate documentation

GameCrate runs Windows games inside a Less-Privileged AppContainer (LPAC) with explicit filesystem grants. This folder documents design, usage, and threat assumptions.

> The project was formerly called **WinDoze**. All paths and binaries use **GameCrate** / `gamecrate.exe`.

## Version map

| Version | Focus | Key docs |
|---|---|---|
| **v0.1** | LPAC launcher, JSON profiles, ACL grants, CLI | [HOW_IT_RUNS.md](HOW_IT_RUNS.md), [ARCHITECTURE.md](ARCHITECTURE.md) |
| **v0.2** | Sandboxed installer + footprint scanner | [SANDBOXED_INSTALL.md](SANDBOXED_INSTALL.md) |
| **v0.3** | AppData virtualization, registry install scan, `destroy-profile` | [VIRTUAL_STORAGE.md](VIRTUAL_STORAGE.md) |
| **v0.4** | WPF tray GUI, [GitHub Releases](https://github.com/RenegadeRadio/GameCrate/releases) zip | [GUI.md](GUI.md) |
| **v0.5** (planned) | Steam/Epic launcher integration | — |

## Start here

| Doc | When to read |
|---|---|
| [CLI.md](CLI.md) | Full command reference, flags, JSON output, install reports |
| [HOW_IT_RUNS.md](HOW_IT_RUNS.md) | What runs when you launch a game; CLI vs GUI |
| [GUI.md](GUI.md) | Tray app usage, install wizard, troubleshooting |
| [SANDBOXED_INSTALL.md](SANDBOXED_INSTALL.md) | Untrusted installer workflow |
| [GAME_COMPATIBILITY.md](GAME_COMPATIBILITY.md) | Which games work; capability tuning |

## Design and security

| Doc | Contents |
|---|---|
| [ARCHITECTURE.md](ARCHITECTURE.md) | Isolation stack, components, lifecycle |
| [THREAT_MODEL.md](THREAT_MODEL.md) | Assets, controls, residual risks |
| [VIRTUAL_STORAGE.md](VIRTUAL_STORAGE.md) | AppData redirect, registry scanning, teardown |

## Related files

- Profile JSON schema: [`profiles/schema.json`](../profiles/schema.json)
- Example profile: [`profiles/examples/offline-indie.json`](../profiles/examples/offline-indie.json)
- PowerShell helpers: [`tools/`](../tools/)
- CI workflow: [`.github/workflows/build.yml`](../.github/workflows/build.yml)
- Windows download: [GitHub Releases](https://github.com/RenegadeRadio/GameCrate/releases) (`GameCrate-windows-x64.zip`; latest stable: `v0.4.0`)
- Cursor Cloud agents: [`AGENTS.md`](../AGENTS.md) (Windows-only build notes)
