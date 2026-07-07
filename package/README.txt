GameCrate v0.4 — Windows x64
==============================

Quick start (GUI)
-----------------
1. Extract this folder anywhere on your Windows VM (e.g. C:\Tools\GameCrate).
2. Double-click GameCrate.Gui.exe
3. Use "Install game..." for sandboxed installers, or "Play" for existing profiles.

Quick start (CLI)
-----------------
  gamecrate.exe list-profiles
  gamecrate.exe install --id my-game --name "My Game" --install-dir D:\Sandbox\MyGame --installer D:\setup.exe
  gamecrate.exe launch --profile my-game

Requirements
------------
- Windows 10 version 1809+ or Windows 11 (x64)
- No separate .NET install required (self-contained GUI build)
- gamecrate.exe is statically linked (v0.4.2+) — no VC++ Redistributable needed
- Standard user account (not required to run as Administrator)

Files
-----
  GameCrate.Gui.exe   Tray GUI (recommended)
  gamecrate.exe       CLI control plane
  profiles\           JSON schema and examples
  tools\              PowerShell helper scripts

Documentation: https://github.com/RenegadeRadio/GameCrate/tree/main/docs
