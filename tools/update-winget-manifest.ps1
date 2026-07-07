#Requires -Version 5.1
<#
.SYNOPSIS
  Create or update winget manifest files for a GameCrate release.

.PARAMETER Version
  Semantic version without leading v (e.g. 0.4.8) or with v prefix.

.PARAMETER ZipPath
  Local path to GameCrate-windows-x64.zip. If omitted, downloads from GitHub Releases.

.PARAMETER Repository
  GitHub repository (owner/name).

.EXAMPLE
  .\tools\update-winget-manifest.ps1 -Version 0.4.8 -ZipPath .\GameCrate-windows-x64.zip
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string] $Version,

    [string] $ZipPath,

    [string] $Repository = 'RenegadeRadio/GameCrate'
)

$ErrorActionPreference = 'Stop'

$semver = $Version.TrimStart('v')
$tag = "v$semver"
$repoRoot = Split-Path -Parent $PSScriptRoot
$manifestDir = Join-Path $repoRoot "winget\manifests\r\RenegadeRadio\GameCrate\$semver"

if (-not $ZipPath) {
    $ZipPath = Join-Path $env:TEMP "GameCrate-windows-x64-$semver.zip"
    $url = "https://github.com/$Repository/releases/download/$tag/GameCrate-windows-x64.zip"
    Write-Host "Downloading $url"
    Invoke-WebRequest -Uri $url -OutFile $ZipPath -UseBasicParsing
}

if (-not (Test-Path $ZipPath)) {
    throw "Zip not found: $ZipPath"
}

$hash = (Get-FileHash -Path $ZipPath -Algorithm SHA256).Hash.ToUpperInvariant()
$installerUrl = "https://github.com/$Repository/releases/download/$tag/GameCrate-windows-x64.zip"

function Set-Utf8NoBomFile {
    param([string] $Path, [string] $Content)
    $utf8 = New-Object System.Text.UTF8Encoding $false
    [System.IO.File]::WriteAllText($Path, $Content, $utf8)
}

New-Item -ItemType Directory -Force -Path $manifestDir | Out-Null

$versionYaml = @"
# yaml-language-server: `$schema=https://aka.ms/winget-manifest.version.1.6.0.schema.json

PackageIdentifier: RenegadeRadio.GameCrate
PackageVersion: $semver
DefaultLocale: en-US
ManifestType: version
ManifestVersion: 1.6.0
"@
Set-Utf8NoBomFile -Path (Join-Path $manifestDir 'RenegadeRadio.GameCrate.yaml') -Content $versionYaml

$installerYaml = @"
# yaml-language-server: `$schema=https://aka.ms/winget-manifest.installer.1.6.0.schema.json

PackageIdentifier: RenegadeRadio.GameCrate
PackageVersion: $semver
Platform:
  - Windows.Desktop
MinimumOSVersion: 10.0.17763.0
InstallerType: zip
NestedInstallerType: portable
NestedInstallerFiles:
  - RelativeFilePath: GameCrate\gamecrate.exe
    PortableCommandAlias: gamecrate
  - RelativeFilePath: GameCrate\GameCrate.Gui.exe
    PortableCommandAlias: gamecrate-gui
Commands:
  - gamecrate
  - gamecrate-gui
Installers:
  - Architecture: x64
    InstallerUrl: $installerUrl
    InstallerSha256: $hash
ManifestType: installer
ManifestVersion: 1.6.0
"@
Set-Utf8NoBomFile -Path (Join-Path $manifestDir 'RenegadeRadio.GameCrate.installer.yaml') -Content $installerYaml

$localePath = Join-Path $manifestDir 'RenegadeRadio.GameCrate.locale.en-US.yaml'
if (-not (Test-Path $localePath)) {
    $localeYaml = @"
# yaml-language-server: `$schema=https://aka.ms/winget-manifest.defaultLocale.1.6.0.schema.json

PackageIdentifier: RenegadeRadio.GameCrate
PackageVersion: $semver
PackageLocale: en-US
Publisher: RenegadeRadio
PublisherUrl: https://github.com/RenegadeRadio
PublisherSupportUrl: https://github.com/$Repository/issues
PackageName: GameCrate
PackageUrl: https://github.com/$Repository
License: MIT
LicenseUrl: https://github.com/$Repository/blob/main/LICENSE
ShortDescription: Windows LPAC sandbox for standalone games
Description: |-
  GameCrate runs standalone games inside a Less-Privileged AppContainer (LPAC) with explicit filesystem grants.
  Installers run monitored; play is sandboxed. For standalone installers and portable games — not Steam/Epic/GOG.
Moniker: gamecrate
Tags:
  - sandbox
  - games
  - security
  - lpac
  - windows
  - portable
ReleaseNotesUrl: https://github.com/$Repository/releases/tag/$tag
ManifestType: defaultLocale
ManifestVersion: 1.6.0
"@
    Set-Utf8NoBomFile -Path $localePath -Content $localeYaml
} else {
    $content = Get-Content -Path $localePath -Raw
    $content = $content -replace 'PackageVersion: .*', "PackageVersion: $semver"
    $content = $content -replace 'ReleaseNotesUrl: .*', "ReleaseNotesUrl: https://github.com/$Repository/releases/tag/$tag"
    Set-Utf8NoBomFile -Path $localePath -Content $content.TrimEnd()
}

Write-Host "Wrote manifests to $manifestDir"
Write-Host "InstallerSha256: $hash"
Write-Host ""
Write-Host "Validate (Windows): winget validate --manifest `"$manifestDir`""
