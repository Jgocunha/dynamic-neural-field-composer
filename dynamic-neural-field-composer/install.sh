#!/bin/bash

# Function to check if running as root
function check_root() {
    if [ "$(id -u)" != "0" ]; then
        echo "Requesting administrative privileges..."
        sudo "$0" "$@"  # Relaunch the script with sudo
        exit $?
    fi
}

# Check if we have root permissions, if not, acquire them.
check_root "$@"

# Get the script's current directory
PROJECT_ROOT="$(dirname "$(readlink -f "$0")")"

# Build your project 
cd "$PROJECT_ROOT/build"
sudo make install

read -p "Press any key to continue... " -n1 -s
exit 0
