#pragma once

#include <string>
#include <vector>

/**
 * Enum for package type classification
 */
enum class PackageType
{
    SYSTEM,        // System package (cannot be uninstalled)
    SYSTEM_UPDATE, // System update package
    USER,          // User-installed package
    UNKNOWN        // Type unknown
};

/**
 * Struct representing an installed Android package
 */
struct Package
{
    std::string packageName;    // Full package name (e.g., "com.android.settings")
    std::string displayName;    // Human-readable name (may be empty initially)
    PackageType type;           // System or User package
    std::string versionCode;    // Version code (may be empty)
    std::string versionName;    // Version name (may be empty)
    long long installSize;      // Installed size in bytes
    bool isEnabled;             // Whether package is enabled/disabled
    std::string lastUpdateTime; // Last update timestamp

    // Constructor
    Package() : packageName(""), displayName(""), type(PackageType::UNKNOWN),
                versionCode(""), versionName(""), installSize(0),
                isEnabled(true), lastUpdateTime("") {}

    // Constructor with package name
    explicit Package(const std::string &name)
        : packageName(name), displayName(name), type(PackageType::UNKNOWN),
          versionCode(""), versionName(""), installSize(0),
          isEnabled(true), lastUpdateTime("") {}

    // Constructor with name and type
    Package(const std::string &name, PackageType t)
        : packageName(name), displayName(name), type(t),
          versionCode(""), versionName(""), installSize(0),
          isEnabled(true), lastUpdateTime("") {}

    // Get short name from package name (last component)
    std::string GetShortName() const
    {
        size_t lastDot = packageName.find_last_of('.');
        if (lastDot != std::string::npos && lastDot + 1 < packageName.length())
        {
            return packageName.substr(lastDot + 1);
        }
        return packageName;
    }

    // Check if package is critical system package
    bool IsCriticalSystem() const
    {
        // List of critical Android system packages that should never be touched
        static const std::vector<std::string> criticalPackages =
            {
                "android",
                "com.android.internal",
                "com.android.providers",
                "com.android.systemui",
                "com.android.shell",
                "com.android.phone",
                "com.android.server",
                "com.android.permissioncontroller",
                "com.android.networkstack",
                "com.android.bluetooth",
                "com.android.externalstorage",

                // Qualcomm / IMS
                "com.qualcomm",
                "com.qti",
                "com.qcril",
                "com.ims",

                // Xiaomi critical
                "com.miui.securitycenter",
                "com.miui.securitycore",
                "com.miui.powerkeeper",
                "com.miui.core",
                "com.miui.system",
                "com.xiaomi.xmsf",
                "com.xiaomi.xmsfkeeper"};

        for (const auto &critical : criticalPackages)
        {
            if (packageName == critical ||
                packageName.find(critical + ".") == 0)
            {
                return true;
            }
        }
        return false;
    }

    // Check if package is known bloatware
    bool IsKnownBloatware() const
    {
        // Common bloatware packages to identify
        static const std::vector<std::string> bloatwarePatterns = {
            "com.facebook.services",
            "com.facebook.appmanager",
            "com.netflix.partner.activation",
            "com.amazon.appmanager",
            "com.booking",
            "com.opera.browser",
            "com.miui.analytics",
            "com.xiaomi.health",
            "com.miui.weather2",
            "com.xiaomi.mipicks",
            "com.samsung.android.game.gamehome",
            "com.samsung.android.themestore"
        };

        for (const auto &pattern : bloatwarePatterns)
        {
            if (packageName == pattern ||
                packageName.find(pattern + ".") == 0)
            {
                return true;
            }
        }
        return false;
    }

    // Check if package is analytics/tracking
    bool IsAnalytics() const
    {
        static const std::vector<std::string> analyticsPatterns = {
            "com.google.firebase",    // Firebase Analytics
            "com.adjust",             // Adjust analytics
            "com.amplitude",          // Amplitude analytics
            "com.mixpanel",           // Mixpanel analytics
            "com.flurry",             // Flurry analytics
            "com.segment",            // Segment analytics
            "com.appsflyer",          // AppsFlyer analytics
            "com.crashlytics"         // Crashlytics analytics
        };

        for (const auto &pattern : analyticsPatterns)
        {
            if (packageName == pattern ||
                packageName.find(pattern + ".") == 0)
            {
                return true;
            }
        }
        return false;
    }
};
