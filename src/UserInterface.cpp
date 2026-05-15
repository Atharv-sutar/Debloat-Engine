#include "UserInterface.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdlib>
#endif

UserInterface::UserInterface(PackageManager& packageMgr, PackageClassifier& classifier)
    : packageMgr(packageMgr), classifier(classifier)
{
}

void UserInterface::ClearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void UserInterface::ShowHeader(const std::string& title)
{
    std::cout << "\n===============================================================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "===============================================================================\n\n";
}

void UserInterface::ShowSeparator(size_t width)
{
    std::cout << std::string(width, '-') << "\n";
}

void UserInterface::ShowInfo(const std::string& message)
{
    std::cout << "[INFO] " << message << "\n";
}

void UserInterface::ShowWarning(const std::string& message)
{
    std::cout << "[WARNING] " << message << "\n";
}

void UserInterface::ShowError(const std::string& message)
{
    std::cout << "[ERROR] " << message << "\n";
}

void UserInterface::ShowSuccess(const std::string& message)
{
    std::cout << "[SUCCESS] " << message << "\n";
}

void UserInterface::PauseForUser(const std::string& message)
{
    std::cout << "\n" << message << "\n";
    std::string dummy;
    std::getline(std::cin, dummy);
}

int UserInterface::GetIntInput(int min, int max)
{
    std::string input;
    while (true)
    {
        std::cout << "Choice [" << min << "-" << max << "]: ";
        std::getline(std::cin, input);

        if (input.empty()) return -1;

        try
        {
            int choice = std::stoi(input);
            if (choice >= min && choice <= max)
            {
                return choice;
            }
            std::cout << "Please enter a number between " << min << " and " << max << ".\n";
        }
        catch (const std::exception&)
        {
            std::cout << "Invalid input. Please enter a number.\n";
        }
    }
}

std::string UserInterface::GetStringInput(const std::string& prompt)
{
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return input;
}

bool UserInterface::GetUserConfirmation(const std::string& message)
{
    while (true)
    {
        std::string input = GetStringInput(message + " (y/n): ");
        if (input.empty()) continue;

        char first = std::tolower(input[0]);
        if (first == 'y') return true;
        if (first == 'n') return false;

        std::cout << "Please enter 'y' for yes or 'n' for no.\n";
    }
}

MenuOption UserInterface::ShowMainMenu()
{
    ClearScreen();
    ShowHeader("Android DeBloat Tool - Main Menu");

    std::cout << "Device: " << packageMgr.GetDeviceSerial() << "\n";
    std::cout << "Total Packages: " << packageMgr.GetPackageCount() << "\n\n";

    std::cout << "1. Show All Packages\n";
    std::cout << "2. Show Critical System Packages (DO NOT TOUCH)\n";
    std::cout << "3. Show Safe to Remove Packages (Bloatware)\n";
    std::cout << "4. Show Optional Packages\n";
    std::cout << "5. Search Packages\n";
    std::cout << "6. View Package Details\n";
    std::cout << "7. Select Packages to Remove\n";
    std::cout << "8. Exit\n\n";

    int choice = GetIntInput(1, 8);
    if (choice == -1 || choice == 8) return MenuOption::EXIT;

    return static_cast<MenuOption>(choice - 1);
}

void UserInterface::DisplayPackageItem(const Package& package, size_t index, bool selected, const PackageClassification& classification)
{
    std::string selStr = selected ? "[X]" : "[ ]";
    std::string typeStr = (package.type == PackageType::SYSTEM || package.type == PackageType::SYSTEM_UPDATE) ? "SYS" : "USR";

    std::string shortName = package.GetShortName();
    if (shortName.length() > 25) shortName = shortName.substr(0, 22) + "...";

    std::cout << std::setw(3) << index << ". " << selStr << " "
              << std::setw(25) << std::left << shortName << " | "
              << std::setw(3) << typeStr << " | "
              << std::setw(15) << PackageClassifier::GetCategoryName(classification.category) << "\n";
    std::cout << std::right; // Reset alignment
}

