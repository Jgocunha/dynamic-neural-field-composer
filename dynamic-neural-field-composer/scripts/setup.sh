#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# ── pinned revisions ──────────────────────────────────────────────────────────
# shellcheck source=../dependencies.env
. "$PROJECT_ROOT/dependencies.env"

# Report a drifted checkout instead of moving it. VCPKG_ROOT is usually a shared
# tool other projects also build against, so the only copy we check out to the
# pin is one we cloned ourselves.
check_pin() {
    local dir="$1" want="$2" name="$3"
    local have
    have="$(git -C "$dir" rev-parse HEAD 2>/dev/null)" || return 0
    if [ "$have" != "$want" ]; then
        echo "WARNING: $name at $dir is at $have,"
        echo "         but dependencies.env pins $want."
        echo "         CI builds against the pin, so local results may differ."
    fi
}

# ── vcpkg ─────────────────────────────────────────────────────────────────────
if [ -z "$VCPKG_ROOT" ]; then
    export VCPKG_ROOT="$HOME/vcpkg"
    echo "VCPKG_ROOT not set. Installing vcpkg to $VCPKG_ROOT..."
    if [ ! -d "$VCPKG_ROOT" ]; then
        git clone https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT"
        git -C "$VCPKG_ROOT" checkout --quiet "$VCPKG_COMMIT"
        "$VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics
    fi
    echo ""
    echo "Add the following line to your shell profile (~/.bashrc or ~/.zshrc) to persist VCPKG_ROOT:"
    echo "  export VCPKG_ROOT=$VCPKG_ROOT"
    echo ""
fi

check_pin "$VCPKG_ROOT" "$VCPKG_COMMIT" vcpkg

# ── triplet detection ─────────────────────────────────────────────────────────
OS=$(uname -s)
ARCH=$(uname -m)
if [ "$OS" = "Darwin" ]; then
    TRIPLET=$( [ "$ARCH" = "arm64" ] && echo "arm64-osx" || echo "x64-osx" )
else
    TRIPLET="x64-linux"
fi
echo "Using vcpkg triplet: $TRIPLET"

# ── vcpkg packages ────────────────────────────────────────────────────────────
echo "Installing vcpkg packages..."
"$VCPKG_ROOT/vcpkg" install \
    "imgui[docking-experimental,core,opengl3-binding,glfw-binding]:$TRIPLET" \
    "implot:$TRIPLET" \
    "imgui-node-editor:$TRIPLET" \
    "nlohmann-json:$TRIPLET" \
    "gtest:$TRIPLET" \
    "catch2:$TRIPLET" \
    "fftw3:$TRIPLET"

# ── imgui-platform-kit ────────────────────────────────────────────────────────
IPK_SRC="$PROJECT_ROOT/deps/imgui-platform-kit"
IPK_INSTALL="$PROJECT_ROOT/deps/ipk-install"

if [ ! -d "$IPK_SRC" ]; then
    echo "Cloning imgui-platform-kit..."
    git clone https://github.com/Jgocunha/imgui-platform-kit.git "$IPK_SRC"
    git -C "$IPK_SRC" checkout --quiet "$IPK_COMMIT"
fi
check_pin "$IPK_SRC" "$IPK_COMMIT" imgui-platform-kit

if [ ! -d "$IPK_INSTALL" ]; then
    echo "Building imgui-platform-kit..."
    PARALLEL=$( [ "$OS" = "Darwin" ] && sysctl -n hw.logicalcpu || nproc )
    cmake -S "$IPK_SRC/imgui-platform-kit" -B "$IPK_SRC/build" \
        -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
        -DVCPKG_TARGET_TRIPLET="$TRIPLET" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$IPK_INSTALL"
    cmake --build "$IPK_SRC/build" --parallel "$PARALLEL"
    cmake --install "$IPK_SRC/build"
else
    echo "imgui-platform-kit already installed, skipping."
fi

echo ""
echo "Setup complete. Run ./scripts/build.sh or ./scripts/build_macos.sh to build the project."
