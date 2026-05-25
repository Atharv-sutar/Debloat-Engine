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
                // Core Android
                "android",
                "com.android.systemui",
                "com.android.settings",
                "com.android.shell",
                "com.android.phone",
                "com.android.providers",
                "com.android.permissioncontroller",
                "com.android.networkstack",
                "com.android.bluetooth",
                "com.android.externalstorage",
                "com.android.launcher",
                "com.android.packageinstaller",
                "com.android.vending",

                // Google Core Services
                "com.google.android.gms",
                "com.google.android.gsf",
                "com.google.android.ext.services",

                // Qualcomm / IMS / Telephony
                "com.qualcomm.qti",
                "com.qti",
                "com.qcril",
                "com.qualcomm.qcrilmsgtunnel",
                "com.android.ims",
                "org.codeaurora.ims",

                // Samsung critical
                "com.samsung.android.providers",
                "com.samsung.android.dialer",
                "com.samsung.android.incallui",

                // Xiaomi critical
                "com.miui.securitycenter",
                "com.miui.securitycore",
                "com.miui.powerkeeper",
                "com.xiaomi.xmsf",

                // OnePlus / Oppo / Realme critical
                "com.oplus.securitypermission",
                "com.coloros.securitypermission",

                // Vivo critical
                "com.vivo.pem"};

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
            // Facebook Meta Services
            "com.facebook.services",
            "com.facebook.appmanager",
            "com.facebook.system",

            // Amazon / Netflix partner preload
            "com.amazon.appmanager",
            "com.amazon.avod.thirdpartyclient",
            "com.netflix.partner.activation",

            // Booking / Opera preload
            "com.booking",
            "com.booking.us",
            "com.opera.browser",
            "com.opera.preinstall",

            // Xiaomi safe removals
            "com.miui.analytics",
            "com.miui.msa.global",
            "com.xiaomi.mipicks",
            "com.xiaomi.discover",
            "com.miui.weather2",
            "com.miui.hybrid",
            "com.miui.hybrid.accessory",
            "com.miui.player",
            "com.miui.videoplayer",
            "com.miui.yellowpage",
            "com.miui.bugreport",
            "com.miui.android.fashiongallery",
            "com.xiaomi.gamecenter",

            // Samsung safe removals
            "com.samsung.android.game.gamehome",
            "com.samsung.android.game.gametools",
            "com.samsung.android.game.gos",
            "com.samsung.android.themestore",
            "com.samsung.android.themecenter",
            "com.samsung.android.aremoji",
            "com.samsung.android.arzone",
            "com.samsung.android.app.spage",
            "com.samsung.android.tvplus",
            "com.samsung.android.stickercenter",
            "com.samsung.android.app.tips",

            // OnePlus / Oppo / Realme
            "com.oneplus.store",
            "com.oneplus.membership",
            "com.heytap.browser",
            "com.heytap.market",
            "com.heytap.themestore",
            "com.heytap.community",
            "com.coloros.gamespace",
            "com.realmestore.app",

            // Vivo
            "com.vivo.browser",
            "com.vivo.appstore",
            "com.vivo.easyshare",
            "com.bbk.appstore",
            "com.bbk.theme"};

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
            // Firebase / Google analytics
            "com.google.firebase",
            "com.google.android.feedback",

            // Common analytics SDKs
            "com.adjust",
            "com.amplitude",
            "com.mixpanel",
            "com.flurry",
            "com.segment",
            "com.appsflyer",
            "com.crashlytics",

            // Ads / tracking SDKs
            "com.ironsource",
            "com.inmobi",
            "com.applovin",
            "com.chartboost",
            "com.tapjoy",
            "com.vungle",

            // Xiaomi analytics
            "com.miui.analytics",
            "com.miui.msa.global",

            // Oppo/Realme telemetry
            "com.oplus.statistics.rom",
            "com.oplus.onetrace",

            // Vivo telemetry
            "com.vivo.analytics"};

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
