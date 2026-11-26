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

:: ── vcpkg ─────────────────────────────────────────────────────────────────────
if not defined VCPKG_ROOT (
    set VCPKG_ROOT=C:\tools\vcpkg
    echo VCPKG_ROOT not set. Installing vcpkg to !VCPKG_ROOT!...
    if not exist "!VCPKG_ROOT!" (
        git clone https://github.com/microsoft/vcpkg.git "!VCPKG_ROOT!"
        if errorlevel 1 ( echo ERROR: Failed to clone vcpkg. & exit /b 1 )
        call "!VCPKG_ROOT!\bootstrap-vcpkg.bat" -disableMetrics
        if errorlevel 1 ( echo ERROR: Failed to bootstrap vcpkg. & exit /b 1 )
    )
    setx VCPKG_ROOT "!VCPKG_ROOT!"
    echo VCPKG_ROOT set permanently to !VCPKG_ROOT!
    echo NOTE: Open a new terminal for VCPKG_ROOT to be visible to other tools.
)

:: ── vcpkg baseline ────────────────────────────────────────────────────────────
:: Every dependency is declared in vcpkg.json and installed by CMake on the first
:: configure, at the versions of the vcpkg commit pinned there as builtin-baseline.
:: vcpkg reads those versions from its own git history, so a shallow or older clone
:: has to fetch that commit first.
set "BASELINE="
for /f "tokens=2 delims=:, " %%B in ('findstr /c:"builtin-baseline" "%PROJECT_ROOT%\vcpkg.json"') do set "BASELINE=%%~B"
if not defined BASELINE ( echo ERROR: no builtin-baseline found in vcpkg.json. & exit /b 1 )

git -C "%VCPKG_ROOT%" cat-file -e %BASELINE% 2>nul
if errorlevel 1 (
    echo vcpkg at %VCPKG_ROOT% does not have the pinned baseline %BASELINE% yet, fetching it...
    set "SHALLOW="
    for /f %%S in ('git -C "%VCPKG_ROOT%" rev-parse --is-shallow-repository 2^>nul') do set "SHALLOW=%%S"
    if "!SHALLOW!"=="true" (
        git -C "%VCPKG_ROOT%" fetch --unshallow origin
    ) else (
        git -C "%VCPKG_ROOT%" fetch origin
    )
    if errorlevel 1 (
        echo ERROR: could not fetch vcpkg commit %BASELINE% into %VCPKG_ROOT%.
        echo VCPKG_ROOT must be a git clone of https://github.com/microsoft/vcpkg that you can write to.
        exit /b 1
    )
)

echo.
echo Setup complete. Run scripts\build.bat to build the project.
echo The first build installs every dependency from vcpkg.json, which takes a while.
exit /b 0
