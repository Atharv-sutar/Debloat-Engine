# Auto-Build and Run Script for DeBloat
# This script watches for changes and automatically rebuilds and runs the project

$projectPath = Split-Path -Parent $MyInvocation.MyCommand.Path
$srcPath = Join-Path $projectPath "src"
$includePath = Join-Path $projectPath "include"
$buildPath = Join-Path $projectPath "build"
$exePath = Join-Path $buildPath "DeBloat.exe"

# Compilation command
$compileCmd = "g++ -std=c++17 -Wall -O2 src/main_production.cpp src/CommandExecutor.cpp src/AdbManager.cpp src/PackageManager.cpp src/PackageClassifier.cpp src/UserInterface.cpp src/RemovalEngine.cpp src/Logger.cpp -Iinclude -o build/DeBloat.exe"

# Color functions for better output
function Write-Status {
    param([string]$Message, [string]$Type = "INFO")
    $colors = @{
        "INFO"    = "Cyan"
        "SUCCESS" = "Green"
        "ERROR"   = "Red"
        "WARNING" = "Yellow"
    }
    $color = $colors[$Type]
    Write-Host "[$Type] $Message" -ForegroundColor $color
}

function Compile-Project {
    Write-Status "Starting compilation..." "INFO"
    
    Set-Location $projectPath
    
    # Ensure previous executable is not running and delete it
    if (Test-Path $exePath) {
        try {
            $process = Get-Process -Name "DeBloat" -ErrorAction SilentlyContinue
            if ($process) { $process | Stop-Process -Force }
        } catch {}
        Remove-Item $exePath -Force -ErrorAction SilentlyContinue
    }
    # Run compilation
    $output = & cmd /c $compileCmd 2>&1
    
    if ($LASTEXITCODE -eq 0) {
        $fileSize = (Get-Item $exePath).Length / 1KB
        Write-Status "[OK] Compilation successful! ($([math]::Round($fileSize, 2)) KB)" "SUCCESS"
        return $true
    } else {
        Write-Status "[FAILED] Compilation failed!" "ERROR"
        Write-Host $output -ForegroundColor Red
        return $false
    }
}

function Run-Project {
    Write-Status "Launching application..." "INFO"
    Write-Host ""
    
    & $exePath
    
    Write-Host ""
    Write-Status "Application exited" "INFO"
}

function Watch-Files {
    param(
        [string]$Path,
        [scriptblock]$Action
    )
    
    $watcher = New-Object System.IO.FileSystemWatcher
    $watcher.Path = $Path
    $watcher.IncludeSubdirectories = $true
    $watcher.Filter = "*.*"

    # Debounce mechanism
    $script:lastChange = [datetime]::MinValue
    $debounceMs = 1000
    
    $handler = {
        $now = [datetime]::Now
        if (($now - $script:lastChange).TotalMilliseconds -gt $debounceMs) {
            $script:lastChange = $now
            & $Action
        }
    }
    
    $null = Register-ObjectEvent -InputObject $watcher -EventName "Changed" -Action $handler
    $null = Register-ObjectEvent -InputObject $watcher -EventName "Created" -Action $handler
    $null = Register-ObjectEvent -InputObject $watcher -EventName "Deleted" -Action $handler
    
    return $watcher
}

