@echo off

set VCPKG_ROOT=%VCPKG_ROOT%
set PROJECT_ROOT=%CD%
set IPK_INSTALL=%PROJECT_ROOT%\deps\ipk-install

:: Check if the environment variable is set
IF NOT DEFINED VCPKG_ROOT (
    echo ERROR: The environment variable VCPKG_ROOT is not set.
    echo Run setup.bat first to install all dependencies automatically.
    pause
    exit /b 1
)

:: Install vcpkg packages
"%VCPKG_ROOT%\vcpkg.exe" install ^
    "imgui[docking-experimental,core,opengl3-binding,glfw-binding,dx12-binding,win32-binding]:x64-windows" ^
    "implot:x64-windows" ^
    "imgui-node-editor:x64-windows" ^
    "nlohmann-json:x64-windows" ^
    "gtest:x64-windows" ^
    "catch2:x64-windows"

:: Create build folders
mkdir %PROJECT_ROOT%\build\x64-release
mkdir %PROJECT_ROOT%\build\x64-debug

:: Run CMake (Release)
cmake -G "Visual Studio 17 2022" -A x64 -S "%PROJECT_ROOT%" -B "%PROJECT_ROOT%\build\x64-release" ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_PREFIX_PATH="%IPK_INSTALL%"

:: Build Release
cmake --build "%PROJECT_ROOT%\build\x64-release" --config Release

:: Run CMake (Debug)
cmake -G "Visual Studio 17 2022" -A x64 -S "%PROJECT_ROOT%" -B "%PROJECT_ROOT%\build\x64-debug" ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_PREFIX_PATH="%IPK_INSTALL%"

:: Build Debug
cmake --build "%PROJECT_ROOT%\build\x64-debug" --config Debug

pause


