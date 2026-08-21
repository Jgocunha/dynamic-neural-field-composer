#!/bin/bash
# Double-click (on macOS, via Finder's "run" association) or `./run_dashboard.sh`.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Repo root is three levels up from resources/benchmark. The venv lives there, not
# under resources/, since CMakeLists.txt installs resources/ wholesale and would
# otherwise ship a few hundred MB of venv with every release.
VENV_DIR="$SCRIPT_DIR/../../../.dashboard-venv"

PYTHON=""
for candidate in python3.13 python3.12 python3.11 python3.10 python3; do
    if command -v "$candidate" >/dev/null 2>&1; then
        PYTHON="$candidate"
        break
    fi
done
if [ -z "$PYTHON" ]; then
    echo "ERROR: no Python interpreter found on PATH." >&2
    echo "Install Python 3.11+ and try again." >&2
    read -p "Press Enter to close..."
    exit 1
fi

if [ ! -x "$VENV_DIR/bin/python" ]; then
    echo "Creating virtual environment with: $PYTHON"
    "$PYTHON" -m venv "$VENV_DIR"
fi

VENV_PY="$VENV_DIR/bin/python"
STAMP="$VENV_DIR/.deps-installed"

if [ ! -f "$STAMP" ]; then
    echo "Installing dependencies -- this happens once..."
    "$VENV_PY" -m pip install --quiet --upgrade pip
    "$VENV_PY" -m pip install --quiet -r requirements.txt
    touch "$STAMP"
fi

echo "Starting the benchmark dashboard..."
"$VENV_PY" -m streamlit run dashboard.py
status=$?
if [ $status -ne 0 ]; then
    echo ""
    echo "The dashboard exited with an error -- see above."
    read -p "Press Enter to close..."
fi
