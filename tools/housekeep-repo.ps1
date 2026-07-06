#Requires -Version 5.1
<#
.SYNOPSIS
    Housekeeping for the GameCrate GitHub repo.

.DESCRIPTION
    - Merges AGENTS.md (if on housekeeping branch) into main
    - Deletes stale cursor/* feature branches already merged to main
    - Closes PR #3 (setup-dev-environment) after AGENTS.md is on main

    Run from a clone with push access and `gh` authenticated:
      gh auth login
      .\tools\housekeep-repo.ps1

.EXAMPLE
    .\tools\housekeep-repo.ps1
    .\tools\housekeep-repo.ps1 -DeleteRemoteBranches
#>
[CmdletBinding()]
param(
    [switch]$DeleteRemoteBranches
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$staleBranches = @(
    "cursor/game-container-sandbox-ae40",
    "cursor/fix-ci-gui-build-ae40",
    "cursor/docs-v04-completion-ae40",
    "cursor/github-releases-ae40",
    "cursor/setup-dev-environment-8834",
    "cursor/housekeeping-ae40"
)

Write-Host "Fetching origin..."
git fetch origin --prune

$current = git branch --show-current
if ($current -ne "main") {
    Write-Host "Checking out main..."
    git checkout main
}

git pull origin main

if (Test-Path "AGENTS.md") {
    if (-not (git diff --quiet HEAD -- AGENTS.md 2>$null) -or
        -not (git log origin/main..HEAD --oneline -- AGENTS.md 2>$null)) {
        $onMain = git show origin/main:AGENTS.md 2>$null
        if (-not $onMain) {
            Write-Host "Committing AGENTS.md to main..."
            git add AGENTS.md
            git commit -m "Add AGENTS.md with Cursor Cloud and Windows-only build notes"
        }
    }
}

Write-Host "Pushing main..."
git push origin main

if ($DeleteRemoteBranches) {
    foreach ($branch in $staleBranches) {
        $exists = git ls-remote --heads origin $branch 2>$null
        if ($exists) {
            Write-Host "Deleting remote branch $branch..."
            git push origin --delete $branch
        }
    }

    foreach ($branch in $staleBranches) {
        if (git show-ref --verify --quiet "refs/heads/$branch") {
            Write-Host "Deleting local branch $branch..."
            git branch -D $branch 2>$null
        }
    }
}

$gh = Get-Command gh -ErrorAction SilentlyContinue
if ($gh) {
    $pr3 = gh pr view 3 --json state -q .state 2>$null
    if ($pr3 -eq "OPEN") {
        Write-Host "Closing PR #3 (AGENTS.md now on main)..."
        gh pr close 3 --comment "Housekeeping: AGENTS.md merged to main. Stale branches removed."
    }
    Write-Host ""
    Write-Host "Open PRs:"
    gh pr list --state open
} else {
    Write-Host "Install GitHub CLI (gh) to auto-close PR #3."
}

Write-Host ""
Write-Host "Done. Remote branches on origin:"
git branch -r
