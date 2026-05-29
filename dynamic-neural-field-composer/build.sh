#!/bin/bash
set -e

export PROJECT_ROOT=$(pwd)
IPK_INSTALL="$PROJECT_ROOT/deps/ipk-install"

echo "VCPKG_ROOT is set to: '$VCPKG_ROOT'"

if [ -z "$VCPKG_ROOT" ]; then
    echo "ERROR: The environment variable VCPKG_ROOT is not set."
    echo "Run ./setup.sh first to install all dependencies automatically."
    exit 1
fi

# Install vcpkg packages
"$VCPKG_ROOT/vcpkg" install \
    "imgui[docking-experimental,core,opengl3-binding,glfw-binding]:x64-linux" \
    "implot:x64-linux" \
    "imgui-node-editor:x64-linux" \
    "nlohmann-json:x64-linux" \
    "gtest:x64-linux" \
    "catch2:x64-linux"

# Build the project
mkdir -p "$PROJECT_ROOT/build/linux-release"
cmake -S "$PROJECT_ROOT" -B "$PROJECT_ROOT/build/linux-release" \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$IPK_INSTALL"
cmake --build "$PROJECT_ROOT/build/linux-release" --parallel "$(nproc)"