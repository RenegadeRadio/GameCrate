# Virtual Storage (v0.3)

GameCrate v0.3 redirects per-game **AppData** into an isolated directory and scans **registry** changes during sandboxed installs.

## AppData redirection

When `virtualizeAppData` is enabled (default), launch and install pass a custom environment block to the sandboxed process:

| Variable | Redirected to |
|---|---|
| `APPDATA` | `%ProgramData%\GameCrate\<id>\virtual\AppData\Roaming` |
| `LOCALAPPDATA` | `%ProgramData%\GameCrate\<id>\virtual\AppData\Local` |
| `TEMP` / `TMP` | `%ProgramData%\GameCrate\<id>\virtual\Temp` |

Games and installers that honor these variables store settings and caches inside the sandbox instead of your real user profile.

### Layout

```
%ProgramData%\GameCrate\<profile-id>\
├── saves\                 # explicit save dir
├── virtual\
│   ├── AppData\
│   │   ├── Roaming\       # APPDATA
│   │   └── Local\         # LOCALAPPDATA
│   └── Temp\              # TEMP/TMP
└── install-report.json
```

### Disable redirection

```powershell
gamecrate create-profile ... --no-virtual-app-data
gamecrate install ... --no-virtual-app-data
```

Use this only for troubleshooting — malware can persist in your real `%APPDATA%` if redirection is off.

## Registry footprint scanning

During `gamecrate install`, GameCrate snapshots these `HKCU` areas before and after the installer runs:

- `Software` (depth-limited)
- `Software\Microsoft\Windows\CurrentVersion\Run`
- `Software\Microsoft\Windows\CurrentVersion\RunOnce`
- Startup approval keys

New or modified values are written to `install-report.json` as `registryChanges`. Entries under Run/RunOnce are flagged as **suspicious**.

### Important limitation

v0.3 **detects** registry persistence; it does not yet **redirect** all registry writes. Installers with `registryRead` can still touch the real `HKCU` hive. Review `outsideRegistryChanges` in the install report — any non-empty list means the installer modified your real registry.

Future versions may add hive redirection; for now, treat registry changes like filesystem outside writes.

## destroy-profile

Remove a sandbox and optionally wipe its data:

```powershell
gamecrate destroy-profile --profile my-game --wipe-data
```

- Deletes the profile JSON and AppContainer registration
- `--wipe-data` removes `%ProgramData%\GameCrate\<id>\` (saves, virtual AppData, reports)

## Malware workflow (v0.3)

1. `gamecrate install` with network off and virtualization on (defaults)
2. Check install report:
   - `outsideWrites` should be **0**
   - `outsideRegistryChanges` should be **empty** (or only benign keys you expect)
   - `suspiciousRegistryChanges` should be **empty**
3. If clean, `gamecrate launch --profile <id>`

If registry changes appear, delete the profile with `--wipe-data` and do not launch.
