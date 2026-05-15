#include "PackageClassifier.h"
#include <algorithm>
#include <set>
#include <unordered_set>
#include <iostream>
#include <iomanip>
#include <cctype>
#include <regex>

PackageClassifier::PackageClassifier(const std::string& deviceModel, OemType oemType)
    : deviceModel(deviceModel), currentOemType(oemType)
{
    BuildDatabase();
}

/**
 * Convert string to lowercase for case-insensitive matching
 */
static std::string ToLower(const std::string& str)
{
    std::string result = str;

    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c)
                   {
                       return std::tolower(c);
                   });

    return result;
}

/**
 * Check if package matches any pattern
 */
static bool MatchesAnyPattern(const std::string& packageName,
                              const std::vector<std::string>& patterns)
{
    for (const auto& pattern : patterns)
    {
        if (packageName.find(pattern) != std::string::npos)
        {
            return true;
        }
    }

    return false;
}

/**
 * Helper for critical classifications
 */
static void SetCritical(PackageClassification& result,
                        const std::string& reason,
                        const std::string& description)
{
    result.category = PackageCategory::DO_NOT_TOUCH;
    result.reason = reason;
    result.description = description;
    result.safetyScore = 0;
    result.canBeDisabled = false;
}

/**
 * Helper for optional classifications
 */
static void SetOptional(PackageClassification& result,
                        const std::string& reason,
                        const std::string& description,
                        int score = 75)
{
    result.category = PackageCategory::OPTIONAL;
    result.reason = reason;
    result.description = description;
    result.safetyScore = score;
    result.canBeDisabled = true;
}

/**
 * Helper for safe-to-remove classifications
 */
static void SetSafe(PackageClassification& result,
                    const std::string& reason,
                    const std::string& description,
                    int score = 90)
{
    result.category = PackageCategory::SAFE_TO_REMOVE;
    result.reason = reason;
    result.description = description;
    result.safetyScore = score;
    result.canBeDisabled = true;
}

static void SetAnalytics(PackageClassification& result,
                         const std::string& reason,
                         const std::string& description,
                         int score = 90)
{
    result.category = PackageCategory::ANALYTICS;
    result.reason = reason;
    result.description = description;
    result.safetyScore = score;
    result.canBeDisabled = true;
}

void PackageClassifier::BuildDatabase()
{
    database.clear();

    auto addCritical =
        [this](const std::string& pkg,
               const std::string& reason,
               const std::string& description)
    {
        PackageClassification item(pkg);

        item.category = PackageCategory::DO_NOT_TOUCH;
        item.reason = reason;
        item.description = description;
        item.safetyScore = 0;
        item.canBeDisabled = false;

        database[pkg] = item;
    };

    // Core Android
    addCritical(
        "android",
        "Core Android framework",
        "Required for Android to function properly");

    addCritical(
        "com.android.systemui",
        "System interface package",
        "Controls notifications, status bar, and navigation");

    addCritical(
        "com.android.settings",
        "Android Settings app",
        "Required for changing device settings");

    addCritical(
        "com.android.phone",
        "Phone and calling service",
        "Required for calls, SMS, and mobile network features");

    addCritical(
        "com.google.android.gms",
        "Google Play Services",
        "Required by many Android apps and Google services");

    // Xiaomi / MIUI critical packages
    addCritical(
        "com.miui.securitycenter",
        "MIUI security service",
        "Controls important security and battery functions");

    addCritical(
        "com.miui.securitycore",
        "MIUI core security service",
        "Required for Xiaomi security features");

    addCritical(
        "com.miui.powerkeeper",
        "Battery management service",
        "Controls battery optimization and app management");

    addCritical(
        "com.miui.core",
        "MIUI core framework",
        "Required for Xiaomi system functionality");

    addCritical(
        "com.miui.system",
        "MIUI system package",
        "Important Xiaomi system component");

    addCritical(
        "com.xiaomi.account",
        "Xiaomi account service",
        "Handles Xiaomi account login and synchronization");

    addCritical(
        "com.xiaomi.xmsf",
        "Xiaomi messaging framework",
        "Supports Xiaomi push notifications and services");

    addCritical(
        "com.xiaomi.xmsfkeeper",
        "Xiaomi service keeper",
        "Keeps Xiaomi background services functioning");

    // Advanced instead of removable
    PackageClassification findDevice("com.xiaomi.finddevice");
    findDevice.category = PackageCategory::OPTIONAL;
    findDevice.reason = "Xiaomi Find Device feature";
    findDevice.description =
        "Removing this disables Xiaomi device tracking and remote location features";
    findDevice.safetyScore = 55;
    findDevice.canBeDisabled = true;

    database["com.xiaomi.finddevice"] = findDevice;
}

