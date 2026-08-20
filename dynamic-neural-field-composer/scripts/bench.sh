#!/bin/bash
# Wraps a perf-tool invocation (dnf_composer_benchmark, dnf_composer_deckbench,
# dnf_composer_profiler) with CPU pinning and elevated priority where available, then
# exports the resulting state as DNFC_BENCH_* environment variables so
# tests/common/bench_env.h can stamp it into the tool's JSON output.
#
# See .claude/performance-workplan/WP-07-machine-hygiene-scripts.md for the full
# rationale. This project's simulation is single-threaded (no OpenMP anywhere in the
# tree, FFTW is not built with threads) and runs on one socket, so pinning is a single
# CPU and NUMA binding is not a concern.
#
# Linux and macOS both land here (Windows uses scripts/bench.ps1) -- macOS has no
# taskset and unprivileged `nice` behaves differently, so every mechanism below
# degrades gracefully and records honestly what it could and could not do, rather
# than claiming a clean state it did not actually reach.
#
# Usage: scripts/bench.sh <exe> [args...]

set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <exe> [args...]" >&2
    exit 1
fi

EXE="$1"
shift

if [ ! -x "$EXE" ]; then
    echo "bench.sh: executable not found or not executable: $EXE" >&2
    exit 1
fi

# Core 2, not 0 -- core 0 is where many Linux systems concentrate interrupt/
# housekeeping work, which is exactly the kind of scheduling noise pinning exists to
# avoid. A single core is enough: the workload is single-threaded.
TASKSET_CORE=2

NICE_LEVEL=-20
NICE_PREFIX=()
if command -v nice >/dev/null 2>&1; then
    # `nice` silently caps the requested value without CAP_SYS_NICE rather than
    # failing, so this records what was REQUESTED, not a verified applied value.
    export DNFC_BENCH_PRIORITY="nice${NICE_LEVEL} (requested; actual value may be capped without elevated privileges)"
    NICE_PREFIX=(nice -n "$NICE_LEVEL")
else
    export DNFC_BENCH_PRIORITY="unrecorded (nice not found)"
fi

# Governor and transparent-hugepage state are RECORDED here, not changed -- setting
# either needs root via distro-specific mechanisms (cpupower / cpufreq-set / sysfs
# writes), which varies enough across Linux distros and is inapplicable on macOS that
# it is left to the operator. An honest read is worth more than a partial, brittle
# attempt to set them from this script.
GOVERNOR="unrecorded"
if command -v cpupower >/dev/null 2>&1; then
    GOVERNOR="$(cpupower frequency-info -p 2>/dev/null | sed -n 's/.*governor "\([^"]*\)".*/\1/p')"
    [ -z "$GOVERNOR" ] && GOVERNOR="unrecorded"
elif [ -r /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor ]; then
    GOVERNOR="$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo unrecorded)"
fi

THP="n/a"
if [ -r /sys/kernel/mm/transparent_hugepage/enabled ]; then
    THP="$(sed -n 's/.*\[\(.*\)\].*/\1/p' /sys/kernel/mm/transparent_hugepage/enabled)"
    [ -z "$THP" ] && THP="unrecorded"
fi

export DNFC_BENCH_POWER="governor=${GOVERNOR},thp=${THP}"

if command -v taskset >/dev/null 2>&1; then
    export DNFC_BENCH_AFFINITY="core${TASKSET_CORE}"
    exec taskset -c "$TASKSET_CORE" "${NICE_PREFIX[@]}" "$EXE" "$@"
else
    # macOS has no taskset and no per-process CPU-affinity API exposed to the shell
    # (only thread-affinity hints via thread_policy_set, unusable from here) -- run
    # unpinned and say so plainly rather than silently skipping the record.
    export DNFC_BENCH_AFFINITY="unrecorded (taskset not available on this platform)"
    exec "${NICE_PREFIX[@]}" "$EXE" "$@"
fi
