# Android DeBloat Tool - Phase 5: User Interface

## What We're Building

### New Files Created:
1. **include/UserInterface.h** - Complete CLI user interface
2. **src/UserInterface.cpp** - Full implementation with menus, selection, and display

### Overview

A comprehensive command-line interface providing:
- Main menu with device status
- Package browsing by category
- Multi-select with visual feedback
- Search functionality
- Detailed package information display
- Removal confirmation with safety warnings

## Core Components

### 1. MenuOption Enum

```cpp
enum class MenuOption
{
    SHOW_ALL_PACKAGES,           // Browse all packages
    SHOW_CRITICAL_PACKAGES,      // View DO NOT TOUCH packages
    SHOW_SAFE_TO_REMOVE,         // View recommended removals
    SHOW_OPTIONAL_PACKAGES,      // View user choice packages
    SEARCH_PACKAGES,             // Search by name
    VIEW_PACKAGE_DETAILS,        // Get detailed info
    SELECT_PACKAGES_TO_REMOVE,   // Multi-select mode
    CONFIRM_REMOVAL,             // Review & confirm
    EXIT                         // Quit program
};
```

### 2. UserInterface Class

**Constructor:**
```cpp
UserInterface(PackageManager& packageMgr, PackageClassifier& classifier);
```

**Main Methods:**

| Method | Purpose |
|--------|---------|
| `ShowMainMenu()` | Display menu and get user choice |
| `ShowPackageList()` | Display list with category filtering |
| `ShowCategorizedPackages()` | Show packages in specific category |
| `ShowPackageDetails()` | Display full package information |
| `MultiSelectPackages()` | Interactive multi-select interface |
| `ConfirmRemoval()` | Review selections with safety info |
| `SearchAndSelectPackage()` | Search and select single package |

**Helper Methods:**

| Method | Purpose |
|--------|---------|
| `ClearScreen()` | Clear console (Windows/Linux) |
| `ShowHeader()` | Display section header |
| `ShowSeparator()` | Draw horizontal line |
| `ShowInfo()` | Info message |
| `ShowWarning()` | Warning message |
| `ShowError()` | Error message |
| `ShowSuccess()` | Success message |
| `PauseForUser()` | Wait for user input |
| `GetIntInput()` | Get integer from user (with validation) |
| `GetStringInput()` | Get string from user |
| `GetUserConfirmation()` | Yes/No confirmation |

### 3. Main Menu Flow

```
1. Display Header
2. Show Device Info
3. Show Available Options:
   - [1] Show All Packages
   - [2] Show Critical Packages
   - [3] Show Safe to Remove
   - [4] Show Optional Packages
   - [5] Search Packages
   - [6] View Package Details
   - [7] Select Packages to Remove
   - [8] Confirm & Remove
   - [9] Exit
4. Get User Selection
5. Execute Selected Operation
```

### 4. Package List Display

**Format:**
```
Index | Package Name                      | Category          | Safety Score
------|----------------------------------|-------------------|---------------
  1   | com.android.settings              | DO NOT TOUCH      | 0
  2   | com.facebook.katana               | SAFE TO REMOVE    | 95
  3   | com.google.android.apps.maps      | OPTIONAL          | 60
```

**Features:**
- Pagination (20 items per page)
- Category filtering
- Safety score display
- Total count
- Page navigation

### 5. Multi-Select Interface

**Interactive Selection:**
- Display packages with indices
- User enters indices to select/deselect
- Real-time feedback with [X] and [ ] markers
- View summary before confirmation
- Navigate with Page Up/Down

**Example:**
```
[1] Select Packages to Remove (SAFE TO REMOVE only)
   [ ] 1. com.facebook.katana
   [ ] 2. com.instagram.android
   [X] 3. com.netflix.mediaclient
   
Enter indices to toggle (comma-separated, or 'q' to finish): 1,2
   [X] 1. com.facebook.katana
   [X] 2. com.instagram.android
   [X] 3. com.netflix.mediaclient
```

### 6. Package Details Display

