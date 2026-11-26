#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# ── vcpkg ─────────────────────────────────────────────────────────────────────
if [ -z "$VCPKG_ROOT" ]; then
    export VCPKG_ROOT="$HOME/vcpkg"
    echo "VCPKG_ROOT not set. Installing vcpkg to $VCPKG_ROOT..."
    if [ ! -d "$VCPKG_ROOT" ]; then
        git clone https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT"
        "$VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics
    fi
    echo ""
    echo "Add the following line to your shell profile (~/.bashrc or ~/.zshrc) to persist VCPKG_ROOT:"
    echo "  export VCPKG_ROOT=$VCPKG_ROOT"
    echo ""
fi

# ── vcpkg baseline ────────────────────────────────────────────────────────────
# Every dependency is declared in vcpkg.json and installed by CMake on the first
# configure, at the versions of the vcpkg commit pinned there as builtin-baseline.
# vcpkg reads those versions from its own git history, so a shallow or older clone
# has to fetch that commit first.
BASELINE=$(sed -n 's/.*"builtin-baseline": *"\([0-9a-f]*\)".*/\1/p' "$PROJECT_ROOT/vcpkg.json")
if [ -z "$BASELINE" ]; then
    echo "ERROR: no builtin-baseline found in $PROJECT_ROOT/vcpkg.json."
    exit 1
fi
if ! git -C "$VCPKG_ROOT" cat-file -e "$BASELINE^{commit}" 2>/dev/null; then
    echo "vcpkg at $VCPKG_ROOT does not have the pinned baseline $BASELINE yet, fetching it..."
    if [ "$(git -C "$VCPKG_ROOT" rev-parse --is-shallow-repository 2>/dev/null)" = "true" ]; then
        git -C "$VCPKG_ROOT" fetch --unshallow origin
    else
        git -C "$VCPKG_ROOT" fetch origin
    fi || {
        echo "ERROR: could not fetch vcpkg commit $BASELINE into $VCPKG_ROOT."
        echo "VCPKG_ROOT must be a git clone of https://github.com/microsoft/vcpkg that you can write to."
        exit 1
    }
fi

echo ""
echo "Setup complete. Run ./scripts/build.sh or ./scripts/build_macos.sh to build the project."
echo "The first build installs every dependency from vcpkg.json, which takes a while."
