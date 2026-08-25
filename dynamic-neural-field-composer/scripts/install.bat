@echo off

:: BatchGotAdmin
:-------------------------------------
REM  --> Check for permissions
    IF "%PROCESSOR_ARCHITECTURE%" EQU "amd64" (
>nul 2>&1 "%SYSTEMROOT%\SysWOW64\cacls.exe" "%SYSTEMROOT%\SysWOW64\config\system"
) ELSE (
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
)

REM --> If error flag set, we do not have admin.
if '%errorlevel%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    set params= %*
    echo UAC.ShellExecute "cmd.exe", "/c ""%~s0"" %params:"=""%", "", "runas", 1 >> "%temp%\getadmin.vbs"

    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit /B

:gotAdmin
    set SCRIPT_DIR=%~dp0
    set PROJECT_ROOT=%SCRIPT_DIR%..

:: Which configuration(s) to install. No argument (or "all") installs both, matching this
:: script's original behaviour. "release" or "debug" installs only that one -- matches
:: scripts\build.bat, which supports building just one configuration; installing "all"
:: after a single-configuration build would fail on the tree that was never built.
set "CONFIG=%~1"
if not defined CONFIG set "CONFIG=all"
if /i not "%CONFIG%"=="all" if /i not "%CONFIG%"=="release" if /i not "%CONFIG%"=="debug" (
    echo ERROR: unknown install configuration "%CONFIG%" ^(expected "release", "debug", or no argument for both^).
    exit /b 1
)

if /i not "%CONFIG%"=="debug" (
    REM Install x64-release configuration (Ninja single-config; no --config needed)
    cmake --build "%PROJECT_ROOT%\build\x64-release" --target install
)

if /i not "%CONFIG%"=="release" (
    REM Install x64-debug configuration
    cmake --build "%PROJECT_ROOT%\build\x64-debug" --target install
)

exit /b 0
