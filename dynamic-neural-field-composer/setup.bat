@echo off
setlocal EnableDelayedExpansion
set PROJECT_ROOT=%CD%

:: ── vcpkg ─────────────────────────────────────────────────────────────────────
if not defined VCPKG_ROOT (
    set VCPKG_ROOT=C:\tools\vcpkg
    echo VCPKG_ROOT not set. Installing vcpkg to !VCPKG_ROOT!...
    if not exist "!VCPKG_ROOT!" (
        git clone https://github.com/microsoft/vcpkg.git "!VCPKG_ROOT!"
        if errorlevel 1 ( echo ERROR: Failed to clone vcpkg. & pause & exit /b 1 )
        call "!VCPKG_ROOT!\bootstrap-vcpkg.bat" -disableMetrics
        if errorlevel 1 ( echo ERROR: Failed to bootstrap vcpkg. & pause & exit /b 1 )
    )
    setx VCPKG_ROOT "!VCPKG_ROOT!"
    echo VCPKG_ROOT set permanently to !VCPKG_ROOT!
    echo NOTE: Open a new terminal for VCPKG_ROOT to be visible to other tools.
)

:: ── vcpkg packages ────────────────────────────────────────────────────────────
echo Installing vcpkg packages...
"%VCPKG_ROOT%\vcpkg.exe" install ^
    "imgui[docking-experimental,core,opengl3-binding,glfw-binding,dx12-binding,win32-binding]:x64-windows" ^
    "implot:x64-windows" ^
    "imgui-node-editor:x64-windows" ^
    "nlohmann-json:x64-windows" ^
    "gtest:x64-windows" ^
    "catch2:x64-windows"
if errorlevel 1 ( echo ERROR: vcpkg install failed. & pause & exit /b 1 )

:: ── imgui-platform-kit ────────────────────────────────────────────────────────
set IPK_SRC=%PROJECT_ROOT%\deps\imgui-platform-kit
set IPK_INSTALL=%PROJECT_ROOT%\deps\ipk-install

if not exist "%IPK_SRC%" (
    echo Cloning imgui-platform-kit...
    git clone https://github.com/Jgocunha/imgui-platform-kit.git "%IPK_SRC%"
    if errorlevel 1 ( echo ERROR: Failed to clone imgui-platform-kit. & pause & exit /b 1 )
)

if not exist "%IPK_INSTALL%" (
    echo Building imgui-platform-kit...
    cmake -S "%IPK_SRC%\imgui-platform-kit" -B "%IPK_SRC%\build" ^
        -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
        -DCMAKE_BUILD_TYPE=Release ^
        -DCMAKE_INSTALL_PREFIX="%IPK_INSTALL%"
    if errorlevel 1 ( echo ERROR: imgui-platform-kit cmake configure failed. & pause & exit /b 1 )
    cmake --build "%IPK_SRC%\build" --config Release --parallel
    if errorlevel 1 ( echo ERROR: imgui-platform-kit build failed. & pause & exit /b 1 )
    cmake --install "%IPK_SRC%\build" --config Release
    if errorlevel 1 ( echo ERROR: imgui-platform-kit install failed. & pause & exit /b 1 )
) else (
    echo imgui-platform-kit already installed, skipping.
)

echo.
echo Setup complete. Run build.bat to build the project.
pause
