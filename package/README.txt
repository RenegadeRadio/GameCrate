GameCrate v0.4.7 — Windows x64
================================

Quick start (GUI)
-----------------
1. Extract this folder anywhere (e.g. C:\Tools\GameCrate).
2. Double-click GameCrate.Gui.exe
3. Use "Install game..." for installers, or "Play" for existing profiles.

Install folder (you choose — AppData is NOT required)
-----------------------------------------------------
  D:\Games\my-game                          recommended if you have D:
  C:\Users\You\Games\my-game               any folder you own
  %LOCALAPPDATA%\Games\my-game              also fine (just a suggestion)

  Avoid: C:\Install, C:\Program Files, drive roots (C:\Something)

GameCrate data (automatic)
--------------------------
  %LOCALAPPDATA%\GameCrate\                 profiles, saves, install reports

## Scope

GameCrate is for **standalone games** — not Steam/Epic/GOG. See docs\SCOPE.md

Testing guide (share with testers)
----------------------------------
  docs\TESTING.md  —  full instructions, what to report back, troubleshooting
  https://github.com/RenegadeRadio/GameCrate/blob/main/docs/TESTING.md

Quick start (CLI)
-----------------
  gamecrate.exe list-profiles
  gamecrate.exe install --id my-game --name "My Game" --install-dir D:\Games\my-game --installer D:\setup.exe
  gamecrate.exe launch --profile my-game

Requirements
------------
- Windows 10 version 1809+ or Windows 11 (x64)
- Self-contained GUI — no separate .NET install required
- gamecrate.exe is statically linked (v0.4.2+) — no VC++ Redistributable needed
- Standard user account; approve UAC if the game installer prompts
- Play needs a GPU (VMs without 3D acceleration may install but not play)

Files
-----
  GameCrate.Gui.exe   Tray GUI (recommended)
  gamecrate.exe       CLI control plane
  profiles\           JSON schema and examples
  tools\              PowerShell helper scripts

Documentation: https://github.com/RenegadeRadio/GameCrate/tree/main/docs