void UserInterface::ShowPackageList(const std::vector<Package>& packages, PackageCategory category)
{
    if (packages.empty())
    {
        ShowInfo("No packages to display.");
        PauseForUser();
        return;
    }

    size_t pageSize = 15;
    size_t totalPages = (packages.size() + pageSize - 1) / pageSize;
    size_t currentPage = 0;

    while (true)
    {
        ClearScreen();
        std::string title = "Package List (" + std::to_string(packages.size()) + " packages)";
        if (category != PackageCategory::UNCATEGORIZED)
        {
            title += " - " + PackageClassifier::GetCategoryName(category);
        }
        ShowHeader(title);

        size_t startIdx = currentPage * pageSize;
        size_t endIdx = std::min(startIdx + pageSize, packages.size());

        std::cout << "Index | Sel | Package Name              | Typ | Classification \n";
        ShowSeparator(75);

        for (size_t i = startIdx; i < endIdx; ++i)
        {
            auto classification = classifier.Classify(packages[i]);
            DisplayPackageItem(packages[i], i + 1, false, classification);
        }
        
        ShowSeparator(75);
        std::cout << "Page " << (currentPage + 1) << " of " << totalPages << "\n\n";

        std::cout << "Controls:\n";
        if (currentPage > 0) std::cout << "  [p] Previous page\n";
        if (currentPage < totalPages - 1) std::cout << "  [n] Next page\n";
        std::cout << "  [q] Return to menu\n\n";

        std::string input = GetStringInput("Action: ");
        if (input.empty()) continue;

        char action = std::tolower(input[0]);
        if (action == 'q') break;
        if (action == 'n' && currentPage < totalPages - 1) currentPage++;
        if (action == 'p' && currentPage > 0) currentPage--;
    }
}

void UserInterface::ShowCategorizedPackages(const std::vector<Package>& packages, PackageCategory category)
{
    auto categorized = classifier.ClassifyMultiple(packages, category);
    std::vector<Package> filteredPackages;
    
    // Find the original packages that match the filtered classifications
    for (const auto& pkg : packages)
    {
        auto it = std::find_if(categorized.begin(), categorized.end(),
            [&pkg](const PackageClassification& c) { return c.packageName == pkg.packageName; });
            
        if (it != categorized.end())
        {
            filteredPackages.push_back(pkg);
        }
    }
    
    ShowPackageList(filteredPackages, category);
}

void UserInterface::ShowPackageDetails(const Package& package, const PackageClassification& classification)
{
    ClearScreen();
    ShowHeader("Package Details");

    std::cout << "Name:           " << package.packageName << "\n";
    std::cout << "Type:           " << (package.type == PackageType::SYSTEM ? "SYSTEM" : "USER") << "\n";
    std::cout << "Version Code:   " << (package.versionCode.empty() ? "Unknown" : package.versionCode) << "\n";
    std::cout << "Version Name:   " << (package.versionName.empty() ? "Unknown" : package.versionName) << "\n";
    
    ShowSeparator();
    
    std::cout << "Classification: " << PackageClassifier::GetCategoryName(classification.category) << "\n";
    std::cout << "Safety Score:   " << classification.safetyScore << "/100\n";
    std::cout << "Can Disable:    " << (classification.canBeDisabled ? "Yes" : "No") << "\n";
    std::cout << "OEM Specific:   " << (classification.isOemSpecific ? "Yes" : "No") << "\n\n";
    
    std::cout << "Description:\n  " << classification.description << "\n\n";
    std::cout << "Reasoning:\n  " << classification.reason << "\n";
    
    PauseForUser();
}

std::vector<PackageClassification> UserInterface::BuildRecommendations(const std::vector<Package>& packages)
{
    return classifier.ClassifyMultiple(packages, PackageCategory::SAFE_TO_REMOVE);
}

