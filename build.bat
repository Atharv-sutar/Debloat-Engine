@echo off
REM DeBloat Build Script for Windows
REM This script compiles the project using available C++ compiler

setlocal enabledelayedexpansion

cd /d "%~dp0"

REM Create build directory
if not exist build mkdir build

REM Check for compiler availability
set "CL_PATH="
set "GPP_PATH="
set "CLANG_PATH="
for /f "delims=" %%P in ('where cl.exe 2^>nul') do set "CL_PATH=%%P"
for /f "delims=" %%P in ('where g++.exe 2^>nul') do set "GPP_PATH=%%P"
for /f "delims=" %%P in ('where clang.exe 2^>nul') do set "CLANG_PATH=%%P"

if defined GPP_PATH (
    echo [*] Found G++ compiler
    set COMPILER=g++
    goto compile_gcc
)

if defined CLANG_PATH (
    echo [*] Found Clang compiler
    set COMPILER=clang
    goto compile_clang
)

if defined CL_PATH (
    echo [*] Found MSVC compiler (cl.exe)
    set COMPILER=msvc
    goto compile_msvc
)

echo [ERROR] No C++ compiler found in PATH!
echo Please install Visual Studio, MinGW, or Clang
exit /b 1

:compile_msvc
echo [*] Compiling with MSVC...
cl.exe /std:c++17 /W4 /EHsc /O2 ^
    src/main_production.cpp src/CommandExecutor.cpp src/AdbManager.cpp src/PackageManager.cpp src/PackageClassifier.cpp src/UserInterface.cpp src/RemovalEngine.cpp src/Logger.cpp ^
    /Iinclude ^
    /Fe:build/DeBloat.exe
if %errorlevel% equ 0 (
    echo [SUCCESS] Build completed: build/DeBloat.exe
    goto end
) else (
    echo [ERROR] MSVC compilation failed
    exit /b 1
)

:compile_gcc
echo [*] Compiling with G++...
g++.exe -std=c++17 -Wall -Wextra -O2 ^
    src/main_production.cpp src/CommandExecutor.cpp src/AdbManager.cpp src/PackageManager.cpp src/PackageClassifier.cpp src/UserInterface.cpp src/RemovalEngine.cpp src/Logger.cpp ^
    -Iinclude ^
    -o build/DeBloat.exe
if %errorlevel% equ 0 (
    echo [SUCCESS] Build completed: build/DeBloat.exe
    goto end
) else (
    echo [ERROR] G++ compilation failed
    exit /b 1
)

:compile_clang
echo [*] Compiling with Clang...
clang++.exe -std=c++17 -Wall -Wextra -O2 ^
    src/main_production.cpp src/CommandExecutor.cpp src/AdbManager.cpp src/PackageManager.cpp src/PackageClassifier.cpp src/UserInterface.cpp src/RemovalEngine.cpp src/Logger.cpp ^
    -Iinclude ^
    -o build/DeBloat.exe
if %errorlevel% equ 0 (
    echo [SUCCESS] Build completed: build/DeBloat.exe
    goto end
) else (
    echo [ERROR] Clang compilation failed
    exit /b 1
)

:end
endlocal
