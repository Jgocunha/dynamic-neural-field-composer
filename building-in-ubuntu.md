# Building in Ubuntu 22.04

## Installing CMake

```bash
sudo apt update
sudo apt install cmake

sudo apt install build-essential
```

## Installing vcpkg

https://lindevs.com/install-vcpkg-on-ubuntu

```bash
sudo apt update
sudo apt install -y zip unzip

sudo apt install -y build-essential pkg-config
wget -qO vcpkg.tar.gz https://github.com/microsoft/vcpkg/archive/master.tar.gz

sudo mkdir /opt/vcpkg
sudo tar xf vcpkg.tar.gz --strip-components=1 -C /opt/vcpkg

sudo /opt/vcpkg/bootstrap-vcpkg.sh
sudo ln -s /opt/vcpkg/vcpkg /usr/local/bin/vcpkg

vcpkg version
```

Add vpkg root to your environment variables. Open your .bashrc and add:

```bash
export VCPKG_ROOT=/opt/vcpkg
```

### Installing Catch2 using vcpkg

```bash
sudo vcpkg install catch2
```

Using it in CMakeLists

```cmake
find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(main PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
```

### Installing imgui using vcpkg

```bash
sudo vcpkg install imgui[docking-experimental,core,opengl3-binding,glfw-binding]
```

Using it in CMakeLists

```cmake
find_package(imgui CONFIG REQUIRED)
target_link_libraries(main PRIVATE imgui::imgui)
```

### Installing implot using vcpkg

```bash
sudo vcpkg install implot
```

Using it in CMakeLists

```cmake
find_package(implot CONFIG REQUIRED)
target_link_libraries(main PRIVATE implot::implot)
```

### Installing nlohmann-json using vcpkg

```bash
sudo vcpkg install nlohmann-json
```

Using it in CMakeLists

```cmake
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json)
```

## Running imgui example_glfw_opengl3

### OpenGl

https://www.khronos.org/opengl/wiki/Getting_Started#Downloading_OpenGL

OpenGl should be installed by default if you have your graphics drivers installed.

Checking the version

```bash
sudo apt-get install mesa-utils
glxinfo | grep "OpenGL version"
```

### GLFW

https://shnoh171.github.io/gpu%20and%20gpu%20programming/2019/08/26/installing-glfw-on-ubuntu.html

```bash
sudo apt-get install libglfw3
sudo apt-get install libglfw3-dev
```

#### Testing

Go to imgui/examples/example_glfw_opengl3

```bash
make
./example_glfw_opengl3
```

The demo should be running.

## Changes in CMakeLists.txt

1. Remove resource files. Windows resource files (.rc) are used to embed metadata, icons, and other resources into Windows executables. These are not applicable on Linux.

2. Remove linking against DirectX libraries (d3d12.lib, dxgi.lib, d3dcompiler.lib), which are specific to Windows. 

## Building

```bash
mkdir build
cd build
cmake ..
make
```