```
Package: com.facebook.katana
Display Name: Facebook
Type: User-installed
Enabled: Yes

Classification:
  Category: SAFE TO REMOVE
  Safety Score: 95/100
  Can Disable: YES
  
  Reason: Known bloatware - resource intensive
  Description: Facebook app - common bloatware
  
Installation Info:
  Version Code: 12345
  Version Name: 1.23.45
  Install Size: 125 MB
  Last Updated: 2024-01-15
```

### 7. Removal Confirmation

```
==============================================================================
  Removal Confirmation
==============================================================================

You have selected 3 packages for removal:

┌──────────────────────────────────────────────────────────────────────────┐
│ SAFE TO REMOVE (3 packages):                                             │
│ • com.facebook.katana                     (Safety: 95/100)               │
│ • com.instagram.android                   (Safety: 90/100)               │
│ • com.netflix.mediaclient                 (Safety: 85/100)               │
└──────────────────────────────────────────────────────────────────────────┘

[WARNING] This action cannot be undone easily!
[INFO] You can disable packages first to test for issues.
[INFO] Packages will be removed in the order listed above.

Proceed with removal? (yes/no):
```

### 8. Search Interface

```
Search Packages
Enter search query (or 'q' to cancel): google

Found 5 matching packages:
  [1] com.google.android.gms
  [2] com.google.android.apps.maps
  [3] com.google.android.apps.messaging
  [4] com.google.android.apps.docs
  [5] com.google.android.apps.photos

Select package number (or 0 to go back): 2
```

## Implementation Details

### Screen Layout

```
┌────────────────────────────────────────────┐
│      Android DeBloat Tool v1.0             │
│      Windows C++ Console Application       │
└────────────────────────────────────────────┘

Device: Google Pixel 6
Total Packages: 327 (15 DO NOT TOUCH, 25 Bloatware, 287 Optional)
Storage: 256 GB | Memory: 8 GB | Android: 13

Main Menu:
[1] Show All Packages
[2] Show Critical Packages
[3] Show Safe to Remove (25)
[4] Show Optional Packages (287)
[5] Search Packages
[6] View Package Details
[7] Select Packages to Remove
[8] Confirm & Remove
[9] Exit

Select option (1-9):
```

## Safety Features

### 1. DO NOT TOUCH Protection
- Never selectable in removal interface
- Clear warning on display
- 0 safety score always

### 2. Multi-step Confirmation
- First select packages
- Review before confirmation
- Display safety scores
- Final yes/no prompt

### 3. Category Filtering
- Show only safe packages in removal mode
- Critical packages hidden from selection
- Warnings for risky packages

### 4. Search Safe Mode
- Display package details before selection
- Show classification & safety info
- Confirm before proceeding

## Build & Test

### Compilation:
```bash
g++ -std=c++17 -Wall -O2 src/main.cpp src/CommandExecutor.cpp \
    src/AdbManager.cpp src/PackageManager.cpp \
    src/PackageClassifier.cpp src/UserInterface.cpp \
    -Iinclude -o build/DeBloat.exe
```

### Phase 5 Test Flow:
1. Display main menu
2. Test package listing
3. Test search functionality
4. Test multi-select (without actual removal)
5. Test confirmation dialog
6. Test detail view
7. Test device info display

## Platform Support

### Windows:
- Uses `system("cls")` for screen clear
- Supports Unicode display
- CMD and PowerShell compatible

### Linux/Mac:
- Uses `system("clear")` for screen clear
- Terminal compatibility tested
- Cross-platform builds with `#ifdef _WIN32`

## Next Phase: Phase 6

Phase 6 will implement the actual package removal engine:
- `RemovePackage()` - Execute adb shell pm uninstall
- `DisablePackage()` - Non-destructive disable
- `RemovalQueue` - Queue multiple removals
- `RollbackSystem` - Backup and restore functionality

## Statistics

- **Lines of Code:** ~500 (Phase 5)
- **Functions:** 15+
- **Menu Options:** 9
- **Supported Categories:** 4
- **Cross-platform:** Yes (Windows/Linux/Mac)

## Compilation Status

✓ Phase 1: CommandExecutor - Complete
✓ Phase 2: ADB Integration - Complete  
✓ Phase 3: Package Management - Complete
✓ Phase 4: Classification System - Complete
→ Phase 5: User Interface - In Progress
◻ Phase 6: Removal System - Not Started
◻ Phase 7: Integration - Not Started
