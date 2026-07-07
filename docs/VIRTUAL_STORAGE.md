# Virtual Storage (v0.3)

GameCrate v0.3 redirects per-game **AppData** into an isolated directory and scans **registry** changes during sandboxed installs.

## AppData redirection

When `virtualizeAppData` is enabled (default), launch and install pass a custom environment block to the sandboxed process:

| Variable | Redirected to |
|---|---|
| `APPDATA` | `%LOCALAPPDATA%\GameCrate\<id>\virtual\AppData\Roaming` |
| `LOCALAPPDATA` | `%LOCALAPPDATA%\GameCrate\<id>\virtual\AppData\Local` |
| `TEMP` / `TMP` | `%LOCALAPPDATA%\GameCrate\<id>\virtual\Temp` |

Games and installers that honor these variables store settings and caches inside the sandbox instead of your real user profile.

### Layout

```
%LOCALAPPDATA%\GameCrate\<profile-id>\
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
- `Software\Microsoft\Windows\CurrentVersion\Explorer\StartupApproved\Run`
- `Software\Wow6432Node\Microsoft\Windows\CurrentVersion\Run`

New or modified values are recorded in `install-report.json`:

| Array | Meaning |
|---|---|
| `registryChanges` | All detected registry diffs |
| `outsideRegistryChanges` | Changes outside the virtual sandbox context |
| `suspiciousRegistryChanges` | Run/RunOnce and similar persistence keys |

Entries under Run/RunOnce are flagged as **suspicious**.

### Important limitation

Installs **fail** only on filesystem writes outside the sandbox or **suspicious** registry keys (Run/RunOnce persistence). Normal `HKCU\Software\...` installer keys are recorded in `registryChanges` for review but do not block install.

Future versions may add hive redirection; for now, treat registry changes like filesystem outside writes.

## destroy-profile

Remove a sandbox and optionally wipe its data:

```powershell
gamecrate destroy-profile --profile my-game --wipe-data
```

- Deletes the profile JSON and AppContainer registration
- Revokes filesystem ACL ACEs on install, save, and extra granted paths
- `--wipe-data` removes `%ProgramData%\GameCrate\<id>\` (saves, virtual AppData, reports)

Does **not** delete game files in `installDir` — remove those manually if needed.

Full reference: [CLI.md](CLI.md#destroy-profile).

## Malware workflow (v0.3)

1. `gamecrate install` with network off and virtualization on (defaults)
2. Check install report:
   - `outsideWrites` should be **0**
   - `outsideRegistryChanges` should be **empty** (suspicious persistence keys only)
   - `registryChanges` may list benign installer keys — review but not a failure by itself
   - `suspiciousRegistryChanges` should be **empty**
3. If clean, `gamecrate launch --profile <id>`

If registry changes appear, delete the profile with `--wipe-data` and do not launch.
