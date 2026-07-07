# Compatibility matrix

Community-tested **standalone** games. Store launcher titles (Steam/Epic/GOG) are [out of scope](SCOPE.md).

To add a row: open a [compatibility report](https://github.com/RenegadeRadio/GameCrate/issues/new?template=compatibility_report.yml) or send a PR editing this file.

| Game | Source | Install | Play | GameCrate | Notes |
|------|--------|---------|------|-----------|-------|
| *(example)* Offline indie | Portable folder | N/A (create-profile) | Works | v0.4.7+ | Template: `profiles/examples/offline-indie.json` |
| City Game Studio | TiNY / repack (`rg-setup.exe`) | Works | VM: no GPU | v0.4.5+ | Install to user-owned folder; approve UAC; Start Menu writes benign in v0.4.6+ |
| — | — | — | — | — | *Add your results here* |

## Legend

| Install / Play | Meaning |
|----------------|---------|
| Works | Succeeded without manual profile edits |
| Works (manual exe) | Install OK after picking `game.exe` when auto-detect failed |
| VM: no GPU | Install OK; Play fails on GPU-less VM (expected) |
| Failed | See issue or notes |

## Related

- [GAME_COMPATIBILITY.md](GAME_COMPATIBILITY.md) — tiers, symptoms, fixes
- [TESTING.md](TESTING.md) — tester handout
- [SCOPE.md](SCOPE.md) — standalone only
