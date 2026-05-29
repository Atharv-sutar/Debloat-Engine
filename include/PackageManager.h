#pragma once

#include "Package.h"
#include <vector>
#include <map>
#include <memory>
#include <unordered_set>

/**
 * PackageManager - Manages Android package operations
 * Responsible for:
 * - Fetching package list from connected device
 * - Parsing and storing packages
 * - Filtering and searching packages
 * - Package categorization
 * - Package detail retrieval
 */
class PackageManager
{
public:
    /**
     * Constructor - associates with a specific device
     * @param deviceSerial Serial number of target device
     */
    explicit PackageManager(const std::string& deviceSerial);

    /**
     * Fetch all installed packages from the device
     * Queries: adb shell pm list packages -3 (user) and -s (system)
     * @return true if fetch successful, false otherwise
     */
    bool FetchPackages();

    /**
     * Fetch all packages (system + user)
     * @return true if successful
     */
    bool FetchAllPackages();

    /**
     * Fetch only system packages
     * @return true if successful
     */
    bool FetchSystemPackages();

    /**
     * Fetch only user-installed packages
     * @return true if successful
     */
    bool FetchUserPackages();

    /**
     * Get all packages
     * @return Vector of all Package objects
     */
    const std::vector<Package>& GetAllPackages() const { return allPackages; }

    /**
     * Get system packages only
     * @return Vector of system Package objects
     */
    std::vector<Package> GetSystemPackages() const;

    /**
     * Get user packages only
     * @return Vector of user Package objects
     */
    std::vector<Package> GetUserPackages() const;

    /**
     * Get package count
     * @return Total number of packages
     */
    size_t GetPackageCount() const { return allPackages.size(); }

    /**
     * Get system package count
     * @return Number of system packages
     */
    size_t GetSystemPackageCount() const;

    /**
     * Get user package count
     * @return Number of user packages
     */
    size_t GetUserPackageCount() const;

    /**
     * Get critical (do-not-touch) packages
     * @return Vector of critical system packages
     */
    std::vector<Package> GetCriticalPackages() const;

    /**
     * Get known bloatware packages
     * @return Vector of known bloatware
     */
    std::vector<Package> GetBloatwarePackages() const;

    /**
     * Get analytics/tracking packages
     * @return Vector of analytics packages
     */
    std::vector<Package> GetAnalyticsPackages() const;

    /**
     * Get disabled packages
     * @return Vector of disabled packages
     */
    std::vector<Package> GetDisabledPackages() const;

    /**
     * Get enabled packages
     * @return Vector of enabled packages
     */
    std::vector<Package> GetEnabledPackages() const;

    /**
     * Get disabled package count
     * @return Number of disabled packages
     */
    size_t GetDisabledPackageCount() const;

    /**
     * Get enabled package count
     * @return Number of enabled packages
     */
    size_t GetEnabledPackageCount() const;

    /**
     * Search packages by name (substring match, case-insensitive)
     * @param query Search query
     * @return Vector of matching packages
     */
    std::vector<Package> SearchPackages(const std::string& query) const;

    /**
     * Get package by exact package name
     * @param packageName Full package name
     * @return Package object if found, empty Package if not found
     */
    Package GetPackageByName(const std::string& packageName) const;

    /**
     * Get detailed information about a package
     * Queries: dumpsys package <packageName>
     * @param packageName Full package name
     * @return Package with detailed info
     */
    Package GetPackageDetails(const std::string& packageName);

    /**
     * Check if package is installed
     * @param packageName Full package name
     * @return true if package exists
     */
    bool IsPackageInstalled(const std::string& packageName) const;

    /**
     * Get last error message
     * @return Error description from last operation
     */
    std::string GetLastError() const { return lastError; }

    /**
     * Clear all cached packages and reload
     */
    void ClearCache() { allPackages.clear(); }

    /**
     * Get device serial this manager is working with
     * @return Device serial number
     */
    std::string GetDeviceSerial() const { return deviceSerial; }

private:
    /**
     * Parse package list from adb output
     * Handles formats like: "package:com.package.name"
     * @param output Raw adb command output
     * @param type Package type to assign (SYSTEM or USER)
     * @return Vector of parsed Package objects
     */
    std::vector<Package> ParsePackageList(const std::string& output, PackageType type);

    /**
     * Get currently disabled package names from the device
     * @return Set of disabled package names
     */
    std::unordered_set<std::string> GetDisabledPackageSet() const;

    /**
     * Extract version code from dumpsys output
     * @param dumpsysOutput dumpsys package output
     * @return Version code string
     */
    static std::string ExtractVersionCode(const std::string& dumpsysOutput);

    /**
     * Extract version name from dumpsys output
     * @param dumpsysOutput dumpsys package output
     * @return Version name string
     */
    static std::string ExtractVersionName(const std::string& dumpsysOutput);

    std::string deviceSerial;
    std::vector<Package> allPackages;
    std::map<std::string, Package> packageMap;  // For O(log n) lookup
    std::string lastError;
};
