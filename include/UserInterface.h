#pragma once

#include "Package.h"
#include "PackageManager.h"
#include "PackageClassifier.h"
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <iomanip>

/**
 * Enum for menu options
 */
enum class MenuOption
{
    SHOW_ALL_PACKAGES,
    SHOW_CRITICAL_PACKAGES,
    SHOW_SAFE_TO_REMOVE,
    SHOW_OPTIONAL_PACKAGES,
    SEARCH_PACKAGES,
    VIEW_PACKAGE_DETAILS,
    SELECT_PACKAGES_TO_REMOVE,
    CONFIRM_REMOVAL,
    EXIT
};

/**
 * UserInterface - Handles all CLI interactions
 * Provides:
 * - Menu display and navigation
 * - Multi-selection interface
 * - Package filtering and searching
 * - Confirmation dialogs
 * - Detailed package information display
 */
class UserInterface
{
public:
    /**
     * Constructor
     * @param packageMgr Reference to PackageManager
     * @param classifier Reference to PackageClassifier
     */
    UserInterface(PackageManager& packageMgr, PackageClassifier& classifier);

    /**
     * Display main menu and return user choice
     * @return Selected MenuOption
     */
    MenuOption ShowMainMenu();

    /**
     * Show all packages with pagination
     * @param packages Packages to display
     * @param category Optional category filter
     */
    void ShowPackageList(const std::vector<Package>& packages,
                        PackageCategory category = PackageCategory::UNCATEGORIZED);

    /**
     * Show classified packages by category
     * @param packages Packages to display
     * @param category Category to filter
     */
    void ShowCategorizedPackages(const std::vector<Package>& packages,
                                 PackageCategory category);

    /**
     * Display package details
     * @param package Package to display
     * @param classification Classification info
     */
    void ShowPackageDetails(const Package& package,
                           const PackageClassification& classification);

    /**
     * Multi-select packages for removal
     * @param packages Packages to select from
     * @param recommendations Pre-recommended packages
     * @return Set of selected package names
     */
    std::set<std::string> MultiSelectPackages(
        const std::vector<Package>& packages,
        const std::vector<PackageClassification>& recommendations);

    /**
     * Show selection summary and get confirmation
     * @param selected Selected package names
     * @param packages All packages
     * @return true if user confirms removal
     */
    bool ConfirmRemoval(const std::set<std::string>& selected,
                        const std::vector<Package>& packages);

    /**
     * Search packages by query
     * @return Selected package name or empty string if cancelled
     */
    std::string SearchAndSelectPackage();

    /**
     * Display warning about critical packages
     */
    void ShowCriticalPackageWarning();

    /**
     * Display confirmation prompt
     * @param message Message to display
     * @return true if user confirms (yes/y)
     */
    static bool GetUserConfirmation(const std::string& message);

    /**
     * Get integer input within range
     * @param min Minimum allowed value
     * @param max Maximum allowed value
     * @return User input or -1 if invalid
     */
    static int GetIntInput(int min, int max);

    /**
     * Get string input from user
     * @param prompt Prompt to display
     * @return User input string
     */
    static std::string GetStringInput(const std::string& prompt);

    /**
     * Clear screen (cross-platform)
     */
    static void ClearScreen();

    /**
     * Display formatted header
     * @param title Title text
     */
    static void ShowHeader(const std::string& title);

    /**
     * Display formatted separator
     * @param width Width of separator
     */
    static void ShowSeparator(size_t width = 80);

    /**
     * Display info message
     * @param message Message to display
     */
    static void ShowInfo(const std::string& message);

    /**
     * Display warning message
     * @param message Message to display
     */
    static void ShowWarning(const std::string& message);

    /**
     * Display error message
     * @param message Message to display
     */
    static void ShowError(const std::string& message);

    /**
     * Display success message
     * @param message Message to display
     */
    static void ShowSuccess(const std::string& message);

    /**
     * Pause and wait for user to press Enter
     * @param message Optional message before pause
     */
    static void PauseForUser(const std::string& message = "Press Enter to continue...");

private:
    PackageManager& packageMgr;
    PackageClassifier& classifier;

    /**
     * Display single package with checkbox/selection marker
     * @param package Package to display
     * @param index Package index
     * @param selected Whether package is selected
     * @param classification Classification info
     */
    void DisplayPackageItem(const Package& package,
                           size_t index,
                           bool selected,
                           const PackageClassification& classification);

    /**
     * Build recommendation list
     * @param packages Packages to evaluate
     * @return Vector of classified recommendations
     */
    std::vector<PackageClassification> BuildRecommendations(
        const std::vector<Package>& packages);
};
