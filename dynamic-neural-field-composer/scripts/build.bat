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

:: Snapshot VCPKG_ROOT before vcvars. vcvars64.bat overwrites VCPKG_ROOT to point at
:: Visual Studio's bundled vcpkg, which does not have our packages installed.
set "PROJECT_VCPKG_ROOT=%VCPKG_ROOT%"

:: Load the MSVC x64 toolchain. Ninja needs cl.exe on PATH; locating Visual Studio via
:: vswhere keeps this version-agnostic (works with whichever VS the machine/runner has).
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * ^
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 ^
    -property installationPath`) do set "VSINSTALL=%%i"
if not defined VSINSTALL ( echo ERROR: Visual Studio with C++ tools not found. & exit /b 1 )
call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 ( echo ERROR: failed to initialize MSVC environment. & exit /b 1 )

:: Create build folders
mkdir %PROJECT_ROOT%\build\x64-release
mkdir %PROJECT_ROOT%\build\x64-debug

:: Run CMake (Release)
cmake -G Ninja -S "%PROJECT_ROOT%" -B "%PROJECT_ROOT%\build\x64-release" ^
    -DCMAKE_TOOLCHAIN_FILE="%PROJECT_VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_PREFIX_PATH="%IPK_RELEASE%"

:: Build Release
cmake --build "%PROJECT_ROOT%\build\x64-release" --parallel

:: Run CMake (Debug)
cmake -G Ninja -S "%PROJECT_ROOT%" -B "%PROJECT_ROOT%\build\x64-debug" ^
    -DCMAKE_TOOLCHAIN_FILE="%PROJECT_VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_PREFIX_PATH="%IPK_DEBUG%"

:: Build Debug
cmake --build "%PROJECT_ROOT%\build\x64-debug" --parallel
