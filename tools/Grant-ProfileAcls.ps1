#Requires -Version 5.1
<#
.SYNOPSIS
    Apply filesystem ACL grants for a GameCrate profile.

.PARAMETER ProfileId
    Profile identifier.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ProfileId
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$gamecrateCandidates = @(
    (Join-Path $PSScriptRoot "..\build\gamecrate.exe"),
    (Join-Path $PSScriptRoot "..\build\Release\gamecrate.exe"),
    (Join-Path $PSScriptRoot "..\out\build\x64-Release\gamecrate.exe"),
    "gamecrate.exe"
)

$gamecrate = $gamecrateCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $gamecrate) { throw "gamecrate.exe not found." }

& $gamecrate grant --profile $ProfileId
exit $LASTEXITCODE
