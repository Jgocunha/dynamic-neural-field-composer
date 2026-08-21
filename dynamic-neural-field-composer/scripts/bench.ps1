<#
.SYNOPSIS
    Wraps a perf-tool invocation (dnf_composer_benchmark, dnf_composer_deckbench,
    dnf_composer_profiler) with CPU pinning, elevated priority and a fixed-clock power
    plan, then exports the resulting state as DNFC_BENCH_* environment variables so
    tests/common/bench_env.h can stamp it into the tool's JSON output.

.DESCRIPTION
    See .claude/performance-workplan/WP-07-machine-hygiene-scripts.md for the full
    rationale. Two things specific to this project make hygiene simpler than the
    general HPC case: the simulation is single-threaded (no OpenMP anywhere in the
    tree, FFTW is not built with threads), and it runs on one socket, so NUMA binding
    is not a concern -- only CPU affinity, priority and clock stability are.

    Pinning is a single logical CPU (affinity mask 0x1). One bit always selects
    exactly one logical processor and never its SMT sibling, regardless of which
    physical core it maps to or how many CCXs the CPU has, so this is safe on any
    x86 topology without querying it first.

    Uses Start-Process -PassThru -NoNewWindow rather than cmd's `start /affinity
    ... /wait`: on this system `start /wait` does NOT reliably propagate the started
    application's exit code as cmd's own ERRORLEVEL despite documentation claiming it
    does (verified directly: a child exiting 7 came back as 0) -- which would have
    silently turned every `--check` regression the wrapper ran into a false pass.
    -NoNewWindow keeps the child's stdout/stderr attached to this console (confirmed),
    and the returned Process object's .ExitCode is the one actually observed here.

    ASLR is deliberately left alone. Linux's `setarch -R` has no per-process Windows
    equivalent; the only lever is `/DYNAMICBASE:NO` linked into the bench target,
    which is a build-time change to a bench-only target. Judged not worth it --
    accept slightly wider variance instead of a permanent link-flag special case.

    DNFC_BENCH_AFFINITY / DNFC_BENCH_PRIORITY are set BEFORE the child is spawned (a
    child snapshots its parent's environment at creation time, so setting them after
    Start-Process would never reach it). Priority and affinity are then applied to the
    started Process object immediately after launch -- a genuine brief race against the
    child's own startup, harmless for a real multi-second benchmark run. This means the
    env vars are recorded optimistically rather than verified: pinning your own
    just-spawned child needs no special privilege on Windows, so failure is the rare
    case, and is flagged with Write-Warning on stderr rather than silently accepted --
    but a failure there will NOT retroactively correct what the child already inherited.

.PARAMETER Exe
    Path to the perf tool to run (e.g. build\release\tests\dnf_composer_benchmark.exe).

.PARAMETER ExeArgs
    Every remaining argument is passed through to Exe unchanged.

.EXAMPLE
    scripts\bench.ps1 build\release\tests\dnf_composer_benchmark.exe 2000 5

.EXAMPLE
    scripts\bench.ps1 build\release\tests\dnf_composer_deckbench.exe --check
#>
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Exe,

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ExeArgs
)

if (-not (Test-Path $Exe)) {
    Write-Error "bench.ps1: executable not found: $Exe"
    exit 1
}
$ResolvedExe = (Resolve-Path $Exe).Path

$HighPerfGuid = '8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c'
$originalGuid = $null
try {
    $schemeLine = (powercfg /getactivescheme) -join "`n"
    if ($schemeLine -match '([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12})') {
        $originalGuid = $Matches[1]
    }
}
catch {
    # powercfg itself is missing or unusable -- proceed unpinned on clocks and say so.
}

# PROCTHROTTLEMAX below is written INTO the High Performance scheme, so it outlives this
# script: restoring only the active-scheme GUID would leave that plan permanently capped
# at 99% for every later use of the machine. Capture the value first so it can be put back.
$originalThrottle = $null
try {
    $throttleQuery = (powercfg /query $HighPerfGuid SUB_PROCESSOR PROCTHROTTLEMAX) -join "`n"
    if ($throttleQuery -match 'Current AC Power Setting Index:\s*0x([0-9a-fA-F]+)') {
        $originalThrottle = [Convert]::ToInt32($Matches[1], 16)
    }
}
catch {
    # Leave $originalThrottle null -- the throttle is then not modified at all below,
    # rather than modified with no way back.
}

