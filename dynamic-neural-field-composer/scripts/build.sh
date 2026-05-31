#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
IPK_INSTALL="$PROJECT_ROOT/deps/ipk-install"

echo "VCPKG_ROOT is set to: '$VCPKG_ROOT'"

if [ -z "$VCPKG_ROOT" ]; then
    echo "ERROR: The environment variable VCPKG_ROOT is not set."
    echo "Run ./scripts/setup.sh first to install all dependencies automatically."
    exit 1
fi

# Build the project
mkdir -p "$PROJECT_ROOT/build/linux-release"
cmake -S "$PROJECT_ROOT" -B "$PROJECT_ROOT/build/linux-release" \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$IPK_INSTALL"
cmake --build "$PROJECT_ROOT/build/linux-release" --parallel "$(nproc)"
