@echo off
REM Auto-Build and Run Script Launcher for DeBloat
REM This batch file runs the PowerShell auto-build script with proper execution policy

setlocal enabledelayedexpansion
cd /d "%~dp0"

echo.
echo ╔════════════════════════════════════════════════════════╗
echo ║        DeBloat Auto-Build and Run Launcher            ║
echo ╚════════════════════════════════════════════════════════╝
echo.

REM Run PowerShell script with bypass execution policy
powershell -ExecutionPolicy Bypass -File "auto-build.ps1"

pause
