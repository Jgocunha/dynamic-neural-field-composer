@echo off
setlocal EnableDelayedExpansion

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%..

:: ── pinned revisions ──────────────────────────────────────────────────────────
set DEPS_ENV=%PROJECT_ROOT%\dependencies.env
if not exist "%DEPS_ENV%" ( echo ERROR: %DEPS_ENV% not found. & exit /b 1 )
for /f "usebackq eol=# tokens=1,* delims==" %%a in ("%DEPS_ENV%") do set "%%a=%%b"

:: ── vcpkg ─────────────────────────────────────────────────────────────────────
if not defined VCPKG_ROOT (
    set VCPKG_ROOT=C:\tools\vcpkg
    echo VCPKG_ROOT not set. Installing vcpkg to !VCPKG_ROOT!...
    if not exist "!VCPKG_ROOT!" (
        git clone https://github.com/microsoft/vcpkg.git "!VCPKG_ROOT!"
        if errorlevel 1 ( echo ERROR: Failed to clone vcpkg. & exit /b 1 )
        git -C "!VCPKG_ROOT!" checkout --quiet %VCPKG_COMMIT%
        if errorlevel 1 ( echo ERROR: Failed to check out pinned vcpkg revision. & exit /b 1 )
        call "!VCPKG_ROOT!\bootstrap-vcpkg.bat" -disableMetrics
        if errorlevel 1 ( echo ERROR: Failed to bootstrap vcpkg. & exit /b 1 )
    )
    setx VCPKG_ROOT "!VCPKG_ROOT!"
    echo VCPKG_ROOT set permanently to !VCPKG_ROOT!
    echo NOTE: Open a new terminal for VCPKG_ROOT to be visible to other tools.
)

call :check_pin "%VCPKG_ROOT%" "%VCPKG_COMMIT%" vcpkg

:: ── vcpkg packages ────────────────────────────────────────────────────────────
echo Installing vcpkg packages...
"%VCPKG_ROOT%\vcpkg.exe" install ^
    "imgui[docking-experimental,core,opengl3-binding,glfw-binding,dx12-binding,win32-binding]:x64-windows" ^
    "implot:x64-windows" ^
    "imgui-node-editor:x64-windows" ^
    "nlohmann-json:x64-windows" ^
    "gtest:x64-windows" ^
    "catch2:x64-windows" ^
    "fftw3:x64-windows"
if errorlevel 1 ( echo ERROR: vcpkg install failed. & exit /b 1 )

:: ── imgui-platform-kit ────────────────────────────────────────────────────────
set IPK_SRC=%PROJECT_ROOT%\deps\imgui-platform-kit
set IPK_INSTALL=%PROJECT_ROOT%\deps\ipk-install

if not exist "%IPK_SRC%" (
    echo Cloning imgui-platform-kit...
    git clone https://github.com/Jgocunha/imgui-platform-kit.git "%IPK_SRC%"
    if errorlevel 1 ( echo ERROR: Failed to clone imgui-platform-kit. & exit /b 1 )
    git -C "%IPK_SRC%" checkout --quiet %IPK_COMMIT%
    if errorlevel 1 ( echo ERROR: Failed to check out pinned imgui-platform-kit revision. & exit /b 1 )
)
call :check_pin "%IPK_SRC%" "%IPK_COMMIT%" imgui-platform-kit

if not exist "%IPK_INSTALL%\release" (
    echo Building imgui-platform-kit Release...
    cmake -S "%IPK_SRC%\imgui-platform-kit" -B "%IPK_SRC%\build-release" ^
        -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
        -DCMAKE_INSTALL_PREFIX="%IPK_INSTALL%\release"
    if errorlevel 1 ( echo ERROR: imgui-platform-kit Release configure failed. & exit /b 1 )
    cmake --build "%IPK_SRC%\build-release" --config Release --parallel
    if errorlevel 1 ( echo ERROR: imgui-platform-kit Release build failed. & exit /b 1 )
    cmake --install "%IPK_SRC%\build-release" --config Release
    if errorlevel 1 ( echo ERROR: imgui-platform-kit Release install failed. & exit /b 1 )
) else (
    echo imgui-platform-kit Release already installed, skipping.
)

if not exist "%IPK_INSTALL%\debug" (
    echo Building imgui-platform-kit Debug...
    cmake -S "%IPK_SRC%\imgui-platform-kit" -B "%IPK_SRC%\build-debug" ^
        -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
        -DCMAKE_INSTALL_PREFIX="%IPK_INSTALL%\debug"
    if errorlevel 1 ( echo ERROR: imgui-platform-kit Debug configure failed. & exit /b 1 )
    cmake --build "%IPK_SRC%\build-debug" --config Debug --parallel
    if errorlevel 1 ( echo ERROR: imgui-platform-kit Debug build failed. & exit /b 1 )
    cmake --install "%IPK_SRC%\build-debug" --config Debug
    if errorlevel 1 ( echo ERROR: imgui-platform-kit Debug install failed. & exit /b 1 )
) else (
    echo imgui-platform-kit Debug already installed, skipping.
)

echo.
echo Setup complete. Run scripts\build.bat to build the project.
exit /b 0

:: ── helpers ───────────────────────────────────────────────────────────────────
:: Report a drifted checkout instead of moving it. VCPKG_ROOT is usually a shared
:: tool other projects also build against, so the only copy we check out to the
:: pin is one we cloned ourselves.
:check_pin
set "HAVE="
for /f "delims=" %%h in ('git -C "%~1" rev-parse HEAD 2^>nul') do set "HAVE=%%h"
if not defined HAVE goto :eof
if /i not "!HAVE!"=="%~2" (
    echo WARNING: %~3 at %~1 is at !HAVE!,
    echo          but dependencies.env pins %~2.
    echo          CI builds against the pin, so local results may differ.
)
goto :eof
