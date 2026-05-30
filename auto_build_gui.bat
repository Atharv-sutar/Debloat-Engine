@echo off
setlocal enabledelayedexpansion

cd /d "%~dp0"

REM Auto build and run script for DeBloat GUI
set "BUILD_DIR=build_gui"
set "TARGET=DeBloatGui"
set "EXE_NAME=DeBloatGui.exe"
set "EXE_PATH=%BUILD_DIR%\%EXE_NAME%"

where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] cmake is not installed or not on PATH.
    echo Install CMake and try again.
    exit /b 1
)

where ninja >nul 2>&1
if errorlevel 1 (
    echo [ERROR] ninja is not installed or not on PATH.
    echo Install ninja and try again.
    exit /b 1
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

if exist "%BUILD_DIR%\CMakeCache.txt" (
    echo [INFO] Removing stale CMake cache from previous build generator...
    rmdir /s /q "%BUILD_DIR%\CMakeFiles" >nul 2>&1
    del /q "%BUILD_DIR%\CMakeCache.txt" >nul 2>&1
)

echo [INFO] Configuring project with Ninja...
cmake -S . -B "%BUILD_DIR%" -G "Ninja"
if errorlevel 1 (
    echo [ERROR] CMake configuration failed.
    exit /b 1
)

echo [INFO] Building %TARGET%...
cmake --build "%BUILD_DIR%" --config Release --target %TARGET%
if errorlevel 1 (
    echo [ERROR] Build failed.
    exit /b 1
)

if exist "%BUILD_DIR%\Release\%EXE_NAME%" (
    set "EXE_PATH=%BUILD_DIR%\Release\%EXE_NAME%"
) else if exist "%BUILD_DIR%\%EXE_NAME%" (
    set "EXE_PATH=%BUILD_DIR%\%EXE_NAME%"
) else (
    echo [ERROR] Built executable not found.
    exit /b 1
)

echo [SUCCESS] Build completed.
echo [INFO] Running %EXE_PATH%...
start "DeBloat GUI" "%EXE_PATH%"

endlocal
