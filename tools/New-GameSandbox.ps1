#Requires -Version 5.1
<#
.SYNOPSIS
    Guided setup for a new WinDoze game sandbox profile.

.DESCRIPTION
    Creates a profile, applies ACL grants, and optionally launches the game.
    Run from an elevated PowerShell if ACL grants fail for system-wide paths.

.PARAMETER Id
    Profile identifier (lowercase, hyphens allowed).

.PARAMETER Name
    Display name for the game.

.PARAMETER InstallDir
    Directory where the game is (or will be) installed.

.PARAMETER Executable
    Path to the game .exe. Defaults to searching for a single .exe in InstallDir.

.PARAMETER Network
    Enable network capabilities for online games.

.EXAMPLE
    .\New-GameSandbox.ps1 -Id "hollow-knight" -Name "Hollow Knight" -InstallDir "D:\Games\HollowKnight" -Executable "D:\Games\HollowKnight\hollow_knight.exe"
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$Id,

    [Parameter(Mandatory = $true)]
    [string]$Name,

    [Parameter(Mandatory = $true)]
    [string]$InstallDir,

    [string]$Executable = "",

    [switch]$Network,

    [switch]$Launch
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Find-WinDozeExe {
    $candidates = @(
        (Join-Path $PSScriptRoot "..\build\Release\windoze.exe"),
        (Join-Path $PSScriptRoot "..\out\build\x64-Release\windoze.exe"),
        "windoze.exe"
    )
    foreach ($path in $candidates) {
        if (Test-Path $path) { return (Resolve-Path $path).Path }
    }
    throw "windoze.exe not found. Build the project first or add it to PATH."
}

$windoze = Find-WinDozeExe

if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
}

if ([string]::IsNullOrWhiteSpace($Executable)) {
    $exes = Get-ChildItem -Path $InstallDir -Filter *.exe -File -ErrorAction SilentlyContinue
    if ($exes.Count -eq 1) {
        $Executable = $exes[0].FullName
    } else {
        throw "Specify -Executable or ensure exactly one .exe exists in InstallDir."
    }
}

$args = @(
    "create-profile",
    "--id", $Id,
    "--name", $Name,
    "--install-dir", $InstallDir,
    "--executable", $Executable
)

if ($Network) { $args += "--network" }

Write-Host "Creating WinDoze profile '$Id'..."
& $windoze @args
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Applying ACL grants..."
& $windoze grant --profile $Id
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if ($Launch) {
    Write-Host "Launching sandboxed game..."
    & $windoze launch --profile $Id
    exit $LASTEXITCODE
}

Write-Host "Done. Launch with: windoze launch --profile $Id"
