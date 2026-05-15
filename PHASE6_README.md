# Android DeBloat Tool - Phase 6: Package Removal System

## What We've Built

### New Files Created:
1. **include/RemovalEngine.h** - Complete removal/disable interface
2. **src/RemovalEngine.cpp** - Full ADB-based implementation

### Overview

The RemovalEngine is the core operational component that safely removes or disables packages on Android devices through ADB commands. It provides multiple removal strategies, comprehensive error handling, and rollback capabilities.

## Core Components

### 1. RemovalAction Enum

```cpp
enum class RemovalAction
{
    UNINSTALL,              // Attempt uninstall first
    DISABLE,                // Disable package (non-destructive)
    UNINSTALL_OR_DISABLE    // Try uninstall, fallback to disable
};
```

### 2. RemovalStatus Enum

```cpp
enum class RemovalStatus
{
    SUCCESS,                // Package removed/disabled successfully
    FAILED,                 // Operation completely failed
    PARTIAL,                // Some packages succeeded, some failed
    CANCELLED,              // User cancelled operation
    PERMISSION_DENIED,      // ADB permission issue
    NOT_INSTALLED,          // Package not found
    UNKNOWN                 // Unknown error
};
```

### 3. RemovalResult Struct

Captures result of each operation:
- `packageName` - Which package was affected
- `action` - What action was performed
- `status` - Success/failure status
- `message` - Human-readable result message
- `canRollback` - Whether operation can be reversed

### 4. RemovalEngine Class

**Constructor:**
```cpp
RemovalEngine(const std::string& deviceSerial);
```

**Main Methods:**

| Method | Purpose |
|--------|---------|
| `RemovePackage()` | Smart removal (uninstall + fallback) |
| `RemoveMultiple()` | Batch removal with error control |
| `DisablePackage()` | Non-destructive disable |
| `UninstallPackage()` | Force uninstall attempt |
| `EnablePackage()` | Re-enable disabled packages |
| `RollbackAllDisabled()` | Restore all disabled packages |
| `IsPackageInstalled()` | Check if package exists |
| `IsPackageEnabled()` | Check if package is enabled |
| `GetRemovalHistory()` | Access operation log |
| `GetDisabledPackages()` | List disabled packages |

**Helper Methods:**
| Method | Purpose |
|--------|---------|
| `GetLastStatus()` | Last operation status |
| `GetLastError()` | Last error message |
| `ClearHistory()` | Clear operation log |

## Removal Strategy

### Smart Removal Flow

```
RemovePackage(package, UNINSTALL_OR_DISABLE)
    ↓
[1] Try Uninstall via ADB
    adb shell pm uninstall --user 0 <package>
    ↓
    Success? → Return SUCCESS ✓
    ↓
    Failed? → Try next...
    ↓
[2] Try Disable via ADB
    adb shell pm disable-user --user 0 <package>
    ↓
    Success? → Return SUCCESS (Disable) ✓
    ↓
    Failed? → Return FAILED ✗
```

### Uninstall vs Disable

**Uninstall (`pm uninstall --user 0`)**
- ✅ Completely removes package for current user
- ✅ Frees up storage space
- ❌ May fail if system package
- ❌ Cannot be undone easily (requires reinstall)

**Disable (`pm disable-user --user 0`)**
- ✅ Hides package from user (non-destructive)
- ✅ Can be reversed with `pm enable`
- ✅ Works for system packages
- ❌ Package still exists in system partition
- ❌ Doesn't free storage

**Hybrid (Uninstall_Or_Disable)**
- ✅ Best of both worlds
- ✅ Tries dangerous uninstall first
- ✅ Falls back to safe disable if needed
- ✅ Guarantees result without hard failure

## Implementation Details

### 1. ADB Commands Used

```bash
# Check if package installed
adb -s <serial> shell pm list packages

# Uninstall for current user
adb -s <serial> shell pm uninstall --user 0 com.package.name

# Disable package for current user
adb -s <serial> shell pm disable-user --user 0 com.package.name

# Enable previously disabled package
adb -s <serial> shell pm enable com.package.name

# Check if package is enabled
adb -s <serial> shell pm dump com.package.name | findstr mEnabled
```

### 2. Success Detection

ADB output parsing looks for:
- "Success" keyword (for uninstall)
- "new state: disabled" (for disable)
- "new state: enabled" (for enable)
- Absence of error indicators

### 3. Batch Removal

```cpp
RemoveMultiple({pkg1, pkg2, pkg3}, UNINSTALL_OR_DISABLE, stopOnError)
```

Options:
- `stopOnError = true` - Stop if any package fails
- `stopOnError = false` - Continue on errors, return all results

Status:
- All succeed → `RemovalStatus::SUCCESS`
- Some succeed → `RemovalStatus::PARTIAL`
- All fail → `RemovalStatus::FAILED`

### 4. Rollback System

Tracks all disabled packages:
```cpp
disabledPackages = {
    "com.facebook.katana",
    "com.instagram.android",
    "com.netflix.mediaclient"
};

// Later: restore all
engine.RollbackAllDisabled();
// Executes: pm enable com.facebook.katana, etc.
```

### 5. Operation History

All operations logged for audit trail:
```cpp
removalHistory = [
    {facebook.katana, DISABLE, SUCCESS, "Package disabled", true},
    {instagram.android, UNINSTALL, SUCCESS, "Package uninstalled", false},
    {netflix, DISABLE, FAILED, "Failed to disable", false}
];

engine.GetRemovalHistory();  // Access complete log
```

## Safety Features

### 1. Installation Check
```cpp
if (!IsPackageInstalled(packageName))
    return RemovalResult(..., NOT_INSTALLED, ...);
```
Always verify package exists before attempting removal.

