#pragma once

#include "Package.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

/**
 * Enum for package categories
 */
enum class PackageCategory
{
    DO_NOT_TOUCH,        // Critical system packages - never remove
    SAFE_TO_REMOVE,      // Known bloatware/safe apps that can be removed
    ANALYTICS,           // Telemetry or tracking packages
    OPTIONAL,            // User's choice - might be useful to some
    USER_APP,            // User-installed app
    UNCATEGORIZED        // Not yet classified
};

/**
 * Enum for OEM types
 */
enum class OemType
{
    STOCK_ANDROID,       // Pure Android (Pixel)
    SAMSUNG,             // Samsung OneUI
    XIAOMI,              // MIUI
    ONEPLUS,             // OxygenOS
    OPPO,                // ColorOS
    VIVO,                // FunTouchOS
    REALME,              // Realme UI
    MOTOROLA,            // Stock-based
    HUAWEI,              // HarmonyOS/EMUI
    NOTHING,             // Nothing OS
    CUSTOM_ROM,          // Custom ROM (LineageOS, etc.)
    UNKNOWN              // Unknown OEM
};

/**
 * Struct containing package classification info
 */
struct PackageClassification
{
    std::string packageName;
    PackageCategory category;
    std::string reason;              // Why it's classified this way
    std::string description;         // What this package does
    bool isOemSpecific;              // Is this OEM-specific?
    OemType oemApplicability;        // Which OEM uses this
    bool canBeDisabled;              // Can be disabled instead of removed
    int safetyScore;                 // 0-100 (higher = safer to remove)

    PackageClassification()
        : packageName(""), category(PackageCategory::UNCATEGORIZED),
          reason(""), description(""), isOemSpecific(false),
          oemApplicability(OemType::UNKNOWN), canBeDisabled(true),
          safetyScore(0) {}

    explicit PackageClassification(const std::string& pkg)
        : packageName(pkg), category(PackageCategory::UNCATEGORIZED),
          reason(""), description(""), isOemSpecific(false),
          oemApplicability(OemType::UNKNOWN), canBeDisabled(true),
          safetyScore(0) {}
};

/**
 * PackageClassifier - Classifies packages into categories with descriptions
 * Provides intelligent categorization based on:
 * - Package name patterns
 * - Known package databases
 * - OEM type
 * - System vs user packages
 * - Device characteristics
 */
class PackageClassifier
{
public:
    /**
     * Constructor
     * @param deviceModel Device model (e.g., "Pixel 6", "SM-G990B")
     * @param oemType OEM type detected from device
     */
    PackageClassifier(const std::string& deviceModel = "", OemType oemType = OemType::UNKNOWN);

    /**
     * Classify a package
     * @param package Package to classify
     * @return PackageClassification with category and reasoning
     */
    PackageClassification Classify(const Package& package);

    /**
     * Detect OEM type from device properties
     * @param manufacturer Device manufacturer
     * @param product Device product name
     * @param buildBrand Device build brand
     * @return Detected OemType
     */
    static OemType DetectOemType(const std::string& manufacturer,
                                  const std::string& product,
                                  const std::string& buildBrand);

    /**
     * Get OEM type name as string
     * @param oemType OEM type enum
     * @return String representation
     */
    static std::string GetOemTypeName(OemType oemType);

    /**
     * Get category name as string
     * @param category Category enum
     * @return String representation
     */
    static std::string GetCategoryName(PackageCategory category);

    /**
     * Get all packages in a specific category
     * @param packages Vector of packages to classify
     * @param category Category to filter
     * @return Classified packages in that category
     */
    std::vector<PackageClassification> ClassifyMultiple(const std::vector<Package>& packages,
                                                         PackageCategory category = PackageCategory::UNCATEGORIZED);

    /**
     * Set current OEM type for classification
     * @param oemType OEM type
     */
    void SetOemType(OemType oemType) { currentOemType = oemType; }

    /**
     * Get current OEM type
     * @return Current OEM type
     */
    OemType GetOemType() const { return currentOemType; }

private:
    /**
     * Get classification database entry for a package
     * @param packageName Full package name
     * @return Classification data or nullptr if not found
     */
    const PackageClassification* GetDatabaseEntry(const std::string& packageName) const;

    /**
     * Build the classification database
     */
    void BuildDatabase();

    /**
     * Check if package matches a pattern
     * @param packageName Full package name
     * @param pattern Pattern to match (supports * wildcard)
     * @return true if matches
     */
    static bool MatchesPattern(const std::string& packageName, const std::string& pattern);

    std::string deviceModel;
    OemType currentOemType;
    std::map<std::string, PackageClassification> database;

    // Static OEM-specific package lists
    static const std::map<std::string, PackageClassification> CORE_DATABASE;
};
