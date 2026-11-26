#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

if [ "$(id -u)" != "0" ]; then
    echo "Requesting administrative privileges..."
    sudo "$0" "$@"
    exit $?
fi

case "$(uname -s)" in
    Darwin) BUILD_DIR="$PROJECT_ROOT/build/macos-release" ;;
    *)      BUILD_DIR="$PROJECT_ROOT/build/linux-release" ;;
esac

cmake --install "$BUILD_DIR"
