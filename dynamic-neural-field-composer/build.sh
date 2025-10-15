#!/bin/bash

# Set environment variables
export PROJECT_ROOT=$(pwd)

echo "VCPKG_ROOT is set to: '$VCPKG_ROOT'"

# Check if the VCPKG_ROOT environment variable is set
if [ -z '$VCPKG_ROOT' ]; then
    echo "ERROR: The environment variable VCPKG_ROOT is not set."
    echo "Download and install VCPKG from https://github.com/microsoft/vcpkg#quick-start-windows."
    echo "Create an environment variable VCPKG_ROOT that points to the installation directory."
    read -p "Press any key to continue... " -n1 -s
    exit 1
fi

# Install packages using Vcpkg
sudo vcpkg install imgui[docking-experimental,core,opengl3-binding,glfw-binding]
sudo vcpkg install implot
sudo vcpkg install imgui-node-editor
sudo vcpkg install nlohmann-json
sudo vcpkg install catch2

# Integrate install might require elevation (sudo)
#sudo vcpkg integrate install

# Build your project 
cd "$PROJECT_ROOT/build"
cmake ..
make

# Optionally, you can run your executable here if applicable

read -p "Press any key to continue... " -n1 -s
exit 1