@echo off
REM DeBloat Build Script for Windows
REM This script compiles the project using available C++ compiler

setlocal enabledelayedexpansion

cd /d "%~dp0"

if /I "%~1"=="gui" goto build_gui

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

:build_gui
where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake is required to build the GUI target but was not found in PATH.
    exit /b 1
)
if not exist build_cmake mkdir build_cmake

set "CMAKE_GENERATOR="
set "CMAKE_GENERATOR_ARGS="

for /f "delims=" %%P in ('where nmake 2^>nul') do set "CMAKE_GENERATOR=NMake Makefiles"
if defined CMAKE_GENERATOR goto use_cmake_generator

for /f "delims=" %%P in ('where mingw32-make 2^>nul') do set "CMAKE_GENERATOR=MinGW Makefiles"
if defined CMAKE_GENERATOR goto use_cmake_generator

for /f "delims=" %%P in ('where ninja 2^>nul') do set "CMAKE_GENERATOR=Ninja"
if defined CMAKE_GENERATOR goto use_cmake_generator

echo [ERROR] No supported CMake build tool found for GUI build.
echo Install nmake, mingw32-make, or ninja and try again.
exit /b 1

:use_cmake_generator
cmake -S . -B build_cmake -G "%CMAKE_GENERATOR%" %CMAKE_GENERATOR_ARGS%
if errorlevel 1 (
    echo [ERROR] CMake configuration failed.
    exit /b 1
)
cmake --build build_cmake --config Release --target DeBloatGui
if errorlevel 1 (
    echo [ERROR] GUI build failed.
    exit /b 1
)
echo [SUCCESS] GUI build completed: build_cmake\Release\DeBloatGui.exe

goto end

:end
endlocal
