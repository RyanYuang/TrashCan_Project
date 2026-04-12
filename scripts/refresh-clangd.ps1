<#
.SYNOPSIS
    Clear clangd on-disk index under this repo and optionally restart clangd.

.DESCRIPTION
    Cursor/VS Code does not expose a stable CLI for "clangd: Restart language server".
    Practical refresh:
    1) Delete workspace .cache/clangd (one common on-disk index location)
    2) Stop clangd.exe; the editor usually respawns clangd and rebuilds index

    WARNING: default behavior stops ALL clangd.exe on this machine. If you rely on
    multiple workspaces simultaneously, use -NoKill and restart clangd from the palette.

.PARAMETER WorkspaceRoot
    Workspace root. Defaults to repo root (parent of the scripts folder).

.PARAMETER RecurseWorkspace
    Recursively find and delete every .cache/clangd under WorkspaceRoot.
    Default only checks WorkspaceRoot and WorkspaceRoot\MCU (faster for this layout).

.PARAMETER ClearIndex
    Delete matched .cache/clangd directories.

.PARAMETER KillClangd
    Stop all processes named clangd.

.PARAMETER NoKill
    Only delete index directories; you restart clangd manually or save sources.

.PARAMETER Quick
    Only restart clangd; keep on-disk index (lighter; may not fix stubborn index bugs).

.EXAMPLE
    .\scripts\refresh-clangd.ps1

.EXAMPLE
    .\scripts\refresh-clangd.ps1 -RecurseWorkspace

.EXAMPLE
    .\scripts\refresh-clangd.ps1 -Quick

.EXAMPLE
    .\scripts\refresh-clangd.ps1 -NoKill
#>
[CmdletBinding()]
param(
    [string]$WorkspaceRoot = '',

    [switch]$RecurseWorkspace,

    [switch]$ClearIndex,

    [switch]$KillClangd,

    [switch]$NoKill,

    [switch]$Quick
)

$ErrorActionPreference = 'Stop'

$scriptDir = if ($PSScriptRoot) { $PSScriptRoot } else { Split-Path -Parent $MyInvocation.MyCommand.Path }
if (-not $WorkspaceRoot) {
    $WorkspaceRoot = (Resolve-Path (Join-Path $scriptDir '..')).Path
}
$WorkspaceRoot = (Resolve-Path -LiteralPath $WorkspaceRoot).Path

if (-not $Quick -and -not $PSBoundParameters.ContainsKey('ClearIndex')) {
    $ClearIndex = $true
}
if (-not $NoKill -and -not $PSBoundParameters.ContainsKey('KillClangd')) {
    $KillClangd = $true
}

function Get-ClangdIndexDirs {
    param([string]$Root, [bool]$Recurse)

    $dirs = [System.Collections.Generic.List[string]]::new()
    $roots = if ($Recurse) { @($Root) } else {
        @($Root, (Join-Path $Root 'MCU')) | Where-Object { Test-Path -LiteralPath $_ }
    }

    foreach ($r in $roots) {
        $direct = Join-Path $r '.cache\clangd'
        if (Test-Path -LiteralPath $direct) {
            $dirs.Add((Resolve-Path -LiteralPath $direct).Path) | Out-Null
        }
    }

    if ($Recurse) {
        try {
            foreach ($d in [System.IO.Directory]::EnumerateDirectories($Root, '.cache', [System.IO.SearchOption]::AllDirectories)) {
                $candidate = Join-Path $d 'clangd'
                if ([System.IO.Directory]::Exists($candidate)) {
                    $dirs.Add((Resolve-Path -LiteralPath $candidate).Path) | Out-Null
                }
            }
        } catch {
            Write-Warning ("Skipped some paths while scanning index dirs: " + $_.Exception.Message)
        }
    }

    return $dirs | Sort-Object -Unique
}

Write-Host "WorkspaceRoot: $WorkspaceRoot"

if ($ClearIndex) {
    $indexDirs = Get-ClangdIndexDirs -Root $WorkspaceRoot -Recurse:$RecurseWorkspace
    if ($indexDirs.Count -eq 0) {
        Write-Host "No .cache/clangd found (try -RecurseWorkspace if your index lives deeper)."
    } else {
        foreach ($p in $indexDirs) {
            Write-Host "Removing index dir: $p"
            Remove-Item -LiteralPath $p -Recurse -Force
        }
    }
} else {
    Write-Host "Skipped index deletion (-Quick)."
}

if ($KillClangd -and -not $NoKill) {
    $procs = Get-Process -Name 'clangd' -ErrorAction SilentlyContinue
    if (-not $procs) {
        Write-Host "No running clangd process found."
    } else {
        Write-Warning "Stopping all clangd processes ($($procs.Count)); the editor usually restarts clangd automatically."
        $procs | Stop-Process -Force
        Write-Host "clangd stop requested."
    }
} elseif ($NoKill) {
    Write-Host "Skipped killing clangd (-NoKill). Run 'clangd: Restart language server' in the palette or save sources."
}

Write-Host "Done."
