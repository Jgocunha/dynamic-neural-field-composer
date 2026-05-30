@echo off

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..
set IPK_INSTALL=%PROJECT_ROOT%\deps\ipk-install
set IPK_RELEASE=%IPK_INSTALL%\release
set IPK_DEBUG=%IPK_INSTALL%\debug

:: Check if the environment variable is set
IF NOT DEFINED VCPKG_ROOT (
    echo ERROR: The environment variable VCPKG_ROOT is not set.
    echo Run scripts\setup.bat first to install all dependencies automatically.
    exit /b 1
)

:: Create build folders
mkdir %PROJECT_ROOT%\build\x64-release
mkdir %PROJECT_ROOT%\build\x64-debug

:: Run CMake (Release)
cmake -G "Visual Studio 17 2022" -A x64 -S "%PROJECT_ROOT%" -B "%PROJECT_ROOT%\build\x64-release" ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_PREFIX_PATH="%IPK_RELEASE%"

:: Build Release
cmake --build "%PROJECT_ROOT%\build\x64-release" --config Release

:: Run CMake (Debug)
cmake -G "Visual Studio 17 2022" -A x64 -S "%PROJECT_ROOT%" -B "%PROJECT_ROOT%\build\x64-debug" ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_PREFIX_PATH="%IPK_DEBUG%"

:: Build Debug
cmake --build "%PROJECT_ROOT%\build\x64-debug" --config Debug
