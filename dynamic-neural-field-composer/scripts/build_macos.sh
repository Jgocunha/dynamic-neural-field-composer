#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
IPK_INSTALL="$PROJECT_ROOT/deps/ipk-install"

# setup.sh can only export VCPKG_ROOT into its own process, so the very first build
# in the same shell as setup would otherwise fail here. Fall back to the location
# setup.sh installs vcpkg to, but only if it actually holds a vcpkg binary.
if [ -z "$VCPKG_ROOT" ] && [ -x "$HOME/vcpkg/vcpkg" ]; then
    export VCPKG_ROOT="$HOME/vcpkg"
    echo "VCPKG_ROOT is not set; falling back to setup.sh's default install at $VCPKG_ROOT."
fi

if [ -z "$VCPKG_ROOT" ]; then
    echo "ERROR: The environment variable VCPKG_ROOT is not set."
    echo "Run ./scripts/setup.sh first to install all dependencies automatically."
    exit 1
fi

echo "VCPKG_ROOT is set to: '$VCPKG_ROOT'"

ARCH=$(uname -m)
if [ "$ARCH" = "arm64" ]; then
    VCPKG_TRIPLET="arm64-osx"
else
    VCPKG_TRIPLET="x64-osx"
fi

echo "Detected architecture: $ARCH, using vcpkg triplet: $VCPKG_TRIPLET"

BUILD_DIR="$PROJECT_ROOT/build/macos-release"
mkdir -p "$BUILD_DIR"

cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET="$VCPKG_TRIPLET" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$IPK_INSTALL"

cmake --build "$BUILD_DIR" --parallel "$(sysctl -n hw.logicalcpu)"