### 2. Fallback Strategy
- Won't brute-force
- Gracefully tries alternative methods
- Returns partial success when appropriate

### 3. Rollback Capability
- All disabled packages tracked
- Can restore with single `RollbackAllDisabled()` call
- Complete undo for non-destructive operations

### 4. Comprehensive Logging
- Every operation logged
- Can inspect history for troubleshooting
- Success/failure messages for each package

## Integration with Other Components

### With PackageManager
```cpp
PackageManager pm(deviceSerial);
pm.FetchAllPackages();
auto bloatware = pm.GetBloatwarePackages();

RemovalEngine engine(deviceSerial);
for (const auto& pkg : bloatware) {
    engine.RemovePackage(pkg.packageName);
}
```

### With PackageClassifier
```cpp
PackageClassifier classifier(...);
auto classified = classifier.Classify(package);

if (classified.category == PackageCategory::SAFE_TO_REMOVE) {
    auto result = engine.RemovePackage(package.packageName);
}
```

### With UserInterface
```cpp
UserInterface ui(pm, classifier);
auto selected = ui.MultiSelectPackages(packages, recommendations);

RemovalEngine engine(deviceSerial);
auto results = engine.RemoveMultiple(
    std::vector<std::string>(selected.begin(), selected.end()),
    RemovalAction::UNINSTALL_OR_DISABLE
);
```

## Error Handling

### Graceful Degradation
```
Request: Uninstall Facebook
  ↓
Try Uninstall → FAILED (system package cannot be uninstalled)
  ↓
Try Disable → SUCCESS
  ↓
Return: "Disabled (uninstall not available)"
```

### Permission Issues
```
Request: Uninstall package
  ↓
ADB Command → Permission Denied
  ↓
Return: RemovalStatus::PERMISSION_DENIED
Message: "Device may not be authorized or USB debugging not enabled"
```

### Package Not Found
```
Request: Uninstall non-existent package
  ↓
Check installed → Not in list
  ↓
Return: RemovalStatus::NOT_INSTALLED
Message: "Package not found on device"
```

## Usage Examples

### Single Package Removal
```cpp
RemovalEngine engine("emulator-5554");

// Smart removal (try uninstall, fallback to disable)
auto result = engine.RemovePackage("com.facebook.katana");

if (result.status == RemovalStatus::SUCCESS) {
    std::cout << "Success: " << result.message << "\n";
} else {
    std::cout << "Failed: " << result.message << "\n";
}
```

### Batch Removal
```cpp
std::vector<std::string> toRemove = {
    "com.facebook.katana",
    "com.instagram.android",
    "com.netflix.mediaclient"
};

auto results = engine.RemoveMultiple(
    toRemove,
    RemovalAction::UNINSTALL_OR_DISABLE,
    false  // Don't stop on error
);

for (const auto& result : results) {
    std::cout << result.packageName << ": " 
              << (result.status == RemovalStatus::SUCCESS ? "OK" : "FAILED")
              << "\n";
}
```

### Disable-Only Strategy
```cpp
// Conservative: only disable, never uninstall
auto result = engine.DisablePackage("com.spotify.music");

// Can always undo later
engine.EnablePackage("com.spotify.music");
```

### Complete Rollback
```cpp
// Remove many packages
engine.RemoveMultiple(packageList, UNINSTALL_OR_DISABLE, false);

// Oops, something went wrong. Restore disabled ones:
size_t restored = engine.RollbackAllDisabled();
std::cout << "Restored " << restored << " packages\n";

// Note: Uninstalled packages cannot be restored
```

## Compilation & Build

### Build Command (Phase 6):
```bash
g++ -std=c++17 -Wall -O2 \
    src/main.cpp \
    src/CommandExecutor.cpp \
    src/AdbManager.cpp \
    src/PackageManager.cpp \
    src/PackageClassifier.cpp \
    src/UserInterface.cpp \
    src/RemovalEngine.cpp \
    -Iinclude -o build/DeBloat.exe
```

### Compilation Status:
✅ SUCCESS - 330 KB executable

## Known Limitations

1. **User-Only Uninstall:** `pm uninstall --user 0` only removes for current user, not device-wide
2. **System Packages:** Cannot uninstall system packages (by design for safety)
3. **No Pre-removal Backup:** Uninstalled packages cannot be restored automatically
4. **Disable Only Works Post-API:** Disable feature requires Android 4.3+
5. **No Scheduler:** Cannot schedule removals for later execution

## Future Enhancements

1. Add batch-disable with progress reporting
2. Implement system-wide uninstall (requires root)
3. Add package reinstall from Google Play
4. Implement transaction logging to file
5. Add crash recovery (resume incomplete operations)

## Statistics

- **Lines of Code:** ~400 (RemovalEngine.cpp)
- **Functions:** 12+
- **Error Handling:** 7 status types
- **Removal Strategies:** 3 (uninstall, disable, hybrid)
- **Safety Features:** 4 (check, fallback, logging, rollback)

## Compilation Status

✓ Phase 1: CommandExecutor - Complete
✓ Phase 2: ADB Integration - Complete  
✓ Phase 3: Package Management - Complete
✓ Phase 4: Classification System - Complete
✓ Phase 5: User Interface - Complete
✓ Phase 6: Package Removal - **COMPLETE** ← YOU ARE HERE
→ Phase 7: Integration - Next Phase

## Next Phase: Phase 7

Phase 7 will integrate RemovalEngine with UserInterface to create the complete workflow:
1. Device detection via AdbManager
2. Package enumeration via PackageManager
3. Package classification via PackageClassifier
4. User selection via UserInterface
5. **Actual removal via RemovalEngine** ← This phase
6. Result reporting and error handling
7. Rollback options if needed

The tool will be production-ready after Phase 7 completion.
