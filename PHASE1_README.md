# Android DeBloat Tool - Phase 1: Project Setup

## Folder Structure
```
DeBloat/
├── CMakeLists.txt
├── include/
│   └── CommandExecutor.h
├── src/
│   ├── main.cpp
│   └── CommandExecutor.cpp
└── build/          (will be created during compilation)
```

## Prerequisites
- Visual Studio 2019 or later (or any C++ compiler with C++17 support)
- CMake 3.16 or later
- ADB (Android Debug Bridge) - will be checked in Phase 2

## Compilation Instructions

### Using Visual Studio (Recommended for Windows)

1. **Open Command Prompt or PowerShell** in the project root (e:\All Programs\Atharv C++\DeBloat)

2. **Generate Visual Studio project:**
   ```
   cmake -B build -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Release
   ```
   (Replace "Visual Studio 16 2019" with your version if different)

3. **Build the project:**
   ```
   cmake --build build --config Release
   ```

4. **Run the executable:**
   ```
   .\build\Release\DeBloat.exe
   ```

### Using CMake with Ninja (Alternative)

1. **Generate build files:**
   ```
   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   ```

2. **Build:**
   ```
   cmake --build build
   ```

3. **Run:**
   ```
   .\build\DeBloat.exe
   ```

## Expected Output (Phase 1)

When you run DeBloat.exe, you should see:

```
================================================
        Android DeBloat Tool v1.0
        Windows C++ Console Application
================================================

[INFO] Phase 1: Project Setup Complete
[INFO] CommandExecutor utility is ready

[TEST] Testing CommandExecutor with simple command...

Command executed successfully: YES
Output:
Hello from DeBloat!

Exit code: 0

[TEST] Testing line splitting...
Split into 1 lines:
  [1] Hello from DeBloat!

================================================
[INFO] Phase 1 test complete!
       Ready to move to Phase 2: ADB Integration
================================================
```

## What We've Built

### CommandExecutor Class
- **Execute()**: Run a command and capture its output
- **ExecuteQuiet()**: Run a command without output capture
- **GetLastExitCode()**: Get the exit code of the last command
- **SplitLines()**: Parse command output into lines

This utility will be the backbone for all ADB command execution in future phases.

## Next Steps

When ready, move to **Phase 2: ADB Integration** where we will:
- Detect ADB installation
- Check ADB server status
- Start ADB server if needed
- Detect connected devices
