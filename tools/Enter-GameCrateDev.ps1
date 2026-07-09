#Requires -Version 5.1
<#
.SYNOPSIS
  Start an isolated GameCrate development session (project-local data, not AppData).

.DESCRIPTION
  Sets GAMECRATE_DATA_ROOT to <repo>\.dev-data so profiles, saves, and install
  reports stay inside the clone. Optionally builds the CLI and publishes the GUI
  into build\dev-package for a self-contained dev tree.

.EXAMPLE
  .\tools\Enter-GameCrateDev.ps1
  .\tools\Enter-GameCrateDev.ps1 -Build
#>
[CmdletBinding()]
param(
    [switch] $Build,
    [switch] $PublishGui,
    [ValidateSet("Release", "Debug")]
    [string] $Configuration = "Release"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$dataRoot = Join-Path $repoRoot ".dev-data"
$gamesRoot = Join-Path $dataRoot "games"
$profilesRoot = Join-Path $dataRoot "profiles"
$devPackage = Join-Path $repoRoot "build\dev-package"

foreach ($dir in @($dataRoot, $gamesRoot, $profilesRoot)) {
    New-Item -ItemType Directory -Force -Path $dir | Out-Null
}

$env:GAMECRATE_DATA_ROOT = $dataRoot
$env:GAMECRATE_DEV_GAMES_DIR = $gamesRoot

if ($Build) {
    & (Join-Path $PSScriptRoot "build.ps1") -Configuration $Configuration
}

if ($PublishGui) {
    $dotnet = Get-Command dotnet -ErrorAction SilentlyContinue
    if (-not $dotnet) {
        throw ".NET 8 SDK required for -PublishGui."
    }

    $cliExe = Join-Path $repoRoot "build\$Configuration\gamecrate.exe"
    if (-not (Test-Path $cliExe)) {
        $cliExe = Join-Path $repoRoot "build\gamecrate.exe"
    }
    if (-not (Test-Path $cliExe)) {
        throw "gamecrate.exe not found. Run with -Build first."
    }

    New-Item -ItemType Directory -Force -Path $devPackage | Out-Null
    dotnet publish (Join-Path $repoRoot "src\gui\GameCrate.Gui\GameCrate.Gui.csproj") `
        -c $Configuration -r win-x64 --self-contained true `
        -p:PublishReadyToRun=true `
        -o $devPackage
    Copy-Item $cliExe (Join-Path $devPackage "gamecrate.exe") -Force
    Copy-Item (Join-Path $repoRoot "profiles") (Join-Path $devPackage "profiles") -Recurse -Force
    Copy-Item (Join-Path $repoRoot "tools") (Join-Path $devPackage "tools") -Recurse -Force
}

$pathEntries = @()
$cliCandidates = @(
    (Join-Path $repoRoot "build\$Configuration\gamecrate.exe"),
    (Join-Path $repoRoot "build\gamecrate.exe"),
    (Join-Path $devPackage "gamecrate.exe")
)
foreach ($candidate in $cliCandidates) {
    $dir = Split-Path -Parent $candidate
    if ((Test-Path $candidate) -and $pathEntries -notcontains $dir) {
        $pathEntries += $dir
    }
}
if ($pathEntries.Count -gt 0) {
    $env:PATH = ($pathEntries -join ';') + ';' + $env:PATH
}

Write-Host ""
Write-Host "GameCrate dev environment" -ForegroundColor Cyan
Write-Host "  GAMECRATE_DATA_ROOT     = $dataRoot"
Write-Host "  GAMECRATE_DEV_GAMES_DIR = $gamesRoot"
Write-Host "  Install test games to   = $gamesRoot\<game-name>"
if (Test-Path (Join-Path $devPackage "GameCrate.Gui.exe")) {
    Write-Host "  GUI                     = $devPackage\GameCrate.Gui.exe"
}
Write-Host ""
Write-Host "Examples:"
Write-Host "  gamecrate list-profiles"
Write-Host "  gamecrate install --id test --name Test --install-dir `"$gamesRoot\test`" --installer D:\Downloads\setup.exe"
Write-Host ""
Write-Host "Type 'exit' to leave this shell. Data stays under .dev-data until you delete it."
Write-Host ""

Set-Location $repoRoot

if ($MyInvocation.InvocationName -eq '.') {
    Write-Host "Dev environment active in this shell." -ForegroundColor Green
    return
}

$pathPrefix = if ($pathEntries.Count -gt 0) { ($pathEntries -join ';') + ';' } else { '' }
$nested = @"
Set-Location '$repoRoot'
`$env:GAMECRATE_DATA_ROOT = '$dataRoot'
`$env:GAMECRATE_DEV_GAMES_DIR = '$gamesRoot'
`$env:PATH = '$pathPrefix' + `$env:PATH
function prompt { 'gamecrate-dev> ' + (`$(Get-Location).Path) + '> ' }
"@
powershell -NoExit -Command $nested
