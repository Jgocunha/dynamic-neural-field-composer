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

# Build the project
mkdir -p "$PROJECT_ROOT/build/linux-release"
cmake -S "$PROJECT_ROOT" -B "$PROJECT_ROOT/build/linux-release" \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$IPK_INSTALL"
cmake --build "$PROJECT_ROOT/build/linux-release" --parallel "$(nproc)"
