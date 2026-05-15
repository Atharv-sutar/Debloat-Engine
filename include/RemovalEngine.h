#pragma once

#include "Package.h"
#include "AdbManager.h"
#include <string>
#include <vector>
#include <set>
#include <map>

/**
 * Enum for removal/disable actions
 */
enum class RemovalAction
{
    UNINSTALL,          // Attempt uninstall first
    DISABLE,            // Disable package (non-destructive)
    UNINSTALL_OR_DISABLE // Try uninstall, fallback to disable
};

/**
 * Enum for removal result status
 */
enum class RemovalStatus
{
    SUCCESS,            // Package removed/disabled successfully
    FAILED,             // Operation completely failed
    PARTIAL,            // Some packages succeeded, some failed
    CANCELLED,          // User cancelled operation
    PERMISSION_DENIED,  // ADB permission issue
    NOT_INSTALLED,      // Package not found
    UNKNOWN             // Unknown error
};

/**
 * Struct for removal result
 */
struct RemovalResult
{
    std::string packageName;
    RemovalAction action;
    RemovalStatus status;
    std::string message;
    bool canRollback;

    RemovalResult()
        : packageName(""), action(RemovalAction::UNINSTALL),
          status(RemovalStatus::UNKNOWN), message(""),
          canRollback(false) {}

    RemovalResult(const std::string& pkg, RemovalAction act, RemovalStatus stat, const std::string& msg)
        : packageName(pkg), action(act), status(stat),
          message(msg), canRollback(false) {}
};

/**
 * RemovalEngine - Handles package removal and disabling
 * Provides:
 * - Safe package removal via ADB
 * - Non-destructive disable option
 * - Removal queuing and batch operations
 * - Rollback capability for disabled packages
 * - Safety checks before removal
 */
class RemovalEngine
{
public:
    /**
     * Constructor
     * @param deviceSerial Serial number of target device
     */
    explicit RemovalEngine(const std::string& deviceSerial);

    /**
     * Remove a single package
     * @param packageName Package to remove
     * @param action Removal action (uninstall, disable, or try both)
     * @return RemovalResult with status
     */
    RemovalResult RemovePackage(const std::string& packageName,
                                RemovalAction action = RemovalAction::UNINSTALL_OR_DISABLE);

    /**
     * Remove multiple packages
     * @param packageNames Packages to remove
     * @param action Removal action for all
     * @param stopOnError Stop if one fails
     * @return Vector of RemovalResult for each package
     */
    std::vector<RemovalResult> RemoveMultiple(const std::vector<std::string>& packageNames,
                                              RemovalAction action = RemovalAction::UNINSTALL_OR_DISABLE,
                                              bool stopOnError = false);

    /**
     * Disable a package (non-destructive)
     * @param packageName Package to disable
     * @return RemovalResult with status
     */
    RemovalResult DisablePackage(const std::string& packageName);

    /**
     * Uninstall a package completely
     * @param packageName Package to uninstall
     * @return RemovalResult with status
     */
    RemovalResult UninstallPackage(const std::string& packageName);

    /**
     * Check if package is installed
     * @param packageName Package name
     * @return true if installed
     */
    bool IsPackageInstalled(const std::string& packageName);

    /**
     * Check if package is enabled
     * @param packageName Package name
     * @return true if enabled, false if disabled or not found
     */
    bool IsPackageEnabled(const std::string& packageName);

    /**
     * Re-enable a previously disabled package
     * @param packageName Package to enable
     * @return RemovalResult with status
     */
    RemovalResult EnablePackage(const std::string& packageName);

    /**
     * Get status of last operation
     * @return RemovalStatus
     */
    RemovalStatus GetLastStatus() const { return lastStatus; }

    /**
     * Get error message from last operation
     * @return Error message
     */
    std::string GetLastError() const { return lastError; }

    /**
     * Get history of all removal operations
     * @return Vector of RemovalResults
     */
    const std::vector<RemovalResult>& GetRemovalHistory() const { return removalHistory; }

    /**
     * Get list of disabled packages (for potential rollback)
     * @return Vector of package names
     */
    std::vector<std::string> GetDisabledPackages() const;

    /**
     * Rollback all disabled packages (enable them)
     * @return Number of packages re-enabled
     */
    size_t RollbackAllDisabled();

    /**
     * Clear removal history
     */
    void ClearHistory() { removalHistory.clear(); }

private:
    std::string deviceSerial;
    RemovalStatus lastStatus;
    std::string lastError;
    std::vector<RemovalResult> removalHistory;
    std::set<std::string> disabledPackages;  // Track disabled packages for rollback

    /**
     * Execute ADB uninstall command
     * @param packageName Package to uninstall
     * @return true if successful
     */
    bool ExecuteUninstall(const std::string& packageName);

    /**
     * Execute ADB disable command
     * @param packageName Package to disable
     * @return true if successful
     */
    bool ExecuteDisable(const std::string& packageName);

    /**
     * Execute ADB enable command
     * @param packageName Package to enable
     * @return true if successful
     */
    bool ExecuteEnable(const std::string& packageName);

    /**
     * Parse ADB output to determine success
     * @param output ADB command output
     * @param action Action that was performed
     * @return true if operation was successful
     */
    static bool ParseAdbOutput(const std::string& output, RemovalAction action);

    /**
     * Log removal result to history
     * @param result Result to log
     */
    void LogResult(const RemovalResult& result);
};