# Main loop
function Main {
    Clear-Host
    Write-Host "====================================================" -ForegroundColor Cyan
    Write-Host "      DeBloat Auto-Build & Run Monitor" -ForegroundColor Cyan
    Write-Host "====================================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Watching for changes in:" -ForegroundColor Cyan
    Write-Host "  - src/          (source files)" -ForegroundColor Gray
    Write-Host "  - include/      (header files)" -ForegroundColor Gray
    Write-Host ""
    Write-Host "The project will automatically:" -ForegroundColor Cyan
    Write-Host "  1. Detect file changes" -ForegroundColor Gray
    Write-Host "  2. Recompile the project" -ForegroundColor Gray
    Write-Host "  3. Run the application" -ForegroundColor Gray
    Write-Host ""
    Write-Host "Press Ctrl+C to stop monitoring" -ForegroundColor Yellow
    Write-Host "====================================================" -ForegroundColor Cyan
    Write-Host ""
    
    # Initial build on startup
    Write-Status "Performing initial build on startup..." "INFO"
    if (Compile-Project) {
        if (Test-Path $exePath) {
            Write-Host ""
            $continueRun = Read-Host "Run the application now? (y/n)"
            if ($continueRun -eq "y" -or $continueRun -eq "Y") {
                Run-Project
            }
        }
    } else {
        Write-Status "Fix compilation errors and save a file to rebuild" "WARNING"
    }
    
    Write-Host ""
    Write-Status "Watching for file changes..." "INFO"
    Write-Host ""
    
    # Watch for changes
    $action = {
        $file = $Event.SourceEventArgs.Name
        if ($file -match "\.(cpp|h)$|CMakeLists\.txt$") {
            Write-Host ""
            Write-Status "File changed: $file" "WARNING"
            Write-Host ""
            
            if (Compile-Project) {
                Write-Host ""
                Run-Project
            } else {
                Write-Status "Fix the errors and save again to rebuild" "ERROR"
            }
            
            Write-Host ""
            Write-Status "Watching for file changes..." "INFO"
            Write-Host ""
        }
    }
    
    # Create watchers for both directories
    $srcWatcher = New-Object System.IO.FileSystemWatcher
    $srcWatcher.Path = $srcPath
    $srcWatcher.IncludeSubdirectories = $false
    $srcWatcher.Filter = "*.cpp"
    
    $includeWatcher = New-Object System.IO.FileSystemWatcher
    $includeWatcher.Path = $includePath
    $includeWatcher.IncludeSubdirectories = $false
    $includeWatcher.Filter = "*.h"

    $rootWatcher = New-Object System.IO.FileSystemWatcher
    $rootWatcher.Path = $projectPath
    $rootWatcher.IncludeSubdirectories = $false
    $rootWatcher.Filter = "CMakeLists.txt"

    # Debounce to prevent multiple triggers
    $script:lastChange = [datetime]::MinValue
    $debounceMs = 1500
    
    $changeAction = {
        $now = [datetime]::Now
        if (($now - $script:lastChange).TotalMilliseconds -gt $debounceMs) {
            $script:lastChange = $now
            
            $file = $Event.SourceEventArgs.Name
            Write-Host ""
            Write-Status "File changed: $file" "WARNING"
            Write-Host ""
            
            if (Compile-Project) {
                Write-Host ""
                Run-Project
            } else {
                Write-Status "Fix the errors and save again to rebuild" "ERROR"
            }
            
            Write-Host ""
            Write-Status "Watching for file changes..." "INFO"
            Write-Host ""
        }
    }
    
    # Register watchers
    $null = Register-ObjectEvent -InputObject $srcWatcher -EventName "Changed" -Action $changeAction
    $null = Register-ObjectEvent -InputObject $srcWatcher -EventName "Created" -Action $changeAction
    $null = Register-ObjectEvent -InputObject $includeWatcher -EventName "Changed" -Action $changeAction
    $null = Register-ObjectEvent -InputObject $includeWatcher -EventName "Created" -Action $changeAction
    $null = Register-ObjectEvent -InputObject $rootWatcher -EventName "Changed" -Action $changeAction
    $null = Register-ObjectEvent -InputObject $rootWatcher -EventName "Created" -Action $changeAction
    
    # Keep the script running
    try {
        while ($true) {
            Start-Sleep -Seconds 1
        }
    }
    finally {
        $srcWatcher.Dispose()
        $includeWatcher.Dispose()
        Get-EventSubscriber | Unregister-Event
        Write-Status "Auto-build monitor stopped" "INFO"
    }
}

# Run main
Main
