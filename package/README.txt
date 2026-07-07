GameCrate v0.4 — Windows x64
==============================

Quick start (GUI)
-----------------
1. Extract this folder anywhere on your Windows VM (e.g. C:\Tools\GameCrate).
2. Double-click GameCrate.Gui.exe
3. Use "Install game..." for sandboxed installers, or "Play" for existing profiles.

Install folder tips
-------------------
- Use a folder you own, e.g. D:\Games\Sandbox\MyGame or %LOCALAPPDATA%\Games\MyGame
- Avoid C:\Install and other drive-root folders — ACL grants often fail there
- Avoid C:\Program Files — installers and ACL grants fail with "Access is denied"
- GameCrate data (profiles, saves, reports) lives under %LOCALAPPDATA%\GameCrate\

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
