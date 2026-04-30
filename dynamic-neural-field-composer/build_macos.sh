#!/bin/bash
set -e

export PROJECT_ROOT=$(pwd)

echo "VCPKG_ROOT is set to: '$VCPKG_ROOT'"

if [ -z "$VCPKG_ROOT" ]; then
    echo "ERROR: The environment variable VCPKG_ROOT is not set."
    echo "Download and install VCPKG from https://github.com/microsoft/vcpkg#quick-start-unix."
    echo "Create an environment variable VCPKG_ROOT that points to the installation directory."
    exit 1
fi

ARCH=$(uname -m)
if [ "$ARCH" = "arm64" ]; then
    VCPKG_TRIPLET="arm64-osx"
else
    VCPKG_TRIPLET="x64-osx"
fi

echo "Detected architecture: $ARCH, using vcpkg triplet: $VCPKG_TRIPLET"

"$VCPKG_ROOT/vcpkg" install \
    "imgui[docking-experimental,core,opengl3-binding,glfw-binding]:$VCPKG_TRIPLET" \
    "implot:$VCPKG_TRIPLET" \
    "imgui-node-editor:$VCPKG_TRIPLET" \
    "nlohmann-json:$VCPKG_TRIPLET" \
    "gtest:$VCPKG_TRIPLET"

# ── imgui-platform-kit (required dependency, built from source) ────────────────
IPK_SRC="$PROJECT_ROOT/imgui-platform-kit-src"
IPK_INSTALL="$PROJECT_ROOT/imgui-platform-kit-install"

if [ ! -d "$IPK_SRC" ]; then
    git clone https://github.com/Jgocunha/imgui-platform-kit.git "$IPK_SRC"
fi

cmake -S "$IPK_SRC/imgui-platform-kit" \
    -B "$IPK_SRC/build" \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET="$VCPKG_TRIPLET" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$IPK_INSTALL"

cmake --build "$IPK_SRC/build" --parallel "$(sysctl -n hw.logicalcpu)"
cmake --install "$IPK_SRC/build"

# ── Main project ───────────────────────────────────────────────────────────────
BUILD_DIR="$PROJECT_ROOT/build/macos-release"
mkdir -p "$BUILD_DIR"

cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET="$VCPKG_TRIPLET" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$IPK_INSTALL"

cmake --build "$BUILD_DIR" --parallel "$(sysctl -n hw.logicalcpu)"
