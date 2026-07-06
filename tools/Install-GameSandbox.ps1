#Requires -Version 5.1
<#
.SYNOPSIS
    Run a game installer inside a GameCrate sandbox and scan the install footprint.

.EXAMPLE
    .\Install-GameSandbox.ps1 -Id "test-game" -Name "Test Game" `
        -InstallDir "D:\Sandbox\TestGame" -Installer "D:\Downloads\setup.exe"
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$Id,

    [Parameter(Mandatory = $true)]
    [string]$Name,

    [Parameter(Mandatory = $true)]
    [string]$InstallDir,

    [Parameter(Mandatory = $true)]
    [string]$Installer,

    [string]$InstallerArgs = "",

    [string]$Executable = "",

    [switch]$Network,

    [switch]$AllowOutsideWrites,

    [switch]$Launch
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Find-GameCrateExe {
    $candidates = @(
        (Join-Path $PSScriptRoot "..\build\gamecrate.exe"),
        (Join-Path $PSScriptRoot "..\build\Release\gamecrate.exe"),
        (Join-Path $PSScriptRoot "..\out\build\x64-Release\gamecrate.exe"),
        "gamecrate.exe"
    )
    foreach ($path in $candidates) {
        if (Test-Path $path) { return (Resolve-Path $path).Path }
    }
    throw "gamecrate.exe not found. Build the project first or add it to PATH."
}

$gamecrate = Find-GameCrateExe

$args = @(
    "install",
    "--id", $Id,
    "--name", $Name,
    "--install-dir", $InstallDir,
    "--installer", $Installer
)

if ($InstallerArgs) { $args += @("--installer-args", $InstallerArgs) }
if ($Executable) { $args += @("--executable", $Executable) }
if ($Network) { $args += "--network" }
if ($AllowOutsideWrites) { $args += "--allow-outside-writes" }

Write-Host "Running sandboxed installer for '$Name'..."
& $gamecrate @args
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $gamecrate show-install-report --profile $Id

if ($Launch) {
    Write-Host "Launching sandboxed game..."
    & $gamecrate launch --profile $Id
    exit $LASTEXITCODE
}

Write-Host "Done. Launch with: gamecrate launch --profile $Id"
