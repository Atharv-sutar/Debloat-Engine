# CLASSIFICATION CONTROL TRANSFER ARCHITECTURE

## CLASSIFICATION SYSTEM OVERVIEW
Classification pipeline: Package → PackageManager → PackageClassifier → PackageClassification → main_production

---

## DATA STRUCTURES

### Package (include/Package.h)
```
Input Data:
  - packageName: std::string
  - type: PackageType { SYSTEM, SYSTEM_UPDATE, USER, UNKNOWN }
  - displayName: std::string
  - versionCode: std::string
  - versionName: std::string
  - installSize: long long
  - isEnabled: bool
  - lastUpdateTime: std::string

Critical Method:
  - IsCriticalSystem() -> bool
    Returns TRUE if packageName matches critical Android framework patterns
```

### PackageClassification (include/PackageClassifier.h)
```
Output Data:
  - packageName: std::string
  - category: PackageCategory { DO_NOT_TOUCH, SAFE_TO_REMOVE, OPTIONAL, USER_APP, UNCATEGORIZED }
  - reason: std::string (classification rationale)
  - description: std::string (package purpose)
  - isOemSpecific: bool
  - oemApplicability: OemType
  - canBeDisabled: bool
  - safetyScore: int (0-100, higher = safer to remove)
```

---

## FILE DEPENDENCIES & CONTROL FLOW

### 1. AdbManager → PackageManager
**File**: src/main_production.cpp:78-92
```cpp
// AdbManager provides device list and properties
auto devices = adbMgr.GetConnectedDevices();
std::string mfg = adbMgr.GetDeviceProperty(targetDevice, "ro.product.manufacturer");

// Passes to PackageManager constructor
PackageManager pkgMgr(targetDevice);
```
**Data Transfer**:
  - FROM: Device serial number, manufacturer property
  - TO: PackageManager

### 2. PackageManager → PackageClassifier
**File**: src/main_production.cpp:107-112
```cpp
std::vector<Package> allPkgs = pkgMgr.GetAllPackages();
OemType detectedOem = PackageClassifier::DetectOemType(mfg, "", "");
PackageClassifier classifier(deviceModel, detectedOem);
```
**Data Transfer**:
  - FROM: Vector<Package>, manufacturer string
  - TRANSFORM: OemType detection (static method)
  - TO: PackageClassifier constructor receives (deviceModel, OemType)

### 3. PackageClassifier Initialization
**File**: src/PackageClassifier.cpp:1-11
```cpp
PackageClassifier::PackageClassifier(const std::string& deviceModel, OemType oemType)
    : deviceModel(deviceModel), currentOemType(oemType)
{
    BuildDatabase();  // Populates static database map
}
```
**State Setup**:
  - deviceModel: stored for reference
  - currentOemType: OemType enum for OEM-specific matching
  - database: std::map<std::string, PackageClassification> loaded with critical packages

### 4. Classification Execution (Core Logic)
**File**: src/main_production.cpp:113-115
```cpp
auto safeToRemove = classifier.ClassifyMultiple(allPkgs, PackageCategory::SAFE_TO_REMOVE);
auto optionalList = classifier.ClassifyMultiple(allPkgs, PackageCategory::OPTIONAL);
```

**Method Chain**:
```
ClassifyMultiple(vector<Package>, PackageCategory filter)
  └─> iterates each Package
      └─> calls Classify(Package) for each
          └─> returns PackageClassification
      └─> filters by category if specified
  └─> returns vector<PackageClassification>
```

### 5. Classify() Execution Flow
**File**: src/PackageClassifier.cpp:111-300
**Method Signature**:
```cpp
PackageClassification PackageClassifier::Classify(const Package& package)
```

**TIER PRECEDENCE ORDER** (returns immediately on match):

#### TIER 1: Critical System Check (HIGHEST PRIORITY)
```
Package.IsCriticalSystem() == true
├─ CHECK: Critical package list in Package.h
│   Android core: android, com.android.internal, com.android.providers.settings, etc.
└─ RETURNS: PackageClassification
   ├─ category: DO_NOT_TOUCH
   ├─ safetyScore: 0
   ├─ canBeDisabled: false
```

#### TIER 2: Database Exact Match
```
GetDatabaseEntry(packageName) != nullptr
├─ CHECK: database map built by BuildDatabase()
├─ DATABASE ENTRIES (include/PackageClassifier.h:43-60):
│  - android → DO_NOT_TOUCH (safetyScore: 0)
│  - com.android.systemui → DO_NOT_TOUCH (safetyScore: 0)
│  - com.android.settings → DO_NOT_TOUCH (safetyScore: 0)
│  - com.android.phone → DO_NOT_TOUCH (safetyScore: 0)
│  - com.google.android.gms → DO_NOT_TOUCH (safetyScore: 0)
└─ RETURNS: Stored PackageClassification
```