PackageClassification PackageClassifier::Classify(const Package& package)
{
    PackageClassification result(package.packageName);

    const std::string pkgLower = ToLower(package.packageName);

    // ===== CRITICAL SYSTEM CHECK =====

    if (package.IsCriticalSystem())
    {
        SetCritical(
            result,
            "Critical Android system package",
            "Required for Android system stability");

        return result;
    }

    // ===== EXACT DATABASE MATCH =====

    const auto* dbEntry = GetDatabaseEntry(package.packageName);

    if (dbEntry)
    {
        return *dbEntry;
    }

    // ===== OVERLAY / RRO PROTECTION =====

    if (MatchesAnyPattern(pkgLower,
    {
        ".overlay",
        ".rro"
    }))
    {
        SetCritical(
            result,
            "System theme or resource overlay",
            "Removing this may break the device interface or themes");

        return result;
    }

    // ===== ANDROID FRAMEWORK / PROVIDER PROTECTION =====

    if (pkgLower.find("com.android.providers.") == 0 ||
        pkgLower == "com.android.permissioncontroller" ||
        pkgLower == "com.android.networkstack" ||
        pkgLower == "com.android.bluetooth" ||
        pkgLower == "com.android.externalstorage")
    {
        SetCritical(
            result,
            "Core Android service",
            "Required for important Android system functionality");

        return result;
    }

    // ===== QUALCOMM / TELEPHONY / IMS PROTECTION =====

    if (MatchesAnyPattern(pkgLower,
    {
        "qti",
        "qualcomm",
        "qcril",
        "ims",
        "iwlan",
        "uim",
        "volte",
        "telephony",
        "carrier"
    }))
    {
        SetCritical(
            result,
            "Network and telephony component",
            "Required for calls, mobile data, SIM, or carrier services");

        return result;
    }

    // ===== MIUI / XIAOMI LOGIC =====

    // Protected Xiaomi packages
    if (MatchesAnyPattern(pkgLower,
    {
        "securitycenter",
        "securitycore",
        "powerkeeper",
        "xmsf",
        "miui.core",
        "miui.system"
    }))
    {
        SetCritical(
            result,
            "Important Xiaomi system component",
            "Required for Xiaomi system stability and security");

        return result;
    }

    // Xiaomi cloud/account integrations -> OPTIONAL
    if (MatchesAnyPattern(pkgLower,
    {
        "finddevice",
        "cloudservice",
        "micloud",
        "backup"
    }))
    {
        SetOptional(
            result,
            "Xiaomi cloud feature",
            "Removing this disables Xiaomi cloud or device tracking features",
            60);

        return result;
    }

    // Xiaomi removable bloatware
    if (MatchesAnyPattern(pkgLower,
    {
        "msa",
        "mipicks",
        "analytics",
        "hybrid",
        "browser",
        "videoplayer",
        "music",
        "joyose",
        "weather",
        "scanner",
        "compass"
    }))
    {
        SetSafe(
            result,
            "Preinstalled Xiaomi app",
            "Optional Xiaomi app that can usually be removed safely",
            88);

        return result;
    }

    // ===== ANALYTICS / ADS =====

    if (MatchesAnyPattern(pkgLower,
    {
        "analytics",
        "telemetry",
        "tracker",
        "ads",
        "admob",
        "advert"
    }))
    {
        if (package.type != PackageType::USER)
        {
            SetAnalytics(
                result,
                "Analytics or tracking package",
                "Collects usage data, telemetry, or advertising information",
                92);

            return result;
        }
    }

    // ===== USER INSTALLED =====

    if (package.type == PackageType::USER)
    {
        result.category = PackageCategory::USER_APP;
        result.reason = "User-installed application";
        result.description = "Installed by the user and safe to remove";
        result.safetyScore = 90;
        result.canBeDisabled = true;

        return result;
    }

    // ===== POPULAR USER APPS =====

    if (MatchesAnyPattern(pkgLower,
    {
        "facebook",
        "instagram",
        "spotify",
        "netflix",
        "whatsapp",
        "telegram",
        "snapchat",
        "twitter",
        "discord",
        "reddit",
        "youtube"
    }))
    {
        SetOptional(
            result,
            "Popular app",
            "You can remove this app if you do not use it",
            95);

        return result;
    }

    // ===== OPTIONAL UTILITIES =====

    if (MatchesAnyPattern(pkgLower,
    {
        "calculator",
        "recorder",
        "notes",
        "gallery",
        "compass",
        "scanner"
    }))
    {
        SetOptional(
            result,
            "Optional utility app",
            "Useful app but not required for Android to function",
            80);

        return result;
    }

    // ===== SYSTEM DEFAULT =====

    if (package.type == PackageType::SYSTEM ||
        package.type == PackageType::SYSTEM_UPDATE)
    {
        SetCritical(
            result,
            "Preinstalled system package",
            "Part of the Android system image");

        return result;
    }

    // ===== FALLBACK =====

    result.category = PackageCategory::UNCATEGORIZED;
    result.reason = "Unfamiliar package";
    result.description =
        "Package could not be confidently identified. Review carefully before removing.";
    result.safetyScore = 40;
    result.canBeDisabled = true;

    return result;
}

