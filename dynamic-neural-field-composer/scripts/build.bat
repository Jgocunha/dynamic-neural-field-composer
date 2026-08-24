@echo off
setlocal EnableDelayedExpansion

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

:: Pin the VS install to whichever one last configured this tree. vswhere -latest can
:: pick a different Visual Studio than an existing build/x64-* tree was configured with;
:: vcvars then sets INCLUDE/LIB for that newly-"latest" VS while the cached
:: CMAKE_CXX_COMPILER still points at the old install's cl.exe, so headers and compiler
:: end up from different VS versions and every translation unit fails with
:: "STL1001: Unexpected compiler version". A fresh tree has no marker yet, so it falls
:: through to the vswhere lookup below exactly as before.
set "VSINSTALL_MARKER=%PROJECT_ROOT%\build\.vsinstall"
set "VSINSTALL="
if exist "%VSINSTALL_MARKER%" set /p VSINSTALL=<"%VSINSTALL_MARKER%"
if defined VSINSTALL if not exist "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat" set "VSINSTALL="

:: Load the MSVC x64 toolchain. Ninja needs cl.exe on PATH; locating Visual Studio via
:: vswhere keeps this version-agnostic (works with whichever VS the machine/runner has).
if not defined VSINSTALL set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not defined VSINSTALL for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * ^
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 ^
    -property installationPath`) do set "VSINSTALL=%%i"
if not defined VSINSTALL ( echo ERROR: Visual Studio with C++ tools not found. & exit /b 1 )

:: A pre-existing build\x64-release with no marker was configured by a build.bat that
:: predates VS pinning (or was configured by hand). Reusing it with the VSINSTALL just
:: detected above is only safe if that VS is the one CMake's cache already has recorded --
:: CACHED_CL uses forward slashes (CMake's own normalization) while VSINSTALL uses
:: backslashes (vswhere's format), so compare after normalizing VSINSTALL to match.
if not exist "%VSINSTALL_MARKER%" if exist "%PROJECT_ROOT%\build\x64-release\CMakeCache.txt" (
    set "CACHED_CL="
    for /f "usebackq tokens=1,* delims==" %%A in (`findstr /b "CMAKE_CXX_COMPILER:" "%PROJECT_ROOT%\build\x64-release\CMakeCache.txt"`) do set "CACHED_CL=%%B"
    set "VSINSTALL_FWD=!VSINSTALL:\=/!"
    echo !CACHED_CL!| findstr /b /i /c:"!VSINSTALL_FWD!" >nul
    if errorlevel 1 (
        echo ERROR: build\x64-release was already configured with a different Visual Studio
        echo   install than the one just detected: !VSINSTALL!
        echo   Reusing it here would fail with STL1001: Unexpected compiler version.
        echo   Delete build\x64-release and build\x64-debug for a clean reconfigure, or create
        echo   build\.vsinstall containing the VS install path that tree was built with, then
        echo   re-run this script.
        exit /b 1
    )
)

call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 ( echo ERROR: failed to initialize MSVC environment. & exit /b 1 )

if not exist "%PROJECT_ROOT%\build" mkdir "%PROJECT_ROOT%\build"
> "%VSINSTALL_MARKER%" echo !VSINSTALL!

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
