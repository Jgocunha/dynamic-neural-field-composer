@echo off
:: Wrap the whole script as a subroutine so every exit /b below returns here instead
:: of closing the window outright -- a double-clicked .bat otherwise closes its console
:: the instant it hits exit /b, taking any error message with it before it can be read.
:: Skipped when CI is set (GitHub Actions sets it automatically for every run) so
:: automated invocations never block waiting on a keypress.
call :main %*
set "EXITCODE=%ERRORLEVEL%"
if not defined CI (
    echo.
    pause
)
exit /b %EXITCODE%

:main
setlocal EnableDelayedExpansion

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..
set IPK_INSTALL=%PROJECT_ROOT%\deps\ipk-install
set IPK_RELEASE=%IPK_INSTALL%\release
set IPK_DEBUG=%IPK_INSTALL%\debug

:: Check if the environment variable is set. setup.bat persists VCPKG_ROOT with setx,
:: which only reaches shells opened afterwards, so the very first build in the same
:: terminal as setup would otherwise fail here. Fall back to the location setup.bat
:: installs vcpkg to, but only if it actually holds a vcpkg.exe.
IF NOT DEFINED VCPKG_ROOT (
    if exist "C:\tools\vcpkg\vcpkg.exe" (
        set "VCPKG_ROOT=C:\tools\vcpkg"
        echo VCPKG_ROOT is not set; falling back to setup.bat's default install at C:\tools\vcpkg.
    ) else (
        echo ERROR: The environment variable VCPKG_ROOT is not set.
        echo Run scripts\setup.bat first to install all dependencies automatically.
        exit /b 1
    )
)

:: Snapshot VCPKG_ROOT before vcvars. vcvars64.bat overwrites VCPKG_ROOT to point at
:: Visual Studio's bundled vcpkg, which does not have our packages installed.
set "PROJECT_VCPKG_ROOT=%VCPKG_ROOT%"

:: Which configuration(s) to build. No argument (or "all") builds both Release and
:: Debug, matching this script's original behaviour, so existing callers and the wiki
:: instructions keep working unchanged. "release" or "debug" builds only that one.
:: Parsed here, before the stale-cache preflight below, so that preflight only inspects
:: the tree(s) actually selected.
set "CONFIG=%~1"
if not defined CONFIG set "CONFIG=all"
if /i not "%CONFIG%"=="all" if /i not "%CONFIG%"=="release" if /i not "%CONFIG%"=="debug" (
    echo ERROR: unknown build configuration "%CONFIG%" ^(expected "release", "debug", or no argument for both^).
    exit /b 1
)

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

:: A pre-existing build\x64-release or build\x64-debug with no marker was configured by a
:: build.bat that predates VS pinning (or was configured by hand). Reusing either with the
:: VSINSTALL just detected above is only safe if that VS is the one CMake's cache already
:: has recorded -- CACHED_CL uses forward slashes (CMake's own normalization) while
:: VSINSTALL uses backslashes (vswhere's format), so compare after normalizing VSINSTALL to
:: match. Only the tree(s) selected by CONFIG are checked.
if not exist "%VSINSTALL_MARKER%" if /i not "%CONFIG%"=="debug" if exist "%PROJECT_ROOT%\build\x64-release\CMakeCache.txt" (
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

if not exist "%VSINSTALL_MARKER%" if /i not "%CONFIG%"=="release" if exist "%PROJECT_ROOT%\build\x64-debug\CMakeCache.txt" (
    set "CACHED_CL="
    for /f "usebackq tokens=1,* delims==" %%A in (`findstr /b "CMAKE_CXX_COMPILER:" "%PROJECT_ROOT%\build\x64-debug\CMakeCache.txt"`) do set "CACHED_CL=%%B"
    set "VSINSTALL_FWD=!VSINSTALL:\=/!"
    echo !CACHED_CL!| findstr /b /i /c:"!VSINSTALL_FWD!" >nul
    if errorlevel 1 (
        echo ERROR: build\x64-debug was already configured with a different Visual Studio
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

if /i not "%CONFIG%"=="debug" (
    REM Create build folder
    if not exist "%PROJECT_ROOT%\build\x64-release" mkdir "%PROJECT_ROOT%\build\x64-release"

    REM Run CMake (Release)
    cmake -G Ninja -S "%PROJECT_ROOT%" -B "%PROJECT_ROOT%\build\x64-release" ^
        -DCMAKE_TOOLCHAIN_FILE="%PROJECT_VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ^
        -DCMAKE_BUILD_TYPE=Release ^
        -DCMAKE_PREFIX_PATH="%IPK_RELEASE%"
    if errorlevel 1 exit /b 1

    REM Build Release
    cmake --build "%PROJECT_ROOT%\build\x64-release" --parallel
    if errorlevel 1 exit /b 1
)

if /i not "%CONFIG%"=="release" (
    REM Create build folder
    if not exist "%PROJECT_ROOT%\build\x64-debug" mkdir "%PROJECT_ROOT%\build\x64-debug"

    REM Run CMake Debug. CMAKE_MSVC_DEBUG_INFORMATION_FORMAT switches MSVC debug info from
    REM the default /Zi to /Z7, which sccache (see ci.yml) can cache -- /Zi cannot, since
    REM its PDB is written incrementally and shared across translation units. This project
    REM declares CMake 3.20 as its minimum; the variable is only honored on CMake 3.25+, so
    REM on older CMake it is silently ignored and Debug keeps the default /Zi -- harmless,
    REM just without the sccache benefit.
    cmake -G Ninja -S "%PROJECT_ROOT%" -B "%PROJECT_ROOT%\build\x64-debug" ^
        -DCMAKE_TOOLCHAIN_FILE="%PROJECT_VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ^
        -DCMAKE_BUILD_TYPE=Debug ^
        -DCMAKE_MSVC_DEBUG_INFORMATION_FORMAT=Embedded ^
        -DCMAKE_PREFIX_PATH="%IPK_DEBUG%"
    if errorlevel 1 exit /b 1

    REM Build Debug
    cmake --build "%PROJECT_ROOT%\build\x64-debug" --parallel
    if errorlevel 1 exit /b 1
)

exit /b 0