#### TIER 3: Dynamic Pattern Matching

**3A. Core Android Framework**
```
Pattern: pkgLower.find("com.android.") == 0 OR pkgLower == "android"
└─ RETURNS: DO_NOT_TOUCH (safetyScore: 0, canBeDisabled: false)
```

**3B. OEM Bloatware Patterns** (MatchesAnyPattern)
```
XIAOMI:
  Patterns: miui, xiaomi, msa, mipicks, joyose, finddevice, securitycenter, mi_*
  Special Protection: com.miui.daemon, com.miui.powerkeeper → DO_NOT_TOUCH
  Others → SAFE_TO_REMOVE (safetyScore: 88)

SAMSUNG:
  Patterns: samsung.android, sec., knox, flipboard, bixby, spage, aqua
  All → SAFE_TO_REMOVE (safetyScore: 85)

OPPO:
  Patterns: oppo, coloros, com.oppo
  All → SAFE_TO_REMOVE (safetyScore: 85)

VIVO:
  Patterns: vivo, funtouch, com.vivo
  All → SAFE_TO_REMOVE (safetyScore: 85)

REALME:
  Patterns: realme, com.realme
  All → SAFE_TO_REMOVE (safetyScore: 85)

ONEPLUS:
  Patterns: oneplus, oxygen, com.oneplus
  All → SAFE_TO_REMOVE (safetyScore: 85)

HUAWEI:
  Patterns: huawei, honor, hicare, com.huawei
  All → SAFE_TO_REMOVE (safetyScore: 85)
```

**3C. Analytics & Telemetry**
```
Patterns: analytics, telemetry, metrics, tracker, firebase, flurry, amplitude, mixpanel
└─ RETURNS: SAFE_TO_REMOVE (safetyScore: 92, canBeDisabled: true)
```

**3D. Advertising SDKs**
```
Patterns: ads, advert, admob, adcolony, googleads, mopub, appodeal, ironsource
└─ RETURNS: SAFE_TO_REMOVE (safetyScore: 90, canBeDisabled: true)
```

**3E. Popular Apps**
```
YouTube Music: com.google.android.apps.youtube.music
└─ RETURNS: SAFE_TO_REMOVE (safetyScore: 90)

Social/Entertainment: whatsapp, instagram, facebook, spotify, youtube, netflix, telegram, discord, etc.
└─ RETURNS: OPTIONAL (safetyScore: 95, canBeDisabled: true)

Productivity: gmail, chrome, maps, drive, linkedin, etc.
└─ RETURNS: OPTIONAL (safetyScore: 95, canBeDisabled: true)

Payment: googleplay, phonepe, googlepay, applepay, paytm, etc.
└─ RETURNS: OPTIONAL (safetyScore: 95, canBeDisabled: true)
```

#### TIER 4: System vs User Package Type
```
IF package.type == PackageType::SYSTEM (unmatched by Tiers 1-3)
└─ RETURNS: DO_NOT_TOUCH (safetyScore: 15, canBeDisabled: false)
   (System packages default to protected unless explicitly matched as bloatware)
```

#### TIER 5: User-Installed Default
```
IF package.type == PackageType::USER (no prior match)
└─ RETURNS: USER_APP (safetyScore: 85, canBeDisabled: true)
```

---

## HELPER METHODS

### DetectOemType (static) - OEM Identification
**File**: src/PackageClassifier.cpp:318-380
```cpp
static OemType DetectOemType(manufacturer, product, buildBrand) -> OemType

Priority Order (case-insensitive substring match):
  1. manufacturer.find("samsung") → OemType::SAMSUNG
  2. manufacturer.find("xiaomi") → OemType::XIAOMI
  3. manufacturer.find("oneplus") → OemType::ONEPLUS
  4. manufacturer.find("oppo") → OemType::OPPO
  5. manufacturer.find("vivo") → OemType::VIVO
  6. manufacturer.find("realme") → OemType::REALME
  7. manufacturer.find("motorola") → OemType::MOTOROLA
  8. manufacturer.find("huawei") → OemType::HUAWEI
  9. manufacturer.find("nothing") → OemType::NOTHING
  10. manufacturer.find("google") → OemType::STOCK_ANDROID
  11. product/brand.find("pixel") → OemType::STOCK_ANDROID
  12. else → OemType::UNKNOWN
```

