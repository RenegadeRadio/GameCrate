# Changelog

All notable user-facing changes. Versions match [GitHub Releases](https://github.com/RenegadeRadio/GameCrate/releases).

## v0.4.8

- GitHub issue templates for compatibility and install failures
- Compatibility matrix doc (`docs/COMPATIBILITY_MATRIX.md`)
- GUI: pick game `.exe` when install finishes but auto-detect fails (`set-executable`)
- GUI: summarized install report dialog; About shows version
- CI: validate profile JSON against schema; smoke-test `create-profile` / `destroy-profile`
- `CHANGELOG.md` added

## v0.4.7

- **`docs/SCOPE.md`** — standalone games only; not Steam/Epic/GOG
- Docs aligned with `%LOCALAPPDATA%\GameCrate\` data paths and monitored install
- Tester guide and package README scope notes

## v0.4.6

- Start Menu shortcuts and icon caches treated as benign outside writes
- Default: do not fail install on benign outside writes (`--strict-outside-writes` to enforce)
- GUI strict-outside-writes checkbox

## v0.4.5

- **Monitored install** — installer runs as normal child process (UAC allowed), not LPAC
- Auto `/DIR=` for Inno-style installers

## v0.4.4

- UTF-8 CLI output; GUI CLI encoding auto-detect

## v0.4.3

- GameCrate data under `%LOCALAPPDATA%\GameCrate\` (was `%ProgramData%`)
- Clearer ACL / access-denied error messages

## v0.4.2

- Statically linked `gamecrate.exe` (no VC++ Redistributable required)

## v0.4.0 – v0.4.1

- WPF tray GUI (`GameCrate.Gui.exe`)
- GitHub Actions Windows builds and Releases

## v0.3

- AppData virtualization, registry install scan, `destroy-profile`

## v0.2

- Sandboxed installer + footprint scanner (superseded by monitored install in v0.4.5)

## v0.1

- LPAC launcher, JSON profiles, ACL grants, CLI
