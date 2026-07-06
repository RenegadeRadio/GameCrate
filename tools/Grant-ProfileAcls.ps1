#Requires -Version 5.1
<#
.SYNOPSIS
    Apply filesystem ACL grants for a WinDoze profile.

.PARAMETER ProfileId
    Profile identifier.

.PARAMETER InstallDir
    Optional override if the profile JSON is not yet created.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ProfileId
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$windozeCandidates = @(
    (Join-Path $PSScriptRoot "..\build\Release\windoze.exe"),
    (Join-Path $PSScriptRoot "..\out\build\x64-Release\windoze.exe"),
    "windoze.exe"
)

$windoze = $windozeCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $windoze) { throw "windoze.exe not found." }

& $windoze grant --profile $ProfileId
exit $LASTEXITCODE