const PackageClassification*
PackageClassifier::GetDatabaseEntry(const std::string& packageName) const
{
    auto it = database.find(packageName);

    if (it != database.end())
    {
        return &it->second;
    }

    return nullptr;
}

bool PackageClassifier::MatchesPattern(
    const std::string& packageName,
    const std::string& pattern)
{
    std::string pat = pattern;

    std::replace(pat.begin(), pat.end(), ' ', '*');

    if (pat.find('*') == std::string::npos)
    {
        return packageName == pat;
    }

    size_t p = 0;
    size_t n = 0;

    while (p < pat.size() && n < packageName.size())
    {
        if (pat[p] == '*')
        {
            while (p < pat.size() && pat[p] == '*')
            {
                ++p;
            }

            if (p == pat.size())
            {
                return true;
            }

            while (n < packageName.size() &&
                   packageName[n] != pat[p])
            {
                ++n;
            }

            if (n == packageName.size())
            {
                return false;
            }
        }
        else
        {
            if (pat[p] != packageName[n])
            {
                return false;
            }

            ++p;
            ++n;
        }
    }

    return p == pat.size() && n == packageName.size();
}

OemType PackageClassifier::DetectOemType(const std::string& manufacturer,
                                         const std::string& product,
                                         const std::string& buildBrand)
{
    std::string mfg = ToLower(manufacturer);
    std::string prod = ToLower(product);
    std::string brand = ToLower(buildBrand);

    if (mfg.find("samsung") != std::string::npos)
        return OemType::SAMSUNG;
    if (mfg.find("xiaomi") != std::string::npos)
        return OemType::XIAOMI;
    if (mfg.find("oneplus") != std::string::npos)
        return OemType::ONEPLUS;
    if (mfg.find("oppo") != std::string::npos)
        return OemType::OPPO;
    if (mfg.find("vivo") != std::string::npos)
        return OemType::VIVO;
    if (mfg.find("realme") != std::string::npos)
        return OemType::REALME;
    if (mfg.find("motorola") != std::string::npos || mfg.find("moto") != std::string::npos)
        return OemType::MOTOROLA;
    if (mfg.find("huawei") != std::string::npos)
        return OemType::HUAWEI;
    if (mfg.find("nothing") != std::string::npos)
        return OemType::NOTHING;
    if (mfg.find("google") != std::string::npos)
        return OemType::STOCK_ANDROID;

    if (prod.find("pixel") != std::string::npos || brand.find("pixel") != std::string::npos)
        return OemType::STOCK_ANDROID;
    if (prod.find("oneplus") != std::string::npos)
        return OemType::ONEPLUS;

    return OemType::UNKNOWN;
}

std::string PackageClassifier::GetOemTypeName(OemType oemType)
{
    switch (oemType)
    {
    case OemType::STOCK_ANDROID:
        return "Stock Android (Pixel)";
    case OemType::SAMSUNG:
        return "Samsung OneUI";
    case OemType::XIAOMI:
        return "Xiaomi MIUI";
    case OemType::ONEPLUS:
        return "OnePlus OxygenOS";
    case OemType::OPPO:
        return "OPPO ColorOS";
    case OemType::VIVO:
        return "Vivo FunTouchOS";
    case OemType::REALME:
        return "Realme Realme UI";
    case OemType::MOTOROLA:
        return "Motorola (Stock-based)";
    case OemType::HUAWEI:
        return "Huawei HarmonyOS/EMUI";
    case OemType::NOTHING:
        return "Nothing OS";
    case OemType::CUSTOM_ROM:
        return "Custom ROM";
    case OemType::UNKNOWN:
    default:
        return "Unknown OEM";
    }
}

std::string PackageClassifier::GetCategoryName(PackageCategory category)
{
    switch (category)
    {
    case PackageCategory::DO_NOT_TOUCH:
        return "Critical";
    case PackageCategory::SAFE_TO_REMOVE:
        return "Bloat";
    case PackageCategory::ANALYTICS:
        return "Analytics";
    case PackageCategory::OPTIONAL:
        return "Optional";
    case PackageCategory::USER_APP:
        return "User";
    case PackageCategory::UNCATEGORIZED:
    default:
        return "Other";
    }
}

std::vector<PackageClassification>
PackageClassifier::ClassifyMultiple(
    const std::vector<Package>& packages,
    PackageCategory category)
{
    std::vector<PackageClassification> result;

    // Prevent duplicate package entries
    std::unordered_set<std::string> seen;

    for (const auto& pkg : packages)
    {
        if (seen.count(pkg.packageName))
        {
            continue;
        }

        seen.insert(pkg.packageName);

        auto classified = Classify(pkg);

        if (category == PackageCategory::UNCATEGORIZED ||
            classified.category == category)
        {
            result.push_back(classified);
        }
    }

    return result;
}