$powerRestored = $false
function Restore-Power {
    if ($script:powerRestored) { return }
    if ($null -ne $script:originalThrottle) {
        powercfg /setacvalueindex $script:HighPerfGuid SUB_PROCESSOR PROCTHROTTLEMAX $script:originalThrottle *> $null
    }
    if ($script:originalGuid) {
        powercfg /setactive $script:originalGuid *> $null
    }
    $script:powerRestored = $true
}
# Ctrl-C during the wrapped run must still restore the original power plan.
Register-EngineEvent -SourceIdentifier PowerShell.Exiting -Action { Restore-Power } | Out-Null

try {
    powercfg /setactive $HighPerfGuid *> $null
    if ($null -ne $originalThrottle) {
        # Caps turbo so clocks stay closer to base across runs -- large, unpredictable
        # boost swings are one of the biggest sources of run-to-run noise on a desktop
        # CPU with no dedicated cooling headroom. Only applied when the pre-existing
        # value was readable, so Restore-Power can always put it back.
        powercfg /setacvalueindex $HighPerfGuid SUB_PROCESSOR PROCTHROTTLEMAX 99 *> $null
        powercfg /setactive $HighPerfGuid *> $null
    }

    # Record what is actually in effect, read back rather than assumed: powercfg reports
    # failure through its exit code, not an exception, so a bare try/catch would happily
    # stamp a clean-looking value onto a run that never got one.
    $applied = (powercfg /query $HighPerfGuid SUB_PROCESSOR PROCTHROTTLEMAX) -join "`n"
    if ($applied -match 'Current AC Power Setting Index:\s*0x([0-9a-fA-F]+)') {
        $env:DNFC_BENCH_POWER = "high-performance,PROCTHROTTLEMAX=$([Convert]::ToInt32($Matches[1], 16))"
    }
    else {
        $env:DNFC_BENCH_POWER = 'high-performance,PROCTHROTTLEMAX=unverified'
    }
}
catch {
    # /setacvalueindex needs elevation on some systems -- degrade gracefully. An
    # unrecorded run is honest; a falsely "clean" one is the failure mode to avoid.
    $env:DNFC_BENCH_POWER = "unrecorded (powercfg failed: $($_.Exception.Message))"
}

# Set BEFORE spawning -- a child process snapshots its parent's environment at creation
# time, so setting these after Start-Process (as an earlier version of this script did)
# would never reach the child at all. Recorded optimistically: pinning your own
# just-spawned child needs no special privilege on Windows, unlike the powercfg calls
# above, so failure here is the rare case -- caught below and flagged loudly on stderr
# rather than silently reflected back into the (already-inherited) env var.
$env:DNFC_BENCH_AFFINITY = '0x1'
$env:DNFC_BENCH_PRIORITY = 'high'

$exitCode = 1
try {
    $proc = Start-Process -FilePath $ResolvedExe -ArgumentList $ExeArgs -NoNewWindow -PassThru

    # Set, then read back: assignment can appear to succeed and still not stick (the
    # child may already have exited on a very short run). The env vars the child
    # inherited cannot be corrected retroactively, so a mismatch is reported loudly here
    # -- treat such a run's recorded hygiene state as unverified.
    try {
        $proc.ProcessorAffinity = [IntPtr]1
        if (-not $proc.HasExited -and [int]$proc.ProcessorAffinity -ne 1) {
            Write-Warning "bench.ps1: CPU affinity read back as $([int]$proc.ProcessorAffinity), not 0x1 -- this run is NOT pinned as its JSON claims."
        }
    }
    catch { Write-Warning "bench.ps1: could not set CPU affinity ($($_.Exception.Message)) -- this run is NOT actually pinned despite DNFC_BENCH_AFFINITY=0x1 in its JSON." }

    try {
        $proc.PriorityClass = 'High'
        if (-not $proc.HasExited -and $proc.PriorityClass -ne 'High') {
            Write-Warning "bench.ps1: priority read back as $($proc.PriorityClass), not High -- this run is NOT elevated as its JSON claims."
        }
    }
    catch { Write-Warning "bench.ps1: could not set process priority ($($_.Exception.Message)) -- this run is NOT actually elevated despite DNFC_BENCH_PRIORITY=high in its JSON." }

    $proc.WaitForExit()
    $exitCode = $proc.ExitCode
}
finally {
    Restore-Power
}

exit $exitCode
