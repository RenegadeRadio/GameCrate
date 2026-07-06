#Requires -Version 5.1
<#
.SYNOPSIS
    Build windoze.exe on Windows with CMake + Visual Studio 2022.

.EXAMPLE
    .\build.ps1
    .\build.ps1 -Configuration Debug
#>
[CmdletBinding()]
param(
    [ValidateSet("Release", "Debug")]
    [string]$Configuration = "Release",

    [string]$BuildDir = "build"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
Push-Location $repoRoot

try {
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vsWhere)) {
        throw "Visual Studio 2022 not found. Install the 'Desktop development with C++' workload."
    }

    $vsPath = & $vsWhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
    if (-not $vsPath) {
        throw "MSBuild component not found in Visual Studio installation."
    }

  $cmake = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmake) {
        throw "cmake not found on PATH. Install CMake 3.20+."
    }

    Write-Host "Configuring ($Configuration, x64)..."
    cmake -B $BuildDir -G "Visual Studio 17 2022" -A x64
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    Write-Host "Building..."
    cmake --build $BuildDir --config $Configuration
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    $exe = Join-Path $repoRoot "$BuildDir\$Configuration\windoze.exe"
    if (-not (Test-Path $exe)) {
        $exe = Join-Path $repoRoot "$BuildDir\windoze.exe"
    }
    if (-not (Test-Path $exe)) {
        throw "Build finished but windoze.exe was not found under $BuildDir."
    }

    Write-Host ""
    Write-Host "Built: $exe" -ForegroundColor Green
    Write-Host "Run:   & '$exe' list-profiles"
}
finally {
    Pop-Location
}
