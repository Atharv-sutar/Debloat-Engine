# DeBloat Auto-Build & Run Guide

## Overview
The auto-build system automatically compiles and runs your DeBloat project whenever you make changes to source files. No need to manually recompile!

## Files

### 1. `auto-build.ps1` (PowerShell Script)
The main auto-build engine that:
- Watches for changes in `src/` and `include/` directories
- Automatically recompiles when `.cpp` or `.h` files change
- Runs the compiled application after successful build
- Displays colored status messages
- Handles build errors gracefully

### 2. `auto-build.bat` (Batch Launcher)
Windows batch file that launches the PowerShell script with proper permissions.

## Quick Start

### Option 1: Using the Batch Launcher (Easiest)
1. Double-click `auto-build.bat` in Explorer
2. The auto-build monitor will start watching for changes
3. Edit any `.cpp` or `.h` file and save
4. The project will automatically rebuild and run!

### Option 2: Manual PowerShell
```powershell
cd "e:\All Programs\Atharv C++\DeBloat"
powershell -ExecutionPolicy Bypass -File "auto-build.ps1"
```

### Option 3: From VS Code Terminal
```powershell
.\auto-build.bat
```

## What Happens

### Initial Run
```
[INFO] Performing initial build on startup...
[SUCCESS] ✓ Compilation successful! (437.28 KB)

Run the application now? (y/n): y

[INFO] Launching application...
================================================
        Android DeBloat Tool v1.0
```

### When You Make Changes
```
[INFO] File changed: UserInterface.cpp

[INFO] Starting compilation...
[SUCCESS] ✓ Compilation successful! (437.28 KB)

[INFO] Launching application...
================================================
        Android DeBloat Tool v1.0
```

## Features

✅ **Automatic Detection**
  - Monitors `src/` for `.cpp` files
  - Monitors `include/` for `.h` files
  - 1.5 second debounce to prevent multiple rebuilds

✅ **Smart Compilation**
  - Full C++17 compilation with `-std=c++17 -Wall -O2`
  - Shows file size after successful build
  - Displays compilation errors clearly

✅ **Automatic Execution**
  - Runs the application immediately after build
  - Press Ctrl+C in app to return to monitor
  - Monitor resumes watching for changes

✅ **Error Handling**
  - Catches compilation errors
  - Shows error messages in red
  - Continues watching for fixes

✅ **Color-Coded Output**
  - 🟦 INFO (Cyan) - General information
  - 🟩 SUCCESS (Green) - Successful operations
  - 🟥 ERROR (Red) - Compilation/runtime errors
  - 🟨 WARNING (Yellow) - Important notices

## Stopping the Monitor

Press `Ctrl+C` to stop the auto-build monitor at any time.

```
^C
[INFO] Auto-build monitor stopped
```

## Example Workflow

1. **Start the monitor:**
   ```bash
   .\auto-build.bat
   ```

2. **Open your code editor** (VS Code, Notepad++, etc.)

3. **Edit a file** (e.g., `src/main.cpp`)

4. **Save the file** - Auto-build automatically:
   - Detects the change
   - Recompiles the project
   - Runs the new executable
   - Shows any errors

5. **Fix errors** and save again - cycle repeats!

6. **Stop monitoring** by pressing `Ctrl+C`

## Troubleshooting

### Script doesn't run (PowerShell execution policy error)
**Solution:** Use `auto-build.bat` instead, or run:
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Changes not detected
**Possible causes:**
- File not in `src/` or `include/` directory
- File extension not `.cpp` or `.h`
- Still watching (check console output)

**Solution:** Make sure you're editing files in the right directories

### Compilation takes a while
- Full compilation (all 8 files) typically takes 2-3 seconds
- This is normal for C++ with `-Wall -O2`
- Subsequent changes only recompile modified files

### Too many rebuilds triggered
- The debounce timer is set to 1.5 seconds
- Wait at least 1.5 seconds between saves if debugging
- This prevents accidental double-compiles

## Tips & Tricks

### Monitor Multiple Projects
Run multiple `auto-build.ps1` instances in different terminals for different projects

### Integration with Version Control
- The auto-build script doesn't interfere with Git/SVN
- Safe to use alongside version control systems
- Compiled binaries remain in `build/` directory

### Performance
- Watching for changes is lightweight (~5% CPU)
- Compilation uses full CPU during rebuild (~80-100%)
- Application runs in foreground (press Ctrl+C to return)

### Development Workflow
1. Start auto-build before coding session
2. Keep terminal visible in background
3. Edit code, save, auto-build rebuilds
4. See results immediately
5. Fix errors as needed

## File Structure

```
DeBloat/
├── auto-build.ps1        ← PowerShell auto-build engine
├── auto-build.bat        ← Batch launcher (run this!)
├── src/                  ← Monitored for changes (*.cpp)
├── include/              ← Monitored for changes (*.h)
├── build/
│   └── DeBloat.exe       ← Auto-compiled executable
└── CMakeLists.txt
```

## Advanced Usage

### Customize Compilation Flags
Edit line in `auto-build.ps1`:
```powershell
$compileCmd = "g++ -std=c++17 -Wall -O2 ..."
```

Example with debug symbols:
```powershell
$compileCmd = "g++ -std=c++17 -Wall -g -O0 ..."
```

### Change Debounce Time
Edit line in `auto-build.ps1`:
```powershell
$debounceMs = 1500  # milliseconds
```

Lower = faster rebuilds (but may cause double-compiles)
Higher = more stable (but slower response)

### Monitor Additional Directories
Add more watchers in the Main function:
```powershell
$dataWatcher = New-Object System.IO.FileSystemWatcher
$dataWatcher.Path = Join-Path $projectPath "data"
$null = Register-ObjectEvent -InputObject $dataWatcher -EventName "Changed" -Action $changeAction
```

## Support

For issues with the auto-build system:
1. Check the error messages in colored output
2. Verify g++ is installed: `g++ --version`
3. Check file paths in `auto-build.ps1`
4. Ensure proper permissions on project directory

## Benefits

⚡ **Faster Development** - No manual recompilation
🔍 **Instant Feedback** - See changes immediately
🛡️ **Error Catching** - Catch compilation errors early
😊 **Peace of Mind** - Automated workflow reduces errors
📊 **Build Status** - Always know if project compiles