std::set<std::string> UserInterface::MultiSelectPackages(
    const std::vector<Package>& packages,
    const std::vector<PackageClassification>& recommendations)
{
    std::set<std::string> selected;
    
    if (packages.empty())
    {
        ShowInfo("No packages available for selection.");
        PauseForUser();
        return selected;
    }

    size_t pageSize = 15;
    size_t totalPages = (packages.size() + pageSize - 1) / pageSize;
    size_t currentPage = 0;

    while (true)
    {
        ClearScreen();
        ShowHeader("Select Packages to Remove");

        size_t startIdx = currentPage * pageSize;
        size_t endIdx = std::min(startIdx + pageSize, packages.size());

        std::cout << "Index | Sel | Package Name              | Typ | Classification \n";
        ShowSeparator(75);

        for (size_t i = startIdx; i < endIdx; ++i)
        {
            auto classification = classifier.Classify(packages[i]);
            bool isSelected = selected.find(packages[i].packageName) != selected.end();
            DisplayPackageItem(packages[i], i + 1, isSelected, classification);
        }
        
        ShowSeparator(75);
        std::cout << "Page " << (currentPage + 1) << " of " << totalPages 
                  << " | Selected: " << selected.size() << "\n\n";

        std::cout << "Controls:\n";
        std::cout << "  [1-" << packages.size() << "] Toggle selection by index\n";
        std::cout << "  [r] Select all 'Safe to Remove' recommendations\n";
        std::cout << "  [c] Clear all selections\n";
        if (currentPage > 0) std::cout << "  [p] Previous page\n";
        if (currentPage < totalPages - 1) std::cout << "  [n] Next page\n";
        std::cout << "  [d] Done (proceed to confirmation)\n";
        std::cout << "  [q] Cancel and return to menu\n\n";

        std::string input = GetStringInput("Action: ");
        if (input.empty()) continue;

        char action = std::tolower(input[0]);
        if (action == 'q') return std::set<std::string>(); // Return empty set to cancel
        if (action == 'd') break;
        if (action == 'n' && currentPage < totalPages - 1) currentPage++;
        if (action == 'p' && currentPage > 0) currentPage--;
        if (action == 'c') selected.clear();
        if (action == 'r')
        {
            for (const auto& rec : recommendations)
            {
                selected.insert(rec.packageName);
            }
        }
        
        // Try parsing as number for index toggle
        try
        {
            size_t idx = std::stoull(input);
            if (idx >= 1 && idx <= packages.size())
            {
                const Package& p = packages[idx - 1];
                auto c = classifier.Classify(p);
                
                if (c.category == PackageCategory::DO_NOT_TOUCH)
                {
                    ShowCriticalPackageWarning();
                }
                else
                {
                    const std::string& name = p.packageName;
                    if (selected.find(name) != selected.end())
                    {
                        selected.erase(name);
                    }
                    else
                    {
                        selected.insert(name);
                    }
                }
            }
        }
        catch (const std::exception&)
        {
            // Not a number, ignore
        }
    }
    
    return selected;
}

bool UserInterface::ConfirmRemoval(const std::set<std::string>& selected, const std::vector<Package>& packages)
{
    if (selected.empty())
    {
        return false;
    }
    
    ClearScreen();
    ShowHeader("Confirm Package Removal");
    
    std::cout << "You have selected " << selected.size() << " packages for removal/disabling:\n\n";
    
    int count = 1;
    for (const auto& name : selected)
    {
        // Find package to show short name
        auto it = std::find_if(packages.begin(), packages.end(), 
            [&name](const Package& p) { return p.packageName == name; });
            
        std::string dispName = name;
        if (it != packages.end()) dispName = it->GetShortName();
        
        std::cout << "  " << count++ << ". " << dispName << " (" << name << ")\n";
    }
    
    std::cout << "\n";
    ShowWarning("This action will attempt to uninstall these packages for the current user.");
    ShowWarning("If uninstall fails, it will attempt to disable them instead.");
    
    bool confirmed = GetUserConfirmation("\n[CONFIRM] Proceed with removal/disabling?");
    return confirmed;
}

std::string UserInterface::SearchAndSelectPackage()
{
    std::string query = GetStringInput("\nEnter package name or part of it to search (or 'q' to cancel): ");
    if (query == "q" || query.empty())
    {
        return "";
    }
    
    auto allPackages = packageMgr.GetAllPackages();
    auto results = packageMgr.SearchPackages(query);
    
    if (results.empty())
    {
        ShowError("No packages found matching: " + query);
        PauseForUser();
        return "";
    }
    
    ClearScreen();
    ShowHeader("Search Results for: " + query);
    
    std::cout << "Found " << results.size() << " matching packages:\n\n";
    
    for (size_t i = 0; i < results.size() && i < 20; ++i)
    {
        auto classification = classifier.Classify(results[i]);
        std::cout << std::setw(3) << (i + 1) << ". " 
                  << std::setw(40) << std::left << results[i].packageName 
                  << " [" << PackageClassifier::GetCategoryName(classification.category) << "]\n";
    }
    
    if (results.size() > 20)
    {
        std::cout << "... and " << (results.size() - 20) << " more\n";
    }
    
    int choice = GetIntInput(0, std::min(results.size(), static_cast<size_t>(20)));
    if (choice <= 0)
    {
        return "";
    }
    
    return results[choice - 1].packageName;
}

void UserInterface::ShowCriticalPackageWarning()
{
    std::cout << "\n";
    ShowSeparator();
    ShowError("CRITICAL PACKAGE BLOCKED");
    std::cout << "You attempted to select a critical system package.\n";
    std::cout << "Removing this package will likely break your device or cause a bootloop.\n";
    std::cout << "Selection has been blocked for your safety.\n";
    ShowSeparator();
    PauseForUser();
}
