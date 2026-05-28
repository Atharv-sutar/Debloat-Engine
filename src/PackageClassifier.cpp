#include "PackageClassifier.h"
#include <algorithm>
#include <set>
#include <unordered_set>
#include <iostream>
#include <iomanip>
#include <cctype>
#include <regex>

PackageClassifier::PackageClassifier(const std::string &deviceModel, OemType oemType)
    : deviceModel(deviceModel), currentOemType(oemType)
{
    BuildDatabase();
}

/**
 * Convert string to lowercase for case-insensitive matching
 */
static std::string ToLower(const std::string &str)
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
static bool MatchesAnyPattern(const std::string &packageName,
                              const std::vector<std::string> &patterns)
{
    for (const auto &pattern : patterns)
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
static void SetCritical(PackageClassification &result,
                        const std::string &reason,
                        const std::string &description)
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
static void SetOptional(PackageClassification &result,
                        const std::string &reason,
                        const std::string &description,
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
static void SetSafe(PackageClassification &result,
                    const std::string &reason,
                    const std::string &description,
                    int score = 90)
{
    result.category = PackageCategory::SAFE_TO_REMOVE;
    result.reason = reason;
    result.description = description;
    result.safetyScore = score;
    result.canBeDisabled = true;
}

static void SetAnalytics(PackageClassification &result,
                         const std::string &reason,
                         const std::string &description,
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
        [this](const std::string &pkg,
               const std::string &reason,
               const std::string &description)
    {
        PackageClassification item(pkg);

        item.category = PackageCategory::DO_NOT_TOUCH;
        item.reason = reason;
        item.description = description;
        item.safetyScore = 0;
        item.canBeDisabled = false;

        database[pkg] = item;
    };

    // Helper to add arbitrary database entries. Use this for bulk/pasted
    // package mappings (from packages.md or other lists). Paste your
    // `addDatabaseEntry("com.example.pkg", PackageCategory::SAFE_TO_REMOVE, "desc");`
    // lines between the START/END markers below.
    auto addDatabaseEntry = [this](const std::string &pkg,
                                   PackageCategory cat,
                                   const std::string &description)
    {
        PackageClassification item(pkg);
        item.category = cat;
        item.reason = "User-provided mapping";
        item.description = description;
        // Set reasonable defaults per category
        switch (cat)
        {
        case PackageCategory::DO_NOT_TOUCH:
            item.safetyScore = 0;
            item.canBeDisabled = false;
            break;
        case PackageCategory::ANALYTICS:
            item.safetyScore = 90;
            item.canBeDisabled = true;
            break;
        case PackageCategory::SAFE_TO_REMOVE:
            item.safetyScore = 85;
            item.canBeDisabled = true;
            break;
        case PackageCategory::OPTIONAL:
            item.safetyScore = 70;
            item.canBeDisabled = true;
            break;
        case PackageCategory::USER_APP:
            item.safetyScore = 95;
            item.canBeDisabled = true;
            break;
        default:
            item.safetyScore = 50;
            item.canBeDisabled = true;
            break;
        }

        database[pkg] = item;
    };

    // === START GENERATED DATABASE ENTRIES ===
    addDatabaseEntry("android", PackageCategory::DO_NOT_TOUCH, "Core Android");
    addDatabaseEntry("com.adobe.reader", PackageCategory::USER_APP, "Adobe Reader");
    addDatabaseEntry("com.airtel.mobile.android", PackageCategory::USER_APP, "Airtel app");
    addDatabaseEntry("com.amazon.appmanager", PackageCategory::SAFE_TO_REMOVE, "Amazon app manager");
    addDatabaseEntry("com.amazon.avod.thirdpartyclient", PackageCategory::SAFE_TO_REMOVE, "Amazon/AVOD third-party client");
    addDatabaseEntry("com.amazon.kindle", PackageCategory::SAFE_TO_REMOVE, "Amazon Kindle");
    addDatabaseEntry("com.amazon.mShop.android.shopping", PackageCategory::SAFE_TO_REMOVE, "Amazon Shopping");
    addDatabaseEntry("com.android.bluetooth", PackageCategory::DO_NOT_TOUCH, "Android Bluetooth");
    addDatabaseEntry("com.android.browser", PackageCategory::OPTIONAL, "Android Browser");
    addDatabaseEntry("com.android.calculator2", PackageCategory::OPTIONAL, "Calculator");
    addDatabaseEntry("com.android.calendar", PackageCategory::OPTIONAL, "Calendar");
    addDatabaseEntry("com.android.captiveportallogin", PackageCategory::DO_NOT_TOUCH, "Captive portal login");
    addDatabaseEntry("com.android.carrierconfig", PackageCategory::DO_NOT_TOUCH, "Carrier config");
    addDatabaseEntry("com.android.cellbroadcastreceiver", PackageCategory::DO_NOT_TOUCH, "Cell broadcast receiver");
    addDatabaseEntry("com.android.contacts", PackageCategory::DO_NOT_TOUCH, "Contacts");
    addDatabaseEntry("com.android.deskclock", PackageCategory::OPTIONAL, "Desk clock");
    addDatabaseEntry("com.android.documentsui", PackageCategory::DO_NOT_TOUCH, "Documents UI");
    addDatabaseEntry("com.android.egg", PackageCategory::UNCATEGORIZED, "Easter egg/unknown");
    addDatabaseEntry("com.android.email", PackageCategory::OPTIONAL, "Email");
    addDatabaseEntry("com.android.externalstorage", PackageCategory::DO_NOT_TOUCH, "External storage");
    addDatabaseEntry("com.android.gallery3d", PackageCategory::OPTIONAL, "Gallery");
    addDatabaseEntry("com.android.hotwordenrollment.okgoogle", PackageCategory::DO_NOT_TOUCH, "Hotword enrollment");
    addDatabaseEntry("com.android.hotwordenrollment.xgoogle", PackageCategory::DO_NOT_TOUCH, "Hotword enrollment (x)");
    addDatabaseEntry("com.android.htmlviewer", PackageCategory::OPTIONAL, "HTML viewer");
    addDatabaseEntry("com.android.ims", PackageCategory::DO_NOT_TOUCH, "IMS services");
    addDatabaseEntry("com.android.keychain", PackageCategory::DO_NOT_TOUCH, "Keychain");
    addDatabaseEntry("com.android.launcher", PackageCategory::DO_NOT_TOUCH, "Launcher");
    addDatabaseEntry("com.android.launcher3", PackageCategory::DO_NOT_TOUCH, "Launcher3");
    addDatabaseEntry("com.android.location.fused", PackageCategory::DO_NOT_TOUCH, "Fused location");
    addDatabaseEntry("com.android.music", PackageCategory::OPTIONAL, "Music app");
    addDatabaseEntry("com.android.networkstack", PackageCategory::DO_NOT_TOUCH, "Network stack");
    addDatabaseEntry("com.android.nfc", PackageCategory::DO_NOT_TOUCH, "NFC");
    addDatabaseEntry("com.android.packageinstaller", PackageCategory::DO_NOT_TOUCH, "Package installer");
    addDatabaseEntry("com.android.permissioncontroller", PackageCategory::DO_NOT_TOUCH, "Permission controller");
    addDatabaseEntry("com.android.phone", PackageCategory::DO_NOT_TOUCH, "Phone");
    addDatabaseEntry("com.android.printspooler", PackageCategory::DO_NOT_TOUCH, "Print spooler");
    addDatabaseEntry("com.android.providers.calendar", PackageCategory::DO_NOT_TOUCH, "Providers: calendar");
    addDatabaseEntry("com.android.providers.contacts", PackageCategory::DO_NOT_TOUCH, "Providers: contacts");
    addDatabaseEntry("com.android.providers.downloads", PackageCategory::DO_NOT_TOUCH, "Providers: downloads");
    addDatabaseEntry("com.android.providers.media", PackageCategory::DO_NOT_TOUCH, "Providers: media");
    addDatabaseEntry("com.android.providers.partnerbookmarks", PackageCategory::OPTIONAL, "Partner bookmarks");
    addDatabaseEntry("com.android.providers.settings", PackageCategory::DO_NOT_TOUCH, "Providers: settings");
    addDatabaseEntry("com.android.providers.telephony", PackageCategory::DO_NOT_TOUCH, "Providers: telephony");
    addDatabaseEntry("com.android.proxyhandler", PackageCategory::DO_NOT_TOUCH, "Proxy handler");
    addDatabaseEntry("com.android.server.telecom", PackageCategory::DO_NOT_TOUCH, "Server telecom");
    addDatabaseEntry("com.android.settings", PackageCategory::DO_NOT_TOUCH, "Settings");
    addDatabaseEntry("com.android.shell", PackageCategory::DO_NOT_TOUCH, "Shell");
    addDatabaseEntry("com.android.soundrecorder", PackageCategory::OPTIONAL, "Sound recorder");
    addDatabaseEntry("com.android.systemui", PackageCategory::DO_NOT_TOUCH, "System UI");
    addDatabaseEntry("com.android.vending", PackageCategory::DO_NOT_TOUCH, "Google Play Store");
    addDatabaseEntry("com.android.wallpaper.livepicker", PackageCategory::OPTIONAL, "Live wallpaper picker");
    addDatabaseEntry("com.android.wifi", PackageCategory::DO_NOT_TOUCH, "WiFi service");
    addDatabaseEntry("com.bbk.appstore", PackageCategory::SAFE_TO_REMOVE, "BBK appstore");
    addDatabaseEntry("com.bbk.theme", PackageCategory::SAFE_TO_REMOVE, "BBK theme");
    addDatabaseEntry("com.booking", PackageCategory::SAFE_TO_REMOVE, "Booking app");
    addDatabaseEntry("com.booking.companion", PackageCategory::SAFE_TO_REMOVE, "Booking companion");
    addDatabaseEntry("com.booking.us", PackageCategory::SAFE_TO_REMOVE, "Booking US");
    addDatabaseEntry("com.brave.browser", PackageCategory::USER_APP, "Brave browser");
    addDatabaseEntry("com.coloros.activation", PackageCategory::UNCATEGORIZED, "ColorOS activation");
    addDatabaseEntry("com.coloros.filemanager", PackageCategory::OPTIONAL, "File manager");
    addDatabaseEntry("com.coloros.gamespace", PackageCategory::SAFE_TO_REMOVE, "Game space");
    addDatabaseEntry("com.coloros.securitypermission", PackageCategory::DO_NOT_TOUCH, "ColorOS security permission");
    addDatabaseEntry("com.coloros.video", PackageCategory::OPTIONAL, "ColorOS video");
    addDatabaseEntry("com.discord", PackageCategory::USER_APP, "Discord");
    addDatabaseEntry("com.facebook.appmanager", PackageCategory::SAFE_TO_REMOVE, "Facebook App Manager");
    addDatabaseEntry("com.facebook.katana", PackageCategory::SAFE_TO_REMOVE, "Facebook");
    addDatabaseEntry("com.facebook.orca", PackageCategory::SAFE_TO_REMOVE, "Facebook Messenger");
    addDatabaseEntry("com.facebook.services", PackageCategory::SAFE_TO_REMOVE, "Facebook services");
    addDatabaseEntry("com.facebook.system", PackageCategory::SAFE_TO_REMOVE, "Facebook system");
    addDatabaseEntry("com.flipkart.android", PackageCategory::USER_APP, "Flipkart");
    addDatabaseEntry("com.google.android.apps.docs", PackageCategory::OPTIONAL, "Google Docs");
    addDatabaseEntry("com.google.android.apps.maps", PackageCategory::OPTIONAL, "Google Maps");
    addDatabaseEntry("com.google.android.apps.messaging", PackageCategory::OPTIONAL, "Google Messages");
    addDatabaseEntry("com.google.android.apps.nexuslauncher", PackageCategory::OPTIONAL, "Nexus Launcher");
    addDatabaseEntry("com.google.android.apps.photos", PackageCategory::OPTIONAL, "Google Photos");
    addDatabaseEntry("com.google.android.apps.translate", PackageCategory::OPTIONAL, "Google Translate");
    addDatabaseEntry("com.google.android.apps.walletnfcrel", PackageCategory::USER_APP, "Google Wallet");
    addDatabaseEntry("com.google.android.apps.wallpaper", PackageCategory::OPTIONAL, "Google Wallpaper");
    addDatabaseEntry("com.google.android.apps.youtube.music", PackageCategory::USER_APP, "YouTube Music");
    addDatabaseEntry("com.google.android.configupdater", PackageCategory::DO_NOT_TOUCH, "Config updater");
    addDatabaseEntry("com.google.android.contacts", PackageCategory::DO_NOT_TOUCH, "Google Contacts");
    addDatabaseEntry("com.google.android.dialer", PackageCategory::DO_NOT_TOUCH, "Google Dialer");
    addDatabaseEntry("com.google.android.ext.services", PackageCategory::DO_NOT_TOUCH, "Google ext services");
    addDatabaseEntry("com.google.android.ext.shared", PackageCategory::DO_NOT_TOUCH, "Google ext shared");
    addDatabaseEntry("com.google.android.feedback", PackageCategory::ANALYTICS, "Google feedback/analytics");
    addDatabaseEntry("com.google.android.gm", PackageCategory::OPTIONAL, "Gmail");
    addDatabaseEntry("com.google.android.gms", PackageCategory::DO_NOT_TOUCH, "Google Play Services");
    addDatabaseEntry("com.google.android.googlequicksearchbox", PackageCategory::OPTIONAL, "Google Search");
    addDatabaseEntry("com.google.android.gsf", PackageCategory::DO_NOT_TOUCH, "Google Services Framework");
    addDatabaseEntry("com.google.android.modulemetadata", PackageCategory::DO_NOT_TOUCH, "Module metadata");
    addDatabaseEntry("com.google.android.odad", PackageCategory::ANALYTICS, "ODAD analytics");
    addDatabaseEntry("com.google.android.onetimeinitializer", PackageCategory::DO_NOT_TOUCH, "One-time initializer");
    addDatabaseEntry("com.google.android.partnersetup", PackageCategory::DO_NOT_TOUCH, "Partner setup");
    addDatabaseEntry("com.google.android.permissioncontroller", PackageCategory::DO_NOT_TOUCH, "Permission controller");
    addDatabaseEntry("com.google.android.syncadapters.calendar", PackageCategory::DO_NOT_TOUCH, "Sync adapter: calendar");
    addDatabaseEntry("com.google.android.syncadapters.contacts", PackageCategory::DO_NOT_TOUCH, "Sync adapter: contacts");
    addDatabaseEntry("com.google.android.youtube", PackageCategory::USER_APP, "YouTube");
    addDatabaseEntry("com.heytap.browser", PackageCategory::SAFE_TO_REMOVE, "HeyTap browser");
    addDatabaseEntry("com.heytap.community", PackageCategory::SAFE_TO_REMOVE, "HeyTap community");
    addDatabaseEntry("com.heytap.market", PackageCategory::SAFE_TO_REMOVE, "HeyTap market");
    addDatabaseEntry("com.heytap.themestore", PackageCategory::SAFE_TO_REMOVE, "HeyTap theme store");
    addDatabaseEntry("com.hotstar.tv", PackageCategory::USER_APP, "Hotstar");
    addDatabaseEntry("com.huawei.android.launcher", PackageCategory::OPTIONAL, "Huawei launcher");
    addDatabaseEntry("com.huawei.calendar", PackageCategory::OPTIONAL, "Huawei calendar");
    addDatabaseEntry("com.huawei.email", PackageCategory::OPTIONAL, "Huawei email");
    addDatabaseEntry("com.huawei.gallery", PackageCategory::OPTIONAL, "Huawei gallery");
    addDatabaseEntry("com.huawei.hwid", PackageCategory::OPTIONAL, "Huawei HWID");
    addDatabaseEntry("com.huawei.music", PackageCategory::OPTIONAL, "Huawei music");
    addDatabaseEntry("com.huawei.systemmanager", PackageCategory::DO_NOT_TOUCH, "Huawei system manager");
    addDatabaseEntry("com.huawei.systemmetrics", PackageCategory::ANALYTICS, "Huawei system metrics");
    addDatabaseEntry("com.instagram.android", PackageCategory::USER_APP, "Instagram");
    addDatabaseEntry("com.jio.join", PackageCategory::USER_APP, "Jio Join");
    addDatabaseEntry("com.jio.media.ondemand", PackageCategory::USER_APP, "Jio media");
    addDatabaseEntry("com.jio.myjio", PackageCategory::USER_APP, "MyJio");
    addDatabaseEntry("com.lge.diagnostics", PackageCategory::ANALYTICS, "LG diagnostics");
    addDatabaseEntry("com.linkedin.android", PackageCategory::SAFE_TO_REMOVE, "LinkedIn");
    addDatabaseEntry("com.mediatek.ims", PackageCategory::DO_NOT_TOUCH, "MediaTek IMS");
    addDatabaseEntry("com.microsoft.office.excel", PackageCategory::USER_APP, "MS Excel");
    addDatabaseEntry("com.microsoft.office.officehubrow", PackageCategory::SAFE_TO_REMOVE, "MS Office hub");
    addDatabaseEntry("com.microsoft.office.word", PackageCategory::USER_APP, "MS Word");
    addDatabaseEntry("com.microsoft.skydrive", PackageCategory::SAFE_TO_REMOVE, "MS SkyDrive");
    addDatabaseEntry("com.microsoft.teams", PackageCategory::SAFE_TO_REMOVE, "MS Teams");
    addDatabaseEntry("com.miui.analytics", PackageCategory::ANALYTICS, "MIUI analytics");
    addDatabaseEntry("com.miui.android.fashiongallery", PackageCategory::SAFE_TO_REMOVE, "MIUI fashion gallery");
    addDatabaseEntry("com.miui.bugreport", PackageCategory::SAFE_TO_REMOVE, "MIUI bugreport");
    addDatabaseEntry("com.miui.calculator", PackageCategory::DO_NOT_TOUCH, "MIUI calculator");
    addDatabaseEntry("com.miui.cloudservice", PackageCategory::DO_NOT_TOUCH, "MIUI cloud service");
    addDatabaseEntry("com.miui.daemon", PackageCategory::DO_NOT_TOUCH, "MIUI daemon");
    addDatabaseEntry("com.miui.gallery", PackageCategory::DO_NOT_TOUCH, "MIUI gallery");
    addDatabaseEntry("com.miui.home", PackageCategory::DO_NOT_TOUCH, "MIUI home");
    addDatabaseEntry("com.miui.hybrid", PackageCategory::SAFE_TO_REMOVE, "MIUI hybrid");
    addDatabaseEntry("com.miui.hybrid.accessory", PackageCategory::SAFE_TO_REMOVE, "MIUI hybrid accessory");
    addDatabaseEntry("com.miui.msa.global", PackageCategory::ANALYTICS, "MIUI MSA global");
    addDatabaseEntry("com.miui.notes", PackageCategory::OPTIONAL, "MIUI notes");
    addDatabaseEntry("com.miui.player", PackageCategory::SAFE_TO_REMOVE, "MIUI player");
    addDatabaseEntry("com.miui.powerkeeper", PackageCategory::DO_NOT_TOUCH, "MIUI powerkeeper");
    addDatabaseEntry("com.miui.securitycenter", PackageCategory::DO_NOT_TOUCH, "MIUI security center");
    addDatabaseEntry("com.miui.securitycore", PackageCategory::DO_NOT_TOUCH, "MIUI security core");
    addDatabaseEntry("com.miui.videoplayer", PackageCategory::SAFE_TO_REMOVE, "MIUI videoplayer");
    addDatabaseEntry("com.miui.weather2", PackageCategory::SAFE_TO_REMOVE, "MIUI weather");
    addDatabaseEntry("com.miui.yellowpage", PackageCategory::SAFE_TO_REMOVE, "MIUI yellowpage");
    addDatabaseEntry("com.motorola.moto.calculator", PackageCategory::OPTIONAL, "Motorola calculator");
    addDatabaseEntry("com.motorola.moto.clock", PackageCategory::OPTIONAL, "Motorola clock");
    addDatabaseEntry("com.mtk.system.receiver", PackageCategory::UNCATEGORIZED, "MTK system receiver");
    addDatabaseEntry("com.myntra.android", PackageCategory::USER_APP, "Myntra");
    addDatabaseEntry("com.netflix.mediaclient", PackageCategory::SAFE_TO_REMOVE, "Netflix");
    addDatabaseEntry("com.netflix.partner.activation", PackageCategory::SAFE_TO_REMOVE, "Netflix partner activation");
    addDatabaseEntry("com.netflix.watch", PackageCategory::USER_APP, "Netflix watch");
    addDatabaseEntry("com.olacabs.customer", PackageCategory::USER_APP, "Ola");
    addDatabaseEntry("com.oneplus.filemanager", PackageCategory::OPTIONAL, "OnePlus filemanager");
    addDatabaseEntry("com.oneplus.gallery", PackageCategory::OPTIONAL, "OnePlus gallery");
    addDatabaseEntry("com.oneplus.membership", PackageCategory::SAFE_TO_REMOVE, "OnePlus membership");
    addDatabaseEntry("com.oneplus.store", PackageCategory::SAFE_TO_REMOVE, "OnePlus store");
    addDatabaseEntry("com.opera.browser", PackageCategory::USER_APP, "Opera");
    addDatabaseEntry("com.opera.preinstall", PackageCategory::SAFE_TO_REMOVE, "Opera preinstall");
    addDatabaseEntry("com.oplus.onetrace", PackageCategory::ANALYTICS, "Oplus onetrace");
    addDatabaseEntry("com.oplus.postmanservice", PackageCategory::SAFE_TO_REMOVE, "Oplus postman service");
    addDatabaseEntry("com.oplus.securitypermission", PackageCategory::DO_NOT_TOUCH, "Oplus security permission");
    addDatabaseEntry("com.oplus.statistics.rom", PackageCategory::ANALYTICS, "Oplus statistics");
    addDatabaseEntry("com.paypal.android.p2pmobile", PackageCategory::USER_APP, "PayPal");
    addDatabaseEntry("com.phonepe.app", PackageCategory::USER_APP, "PhonePe");
    addDatabaseEntry("com.primevideo", PackageCategory::USER_APP, "Prime Video");
    addDatabaseEntry("com.qcril", PackageCategory::DO_NOT_TOUCH, "QCRIL");
    addDatabaseEntry("com.qti", PackageCategory::DO_NOT_TOUCH, "QTI");
    addDatabaseEntry("com.qti.qualcomm.deviceinfo", PackageCategory::DO_NOT_TOUCH, "Qualcomm device info");
    addDatabaseEntry("com.qualcomm.location", PackageCategory::DO_NOT_TOUCH, "Qualcomm location");
    addDatabaseEntry("com.qualcomm.qcrilmsgtunnel", PackageCategory::DO_NOT_TOUCH, "Qualcomm qcrilmsgtunnel");
    addDatabaseEntry("com.qualcomm.qti", PackageCategory::DO_NOT_TOUCH, "Qualcomm qti");
    addDatabaseEntry("com.qualcomm.qti.auth.fidocryptoservice", PackageCategory::UNCATEGORIZED, "Qualcomm fidocrypto");
    addDatabaseEntry("com.qualcomm.timeservice", PackageCategory::DO_NOT_TOUCH, "Qualcomm timeservice");
    addDatabaseEntry("com.realmestore.app", PackageCategory::SAFE_TO_REMOVE, "Realmestore");
    addDatabaseEntry("com.reddit.frontpage", PackageCategory::USER_APP, "Reddit");
    addDatabaseEntry("com.samsung.android.app.notes", PackageCategory::DO_NOT_TOUCH, "Samsung Notes");
    addDatabaseEntry("com.samsung.android.app.sbrowser", PackageCategory::DO_NOT_TOUCH, "Samsung Browser");
    addDatabaseEntry("com.samsung.android.app.spage", PackageCategory::SAFE_TO_REMOVE, "Samsung Spage");
    addDatabaseEntry("com.samsung.android.app.tips", PackageCategory::SAFE_TO_REMOVE, "Samsung Tips");
    addDatabaseEntry("com.samsung.android.aremoji", PackageCategory::SAFE_TO_REMOVE, "Samsung Aremoji");
    addDatabaseEntry("com.samsung.android.arzone", PackageCategory::SAFE_TO_REMOVE, "Samsung Arzone");
    addDatabaseEntry("com.samsung.android.brightnessbackupservice", PackageCategory::UNCATEGORIZED, "Samsung brightness backup");
    addDatabaseEntry("com.samsung.android.calendar", PackageCategory::OPTIONAL, "Samsung calendar");
    addDatabaseEntry("com.samsung.android.dialer", PackageCategory::DO_NOT_TOUCH, "Samsung dialer");
    addDatabaseEntry("com.samsung.android.email.provider", PackageCategory::OPTIONAL, "Samsung email provider");
    addDatabaseEntry("com.samsung.android.game.gamehome", PackageCategory::SAFE_TO_REMOVE, "Samsung Game Home");
    addDatabaseEntry("com.samsung.android.game.gametools", PackageCategory::SAFE_TO_REMOVE, "Samsung Game Tools");
    addDatabaseEntry("com.samsung.android.game.gos", PackageCategory::SAFE_TO_REMOVE, "Samsung Game Optimizing Service");
    addDatabaseEntry("com.samsung.android.incallui", PackageCategory::DO_NOT_TOUCH, "Samsung incall UI");
    addDatabaseEntry("com.samsung.android.messaging", PackageCategory::OPTIONAL, "Samsung messaging");
    addDatabaseEntry("com.samsung.android.mobileservice", PackageCategory::DO_NOT_TOUCH, "Samsung mobile service");
    addDatabaseEntry("com.samsung.android.networkstack", PackageCategory::DO_NOT_TOUCH, "Samsung network stack");
    addDatabaseEntry("com.samsung.android.providers.contacts", PackageCategory::DO_NOT_TOUCH, "Samsung providers contacts");
    addDatabaseEntry("com.samsung.android.securitylogagent", PackageCategory::ANALYTICS, "Samsung security log agent");
    addDatabaseEntry("com.samsung.android.sm.policy", PackageCategory::UNCATEGORIZED, "Samsung SM policy");
    addDatabaseEntry("com.samsung.android.stickercenter", PackageCategory::SAFE_TO_REMOVE, "Samsung sticker center");
    addDatabaseEntry("com.samsung.android.themecenter", PackageCategory::SAFE_TO_REMOVE, "Samsung theme center");
    addDatabaseEntry("com.samsung.android.themestore", PackageCategory::SAFE_TO_REMOVE, "Samsung theme store");
    addDatabaseEntry("com.samsung.android.tvplus", PackageCategory::SAFE_TO_REMOVE, "Samsung TV Plus");
    addDatabaseEntry("com.samsung.android.weather", PackageCategory::OPTIONAL, "Samsung weather");
    addDatabaseEntry("com.sec.android.diagmonagent", PackageCategory::ANALYTICS, "Samsung diagmon agent");
    addDatabaseEntry("com.sec.bcservice", PackageCategory::UNCATEGORIZED, "SEC BC service");
    addDatabaseEntry("com.slack", PackageCategory::USER_APP, "Slack");
    addDatabaseEntry("com.snapdeal.main", PackageCategory::USER_APP, "Snapdeal");
    addDatabaseEntry("com.spotify.music", PackageCategory::USER_APP, "Spotify");
    addDatabaseEntry("com.swiggy.android", PackageCategory::USER_APP, "Swiggy");
    addDatabaseEntry("com.tencent.soter.soterserver", PackageCategory::SAFE_TO_REMOVE, "Tencent soter server");
    addDatabaseEntry("com.tiktok.musically", PackageCategory::SAFE_TO_REMOVE, "TikTok (Musically)");
    addDatabaseEntry("com.truecaller", PackageCategory::USER_APP, "Truecaller");
    addDatabaseEntry("com.twitter.android", PackageCategory::USER_APP, "Twitter");
    addDatabaseEntry("com.ubercab", PackageCategory::USER_APP, "Uber");
    addDatabaseEntry("com.vivo.analytics", PackageCategory::ANALYTICS, "Vivo analytics");
    addDatabaseEntry("com.vivo.appstore", PackageCategory::SAFE_TO_REMOVE, "Vivo appstore");
    addDatabaseEntry("com.vivo.browser", PackageCategory::SAFE_TO_REMOVE, "Vivo browser");
    addDatabaseEntry("com.vivo.daemonService", PackageCategory::UNCATEGORIZED, "Vivo daemon service");
    addDatabaseEntry("com.vivo.easyshare", PackageCategory::SAFE_TO_REMOVE, "Vivo easyshare");
    addDatabaseEntry("com.vivo.email", PackageCategory::OPTIONAL, "Vivo email");
    addDatabaseEntry("com.vivo.gallery", PackageCategory::OPTIONAL, "Vivo gallery");
    addDatabaseEntry("com.vivo.pem", PackageCategory::DO_NOT_TOUCH, "Vivo PEM");
    addDatabaseEntry("com.whatsapp", PackageCategory::USER_APP, "WhatsApp");
    addDatabaseEntry("com.whatsapp.w4b", PackageCategory::USER_APP, "WhatsApp Business");
    addDatabaseEntry("com.x.android", PackageCategory::USER_APP, "X app");
    addDatabaseEntry("com.xiaomi.bluetooth", PackageCategory::DO_NOT_TOUCH, "Xiaomi Bluetooth");
    addDatabaseEntry("com.xiaomi.discover", PackageCategory::SAFE_TO_REMOVE, "Xiaomi Discover");
    addDatabaseEntry("com.xiaomi.gamecenter", PackageCategory::SAFE_TO_REMOVE, "Xiaomi Game Center");
    addDatabaseEntry("com.xiaomi.mipicks", PackageCategory::SAFE_TO_REMOVE, "Xiaomi MiPicks");
    addDatabaseEntry("com.xiaomi.xmsf", PackageCategory::DO_NOT_TOUCH, "Xiaomi XMSF");
    addDatabaseEntry("com.zhiliaoapp.musically", PackageCategory::SAFE_TO_REMOVE, "Zhiliao Musically");
    addDatabaseEntry("com.zomato.android", PackageCategory::USER_APP, "Zomato");
    addDatabaseEntry("in.org.npci.upiapp", PackageCategory::USER_APP, "NPCI UPI app");
    addDatabaseEntry("net.one97.paytm", PackageCategory::USER_APP, "Paytm");
    addDatabaseEntry("org.codeaurora.ims", PackageCategory::DO_NOT_TOUCH, "CodeAurora IMS");
    addDatabaseEntry("org.mozilla.firefox", PackageCategory::USER_APP, "Firefox");
    addDatabaseEntry("org.telegram.messenger", PackageCategory::USER_APP, "Telegram");
    addDatabaseEntry("us.zoom.videomeetings", PackageCategory::USER_APP, "Zoom");
    addDatabaseEntry("com.android.stk", PackageCategory::DO_NOT_TOUCH, "SIM Toolkit");
    addDatabaseEntry("com.android.managedprovisioning", PackageCategory::DO_NOT_TOUCH, "Managed provisioning");
    addDatabaseEntry("com.android.se", PackageCategory::DO_NOT_TOUCH, "Secure element");
    addDatabaseEntry("com.android.wallpaperbackup", PackageCategory::OPTIONAL, "Wallpaper backup");
    addDatabaseEntry("com.android.emergency", PackageCategory::DO_NOT_TOUCH, "Emergency information");
    addDatabaseEntry("com.android.calllogbackup", PackageCategory::OPTIONAL, "Call log backup");
    addDatabaseEntry("com.android.wallpapercropper", PackageCategory::OPTIONAL, "Wallpaper cropper");
    addDatabaseEntry("com.google.android.marvin.talkback", PackageCategory::OPTIONAL, "TalkBack accessibility");
    addDatabaseEntry("com.google.android.tts", PackageCategory::OPTIONAL, "Google Text-to-Speech");
    addDatabaseEntry("com.google.android.calendar", PackageCategory::OPTIONAL, "Google Calendar");
    addDatabaseEntry("com.google.android.keep", PackageCategory::OPTIONAL, "Google Keep");
    addDatabaseEntry("com.google.android.apps.docs.editors.sheets", PackageCategory::USER_APP, "Google Sheets");
    addDatabaseEntry("com.google.android.apps.docs.editors.slides", PackageCategory::USER_APP, "Google Slides");
    addDatabaseEntry("com.google.android.apps.tachyon", PackageCategory::OPTIONAL, "Google Meet");
    addDatabaseEntry("com.google.android.apps.magazines", PackageCategory::SAFE_TO_REMOVE, "Google News");
    addDatabaseEntry("com.google.android.music", PackageCategory::SAFE_TO_REMOVE, "Google Play Music");
    addDatabaseEntry("com.google.ar.lens", PackageCategory::OPTIONAL, "Google Lens");
    addDatabaseEntry("com.google.android.apps.wellbeing", PackageCategory::OPTIONAL, "Digital Wellbeing");
    addDatabaseEntry("com.google.android.projection.gearhead", PackageCategory::OPTIONAL, "Android Auto");
    addDatabaseEntry("com.google.android.apps.subscriptions.red", PackageCategory::USER_APP, "YouTube Premium");
    addDatabaseEntry("com.samsung.android.bixby.agent", PackageCategory::SAFE_TO_REMOVE, "Samsung Bixby Agent");
    addDatabaseEntry("com.samsung.android.bixby.wakeup", PackageCategory::SAFE_TO_REMOVE, "Samsung Bixby Wakeup");
    addDatabaseEntry("com.samsung.android.visionintelligence", PackageCategory::SAFE_TO_REMOVE, "Samsung Vision Intelligence");
    addDatabaseEntry("com.miui.miwallpaper", PackageCategory::OPTIONAL, "MIUI wallpaper");
    addDatabaseEntry("com.miui.screenrecorder", PackageCategory::OPTIONAL, "MIUI screen recorder");
    addDatabaseEntry("com.oneplus.soundrecorder", PackageCategory::OPTIONAL, "OnePlus sound recorder");
    addDatabaseEntry("com.oplus.games", PackageCategory::SAFE_TO_REMOVE, "Oplus Games");
    addDatabaseEntry("com.vivo.weather", PackageCategory::OPTIONAL, "Vivo weather");
    addDatabaseEntry("com.transsion.phonemaster", PackageCategory::SAFE_TO_REMOVE, "Phone Master");
    addDatabaseEntry("com.transsion.magicshow", PackageCategory::SAFE_TO_REMOVE, "Transsion Magic Show");

    // === END GENERATED DATABASE ENTRIES ===

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

PackageClassification PackageClassifier::Classify(const Package &package)
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

    const auto *dbEntry = GetDatabaseEntry(package.packageName);

    if (dbEntry)
    {
        return *dbEntry;
    }

    // ===== OVERLAY / RRO PROTECTION =====

    if (MatchesAnyPattern(pkgLower,
                          {".overlay",
                           ".rro"}))
    {
        SetCritical(
            result,
            "System theme or resource overlay",
            "Removing this may break the device interface or themes");

        return result;
    }

    // ===== PACKAGE.H KNOWN ANALYTICS / BLOATWARE =====

    if (package.IsAnalytics())
    {
        SetAnalytics(
            result,
            "Analytics or tracking package",
            "Collects usage, telemetry, or advertising information",
            92);

        return result;
    }

    if (package.IsKnownBloatware())
    {
        SetSafe(
            result,
            "Known bloatware package",
            "Preinstalled or partner app that is usually safe to remove",
            88);

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
                          {"qti",
                           "qualcomm",
                           "qcril",
                           "ims",
                           "iwlan",
                           "uim",
                           "volte",
                           "telephony",
                           "carrier"}))
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
                          {"securitycenter",
                           "securitycore",
                           "powerkeeper",
                           "xmsf",
                           "miui.core",
                           "miui.system"}))
    {
        SetCritical(
            result,
            "Important Xiaomi system component",
            "Required for Xiaomi system stability and security");

        return result;
    }

    // Xiaomi cloud/account integrations -> OPTIONAL
    if (MatchesAnyPattern(pkgLower,
                          {"finddevice",
                           "cloudservice",
                           "micloud",
                           "backup"}))
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
                          {"msa",
                           "mipicks",
                           "analytics",
                           "hybrid",
                           "browser",
                           "videoplayer",
                           "music",
                           "joyose",
                           "weather",
                           "scanner",
                           "compass"}))
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
                          {"analytics",
                           "telemetry",
                           "tracker",
                           "ads",
                           "admob",
                           "advert"}))
    {
        SetAnalytics(
            result,
            "Analytics or tracking package",
            "Collects usage data, telemetry, or advertising information",
            92);

        return result;
    }

    // ===== USER INSTALLED =====
    // NOTE: We intentionally do not return immediately for user-installed
    // packages here so that pattern/database-driven rules (popular apps,
    // analytics, optional utilities, or explicit database entries) can
    // override the default USER_APP classification.

    // ===== POPULAR USER APPS =====

    if (MatchesAnyPattern(pkgLower,
                          {"facebook",
                           "instagram",
                           "spotify",
                           "netflix",
                           "whatsapp",
                           "telegram",
                           "snapchat",
                           "twitter",
                           "discord",
                           "reddit",
                           "youtube"}))
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
                          {"calculator",
                           "recorder",
                           "notes",
                           "gallery",
                           "compass",
                           "scanner"}))
    {
        SetOptional(
            result,
            "Optional utility app",
            "Useful app but not required for Android to function",
            80);

        return result;
    }

    // After checking patterns and explicit database matches above, use
    // a fallback classification for user-installed apps.
    if (package.type == PackageType::USER)
    {
        result.category = PackageCategory::USER_APP;
        result.reason = "User-installed application";
        result.description = "Installed by the user and safe to remove";
        result.safetyScore = 90;
        result.canBeDisabled = true;

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

const PackageClassification *
PackageClassifier::GetDatabaseEntry(const std::string &packageName) const
{
    auto it = database.find(packageName);

    if (it != database.end())
    {
        return &it->second;
    }

    return nullptr;
}

bool PackageClassifier::MatchesPattern(
    const std::string &packageName,
    const std::string &pattern)
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

OemType PackageClassifier::DetectOemType(const std::string &manufacturer,
                                         const std::string &product,
                                         const std::string &buildBrand)
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
        return "Unknown";
    }
}

std::vector<PackageClassification>
PackageClassifier::ClassifyMultiple(
    const std::vector<Package> &packages,
    PackageCategory category)
{
    std::vector<PackageClassification> result;

    // Prevent duplicate package entries
    std::unordered_set<std::string> seen;

    for (const auto &pkg : packages)
    {
        if (seen.count(pkg.packageName))
        {
            continue;
        }

        seen.insert(pkg.packageName);

        auto classified = Classify(pkg);

        if (classified.category == category)
        {
            result.push_back(classified);
        }
    }

    return result;
}