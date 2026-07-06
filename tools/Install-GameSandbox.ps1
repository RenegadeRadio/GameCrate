#Requires -Version 5.1
<#
.SYNOPSIS
    Run a game installer inside a WinDoze sandbox and scan the install footprint.

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

function Find-WinDozeExe {
    $candidates = @(
        (Join-Path $PSScriptRoot "..\build\windoze.exe"),
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
& $windoze @args
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $windoze show-install-report --profile $Id

if ($Launch) {
    Write-Host "Launching sandboxed game..."
    & $windoze launch --profile $Id
    exit $LASTEXITCODE
}

Write-Host "Done. Launch with: windoze launch --profile $Id"
