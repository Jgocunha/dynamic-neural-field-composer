@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

rem Repo root is three levels up from resources\benchmark. The venv lives there,
rem not under resources\, since CMakeLists.txt installs resources\ wholesale and
rem would otherwise ship a few hundred MB of venv with every release.
set VENV_DIR=..\..\..\.dashboard-venv
set PYTHON=

rem Prefer 3.13/3.12/3.11 over whatever "python"/the newest py resolves to --
rem a brand-new Python minor version is the most likely one to be missing
rem prebuilt wheels (pyarrow, a streamlit dependency, is usually the first to lag).
where py >nul 2>nul
if %errorlevel%==0 (
    for %%V in (3.13 3.12 3.11 3.10) do (
        if not defined PYTHON (
            py -%%V -c "" >nul 2>nul
            if !errorlevel!==0 set PYTHON=py -%%V
        )
    )
)
if not defined PYTHON (
    where python >nul 2>nul
    if %errorlevel%==0 (
        set PYTHON=python
    ) else (
        echo ERROR: no Python interpreter found on PATH.
        echo Install Python 3.11+ from https://www.python.org/downloads/ and try again.
        pause
        exit /b 1
    )
)

if not exist "%VENV_DIR%\Scripts\python.exe" (
    echo Creating virtual environment with: %PYTHON%
    %PYTHON% -m venv "%VENV_DIR%"
    if errorlevel 1 (
        echo ERROR: failed to create the virtual environment.
        pause
        exit /b 1
    )
)

set VENV_PY=%VENV_DIR%\Scripts\python.exe
set STAMP=%VENV_DIR%\.deps-installed

if not exist "%STAMP%" (
    echo Installing dependencies -- this happens once...
    "%VENV_PY%" -m pip install --quiet --upgrade pip
    "%VENV_PY%" -m pip install --quiet -r requirements.txt
    if errorlevel 1 (
        echo ERROR: pip install failed. See the output above.
        pause
        exit /b 1
    )
    echo installed> "%STAMP%"
)

echo Starting the benchmark dashboard...
"%VENV_PY%" -m streamlit run dashboard.py
if errorlevel 1 (
    echo.
    echo The dashboard exited with an error -- see above.
    pause
)