### MatchesAnyPattern (static)
**File**: src/PackageClassifier.cpp:26-33
```cpp
static bool MatchesAnyPattern(packageName: string, patterns: vector<string>) -> bool

Execution:
  1. Convert packageName to lowercase
  2. For each pattern in patterns:
     IF packageName.find(pattern) != npos → return TRUE
  3. return FALSE (no match)
```

### ToLower (static)
**File**: src/PackageClassifier.cpp:14-21
```cpp
static std::string ToLower(str: string) -> string
  Transform all chars to lowercase using std::tolower
```

### GetDatabaseEntry (private)
**File**: src/PackageClassifier.cpp:293-308
```cpp
private const PackageClassification* GetDatabaseEntry(packageName: string) -> ptr

Execution:
  1. Exact map lookup: database.find(packageName)
     IF found → return pointer to value
  2. Pattern matching fallback: iterate database keys
     IF MatchesPattern(packageName, key) → return pointer to value
  3. return nullptr (not found)
```

---

## COMPLETE DATA FLOW SEQUENCE

```
main_production.cpp:
  ├─ STEP 1: AdbManager::GetConnectedDevices() → vector<Device>
  │            └─ Extract: serialNumber, model, authStatus
  │
  ├─ STEP 2: AdbManager::GetDeviceProperty("ro.product.manufacturer") → string
  │
  ├─ STEP 3: PackageManager(serialNumber)
  │            └─ Stores: targetSerial = serialNumber
  │
  ├─ STEP 4: PackageManager::FetchAllPackages() → bool
  │            └─ Executes: adb shell pm list packages (system + user)
  │            └─ Parses: packageName → Package objects
  │            └─ Stores: allPackages = vector<Package>
  │
  ├─ STEP 5: PackageManager::GetAllPackages() → vector<Package>
  │            └─ Returns: vector of Package structs with type/name info
  │
  ├─ STEP 6: PackageClassifier::DetectOemType(manufacturer, "", "") → OemType
  │            └─ Parses: manufacturer string
  │            └─ Returns: OemType enum value
  │
  ├─ STEP 7: PackageClassifier(deviceModel, detectedOemType)
  │            └─ Stores: currentOemType, deviceModel
  │            └─ Calls: BuildDatabase() → populates internal database map
  │
  ├─ STEP 8: PackageClassifier::ClassifyMultiple(allPackages, SAFE_TO_REMOVE) → vector<PackageClassification>
  │            ├─ For each Package in allPackages:
  │            │    └─ Classify(package) → PackageClassification
  │            │       (Executes TIER 1-5 logic above)
  │            └─ Filter: keep only results where category == SAFE_TO_REMOVE
  │            └─ Returns: vector<PackageClassification> for SAFE_TO_REMOVE category
  │
  └─ STEP 9: Display results in main_production.cpp menu system
              (safeToRemoveList shown with safetyScore, reason, description)
```

---

## CRITICAL INVARIANTS

1. **Tier Precedence**: Tiers 1-5 are evaluated in strict order. First match wins, no further evaluation.
2. **Case Insensitivity**: All pattern matching is case-insensitive (converted to lowercase).
3. **Pattern Substring Matching**: `pattern.find(substring) != npos` - patterns match ANY occurrence in packageName.
4. **Static Methods**: DetectOemType, MatchesAnyPattern, ToLower are static utilities - NO instance state required.
5. **Database Immutability**: BuildDatabase() populates database once at construction. Not modified during classification.
6. **Default Safety**: Unknown/unmatched packages default to DO_NOT_TOUCH (system) or USER_APP (user-installed).

---

## CALLING SEQUENCE FOR AI INTEGRATION

```cpp
// Initialize
PackageClassifier classifier(deviceModel, oemType);

// Classify single package
PackageClassification result = classifier.Classify(singlePackage);

// Classify batch and filter
vector<PackageClassification> filtered = 
    classifier.ClassifyMultiple(allPackages, targetCategory);

// Access results
string category = PackageClassifier::GetCategoryName(result.category);
string oem = PackageClassifier::GetOemTypeName(result.oemApplicability);
```

---

## FILE RESPONSIBILITY MATRIX

| File | Responsibility |
|------|-----------------|
| Package.h | Data: package name, type (SYSTEM/USER), properties; Method: IsCriticalSystem() |
| PackageClassifier.h | Interface: Classify(), ClassifyMultiple(); Enums: PackageCategory, OemType |
| PackageClassifier.cpp | Implementation: 5-tier classification logic, pattern matching, database, OEM detection |
| PackageManager.h | Data retrieval: FetchAllPackages(), GetAllPackages() |
| main_production.cpp | Orchestration: flow control, result display, user interaction |

