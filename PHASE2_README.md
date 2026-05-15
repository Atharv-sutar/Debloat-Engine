# Android DeBloat Tool - Phase 2: ADB Integration

## What We've Built

### New Files Created:
1. **include/AdbManager.h** - Complete ADB management interface
2. **src/AdbManager.cpp** - ADB implementation with all core functionality
3. **Updated src/main.cpp** - Phase 2 test suite

### Core Features Implemented:

#### 1. **AdbManager Class (Singleton Pattern)**
   - Single instance manages all ADB operations
   - Thread-safe initialization
   - Centralized error handling

#### 2. **ADB Installation Detection**
   - Searches PATH using `where adb.exe`
   - Checks common Android SDK locations:
     - `C:\Android\platform-tools\`
     - `C:\Program Files\Android\platform-tools\`
     - `%LOCALAPPDATA%\Android\Sdk\platform-tools\`
     - Environment variable expansion support

#### 3. **ADB Server Management**
   - `StartAdbServer()` - Starts ADB server if not running
   - `KillAdbServer()` - Stops the ADB server
   - `IsAdbServerRunning()` - Checks server status
   - Auto-wait mechanism for server startup

#### 4. **Device Detection**
   - `GetConnectedDevices()` - Lists all connected devices
   - Parses `adb devices -l` output
   - Extracts device info:
     - Serial number
     - Authorization status
     - Model
     - Product
     - Device name

#### 5. **Authorization Handling**
   - Enum: `AuthStatus` with 4 states
     - `AUTHORIZED` - Device ready for commands
     - `UNAUTHORIZED` - Waiting for user approval
     - `OFFLINE` - Device not reachable
     - `UNKNOWN` - Status unclear
   - Method: `GetDeviceAuthStatus()` - Query individual device status
   - Method: `ParseAuthStatus()` - Parse ADB status strings

#### 6. **Device Properties**
   - `GetDeviceProperty()` - Query any device property
   - Uses `adb shell getprop <property>`
   - Examples:
     - `ro.build.version.release` - Android version
     - `ro.build.id` - Build ID
     - `ro.product.manufacturer` - Manufacturer

#### 7. **Error Handling**
   - `GetLastError()` - Retrieve detailed error messages
   - Every operation validates prerequisites
   - Graceful degradation if ADB not found

## Compilation

### Build Command:
```bash
g++ -std=c++17 -Wall -O2 src/main.cpp src/CommandExecutor.cpp src/AdbManager.cpp -Iinclude -o build/DeBloat.exe
```

### Or use the build.bat script:
```cmd
cd e:\All Programs\Atharv C++\DeBloat
.\build.bat
```

## Testing Phase 2

### Test Results Breakdown:

**TEST 1: ADB Installation**
```
[TEST 1] Checking ADB Installation...
  ADB Installed: YES/NO
  ADB Path: <path to adb.exe or error message>
```

**TEST 2: ADB Server Status**
```
[TEST 2] Checking ADB Server Status...
  ADB Server Running: YES/NO
  (Auto-starts if not running)
```

**TEST 3: Device Detection**
```
[TEST 3] Detecting Connected Devices...
  Devices Found: <number>
  Connected Devices:
  Serial Number    |    Status       |    Model    |    Device
  FA8K20005345     |    AUTHORIZED   |    Pixel 3  |    blueline
```

**TEST 4: Device Properties**
```
[TEST 4] Querying Device Properties...
  Device: FA8K20005345
  Getting properties...
    Android Version: 12
    Build ID: TP1A.220624.014
    Manufacturer: Google
```

## Usage Example for Phase 3+

```cpp
#include "AdbManager.h"

int main()
{
    AdbManager& adb = AdbManager::GetInstance();
    
    // Check ADB is available
    if (!adb.IsAdbInstalled()) {
        std::cerr << adb.GetLastError() << std::endl;
        return 1;
    }
    
    // Start server
    if (!adb.StartAdbServer()) {
        std::cerr << adb.GetLastError() << std::endl;
        return 1;
    }
    
    // Get devices
    auto devices = adb.GetConnectedDevices();
    
    // For each authorized device
    for (const auto& device : devices) {
        if (device.authStatus == AuthStatus::AUTHORIZED) {
            // Query packages (Phase 3)
            // ...
        }
    }
    
    return 0;
}
```

## Important Notes

### Prerequisites for Full Testing:
1. **Android SDK Platform-Tools** installed
   - Download: https://developer.android.com/studio/releases/platform-tools
   - Add to PATH or install in default location

2. **Android Device** connected via USB
   - Settings > Developer Options > USB Debugging (ON)
   - Authorize when prompted

3. **USB Cable** for device connection

### Common Issues & Solutions:

| Issue | Cause | Solution |
|-------|-------|----------|
| "ADB not installed" | ADB not in PATH | Install Android SDK Platform-Tools |
| "Device offline" | USB connection issue | Reconnect cable, restart ADB: `adb kill-server` |
| "Unauthorized" | Not approved on device | Tap "OK" on device authorization prompt |
| "Command timeout" | Slow device | Increase sleep duration in StartAdbServer() |

## Architecture Patterns Used

### 1. **Singleton Pattern**
- Single global instance of AdbManager
- Thread-safe creation
- Access via `AdbManager::GetInstance()`

### 2. **RAII (Resource Acquisition Is Initialization)**
- Pipes managed via smart pointers
- Automatic cleanup in CommandExecutor

### 3. **Error Reporting**
- Non-exception error handling
- `GetLastError()` for debugging
- Exit codes for batch operations

### 4. **Enum for Type Safety**
- AuthStatus instead of string comparisons
- Compile-time type checking

## Code Statistics

- **Lines of Code**: ~350 (AdbManager.cpp/h)
- **Methods**: 11 public, 3 private
- **Structs**: 1 (AndroidDevice)
- **Enums**: 1 (AuthStatus)
- **Error Handling**: Comprehensive with fallback paths

## Next Steps: Phase 3

In Phase 3: Package Management, we will:
- Create `PackageManager` class
- Implement `adb shell pm list packages` parsing
- Store packages in data structures
- Prepare for classification system
- Add package filtering and searching

Run: `DeBloat.exe` to proceed with Phase 2 testing!
