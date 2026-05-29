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
