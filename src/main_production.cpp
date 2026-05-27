#include <iostream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <set>
#include <cstdio>
#include "CommandExecutor.h"
#include "AdbManager.h"
#include "PackageManager.h"
#include "PackageClassifier.h"
#include "RemovalEngine.h"
#include "Logger.h"

// Global storage for selected packages
std::set<std::string> selectedPackages;
std::string g_currentDevice = "";
std::string g_currentDeviceModel = "";

void ClearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void PrintHeader()
{
    std::cout << "\n";
    std::cout << "==============================================\n";
    std::cout << "        DeBloat Tool v1.0\n";
    std::cout << "   Android Bloatware Remover\n";
    if (!g_currentDevice.empty())
    {
        std::cout << "   Device: " << g_currentDeviceModel << " (" << g_currentDevice << ")\n";
    }
    std::cout << "==============================================\n";
    std::cout << "\n";
}

void PrintMenu()
{
    std::cout << "==============================================\n";
    std::cout << "                   MAIN MENU\n";
    std::cout << "==============================================\n";
    std::cout << "1. Scan Device & Show Safe Bloatware\n";
    std::cout << "2. Advanced Search (by category)\n";
    std::cout << "3. Remove Selected Packages\n";
    std::cout << "4. View Detailed Report\n";
    std::cout << "5. Exit\n";
    std::cout << "6. Refresh Scan\n";
    std::cout << "7. Switch Device\n";
    std::cout << "==============================================\n";
    std::cout << "\nEnter choice (1-7): ";
}

void PrintCategoryMenu()
{
    std::cout << "==============================================\n";
    std::cout << "        ADVANCED SEARCH - SELECT CATEGORY\n";
    std::cout << "==============================================\n";
    std::cout << "1. Critical (Do not remove)\n";
    std::cout << "2. Bloat (Safe to remove)\n";
    std::cout << "3. Analytics (Tracking/telemetry)\n";
    std::cout << "4. Optional (User's choice)\n";
    std::cout << "5. User (User-installed apps)\n";
    std::cout << "6. Other (Unknown packages)\n";
    std::cout << "7. All Packages\n";
    std::cout << "8. Back to main menu\n";
    std::cout << "==============================================\n";
    std::cout << "\nEnter choice (1-8): ";
}

void DisplayPackagesWithCheckboxes(const std::vector<PackageClassification>& packages,
                                   std::set<std::string>& selected)
{
    if (packages.empty())
    {
        std::cout << "[INFO] No packages in this category.\n\n";
        return;
    }

    std::cout << "\n" << std::string(100, '=') << "\n";
    std::cout << "Packages (" << packages.size() << " total)\n";
    std::cout << std::string(100, '=') << "\n";
    std::cout << "[#]   [Sel]  Package Name                           | Type  | Category\n";
    std::cout << std::string(100, '-') << "\n";

    for (size_t i = 0; i < packages.size(); ++i)
    {
        bool isSelected = selected.count(packages[i].packageName) > 0;
        std::string checkbox = isSelected ? "[X]" : "[ ]"; // erro, it prints ΓÿÉ instead of checkboxes, maybe console doesn't support unicode so
        std::string typeStr = (packages[i].canBeDisabled) ? "USR" : "SYS";
        std::string shortName = packages[i].packageName;
        if (shortName.length() > 35)
            shortName = shortName.substr(0, 32) + "...";

        printf("[%-3zu] %s  %-35s | %5s | %s\n",
               i + 1, checkbox.c_str(), shortName.c_str(), typeStr.c_str(),
               PackageClassifier::GetCategoryName(packages[i].category).c_str());
    }

    std::cout << std::string(100, '=') << "\n";
    std::cout << "Commands: [1-" << packages.size() << "] Toggle, [a] Select All, [c] Clear, [d] Done\n\n";
}

void ManagePackageSelection(const std::vector<PackageClassification>& packages,
                            std::set<std::string>& selected,
                            bool allowCritical = false)
{
    bool selecting = true;
    while (selecting)
    {
        DisplayPackagesWithCheckboxes(packages, selected);
        std::cout << "Enter command: ";
        std::string input;
        std::getline(std::cin, input);

        if (input == "d" || input == "D")
        {
            selecting = false;
        }
        else if (input == "a" || input == "A")
        {
            for (const auto& pkg : packages)
            {
                if (allowCritical || pkg.category != PackageCategory::DO_NOT_TOUCH)
                {
                    selected.insert(pkg.packageName);
                }
            }
        }
        else if (input == "c" || input == "C")
        {
            selected.clear();
        }
        else
        {
            try
            {
                size_t idx = std::stoi(input) - 1;
                if (idx < packages.size())
                {
                    if (allowCritical || packages[idx].category != PackageCategory::DO_NOT_TOUCH)
                    {
                        const auto& pkgName = packages[idx].packageName;
                        if (selected.count(pkgName))
                        {
                            selected.erase(pkgName);
                        }
                        else
                        {
                            selected.insert(pkgName);
                        }
                    }
                    else
                    {
                        std::cout << "[WARNING] Cannot select Critical packages!\n";
                    }
                }
            }
            catch (...)
            {
                // Invalid input, ignore
            }
        }
        if (selecting)
        {
            ClearScreen();
            PrintHeader();
        }
    }
}

std::string SelectDevice(AdbManager& adbMgr)
{
    auto devices = adbMgr.GetConnectedDevices();

    if (devices.empty())
    {
        std::cout << "\n[ERROR] No devices connected!\n";
        return "";
    }

    if (devices.size() == 1)
    {
        if (devices[0].authStatus == AuthStatus::AUTHORIZED)
        {
            return devices[0].serialNumber;
        }
        else
        {
            std::cout << "\n[ERROR] Device not authorized for debugging!\n";
            return "";
        }
    }

    // Multiple devices - let user choose
    std::cout << "\n[*] Multiple devices detected:\n";
    std::cout << std::string(60, '-') << "\n";

    int idx = 1;
    for (const auto& device : devices)
    {
        std::string status = (device.authStatus == AuthStatus::AUTHORIZED) ? "AUTHORIZED" : "UNAUTHORIZED";
        std::cout << "[" << idx << "] " << device.model << " (" << device.serialNumber << ") - " << status << "\n";
        idx++;
    }
    std::cout << std::string(60, '-') << "\n";
    std::cout << "Enter device number (1-" << devices.size() << "): ";

    std::string choice;
    std::getline(std::cin, choice);

    try
    {
        int devIdx = std::stoi(choice) - 1;
        if (devIdx >= 0 && devIdx < static_cast<int>(devices.size()))
        {
            if (devices[devIdx].authStatus == AuthStatus::AUTHORIZED)
            {
                return devices[devIdx].serialNumber;
            }
            else
            {
                std::cout << "[ERROR] Selected device is not authorized!\n";
                return "";
            }
        }
    }
    catch (...)
    {
    }

    std::cout << "[ERROR] Invalid selection!\n";
    return "";
}

bool IsDeviceStillConnected(AdbManager& adbMgr, const std::string& deviceSerial)
{
    auto devices = adbMgr.GetConnectedDevices();
    for (const auto& device : devices)
    {
        if (device.serialNumber == deviceSerial && device.authStatus == AuthStatus::AUTHORIZED)
        {
            return true;
        }
    }
    return false;
}

int main()
{
    ClearScreen();
    PrintHeader();

    Logger& logger = Logger::GetInstance();
    logger.StartSession("logs");

    AdbManager& adbMgr = AdbManager::GetInstance();

    // Step 1: Check and select device
    std::cout << "[*] Checking for connected devices...\n";
    std::string targetDevice = SelectDevice(adbMgr);

    if (targetDevice.empty())
    {
        std::cout << "\n[ERROR] Failed to select device!\n";
        std::cout << "[INFO] Please:\n";
        std::cout << "  1. Connect Android device via USB\n";
        std::cout << "  2. Enable USB Debugging (Settings > Developer Options)\n";
        std::cout << "  3. Tap OK to allow debugging\n";
        std::cout << "  4. Run this application again\n\n";
        logger.EndSession();
        return 1;
    }

    // Get device info
    auto allDevices = adbMgr.GetConnectedDevices();
    std::string deviceModel = "";
    for (const auto& device : allDevices)
    {
        if (device.serialNumber == targetDevice)
        {
            deviceModel = device.model;
            break;
        }
    }

    g_currentDevice = targetDevice;
    g_currentDeviceModel = deviceModel;

    std::cout << "[✓] Device selected: " << deviceModel << "\n\n";
    logger.Info("Device connected", targetDevice);

    // Step 2: Fetch packages
    std::cout << "[*] Scanning packages...\n";
    PackageManager pkgMgr(targetDevice);
    if (!pkgMgr.FetchAllPackages())
    {
        std::cout << "[ERROR] Failed to fetch packages\n";
        logger.Error("Failed to fetch packages");
        logger.EndSession();
        return 1;
    }

    auto allPkgs = pkgMgr.GetAllPackages();
    std::cout << "[✓] Found " << allPkgs.size() << " packages\n\n";

    // Step 3: Classify packages
    std::cout << "[*] Analyzing packages...\n";
    std::string mfg = adbMgr.GetDeviceProperty(targetDevice, "ro.product.manufacturer");
    OemType detectedOem = PackageClassifier::DetectOemType(mfg, "", "");
    PackageClassifier classifier(deviceModel, detectedOem);

    auto safeToRemove = classifier.ClassifyMultiple(allPkgs, PackageCategory::SAFE_TO_REMOVE);
    std::cout << "[✓] Classification complete\n";
    std::cout << "[✓] Found " << safeToRemove.size() << " removable packages\n\n";

    logger.Info("Analysis complete", std::to_string(safeToRemove.size()) + " removable");

    // Main menu
    bool running = true;
    std::vector<std::string> selectedForRemoval;
    std::vector<PackageClassification> safeToRemoveList;
    std::vector<PackageClassification> optionalList;
    std::vector<PackageClassification> analyticsList;

    while (running)
    {
        // Check if current device is still connected
        if (!IsDeviceStillConnected(adbMgr, targetDevice))
        {
            ClearScreen();
            PrintHeader();
            std::cout << "\n[WARNING] Current device disconnected!\n";
            
            auto currentDevices = adbMgr.GetConnectedDevices();
            if (!currentDevices.empty())
            {
                std::cout << "[*] New device(s) detected. Would you like to switch? (yes/no): ";
                std::string switchChoice;
                std::getline(std::cin, switchChoice);
                
                if (switchChoice == "yes" || switchChoice == "y" || switchChoice == "Y" || switchChoice == "YES")
                {
                    targetDevice = SelectDevice(adbMgr);
                    if (!targetDevice.empty())
                    {
                        allDevices = adbMgr.GetConnectedDevices();
                        for (const auto& device : allDevices)
                        {
                            if (device.serialNumber == targetDevice)
                            {
                                deviceModel = device.model;
                                break;
                            }
                        }
                        g_currentDevice = targetDevice;
                        g_currentDeviceModel = deviceModel;
                        
                        PackageManager newPkgMgr(targetDevice);
                        if (newPkgMgr.FetchAllPackages())
                        {
                            pkgMgr = newPkgMgr;
                            allPkgs = pkgMgr.GetAllPackages();
                            selectedPackages.clear();
                            
                            std::string mfg2 = adbMgr.GetDeviceProperty(targetDevice, "ro.product.manufacturer");
                            OemType detectedOem2 = PackageClassifier::DetectOemType(mfg2, "", "");
                            classifier = PackageClassifier(deviceModel, detectedOem2);
                            
                            safeToRemoveList = classifier.ClassifyMultiple(allPkgs, PackageCategory::SAFE_TO_REMOVE);
                            optionalList = classifier.ClassifyMultiple(allPkgs, PackageCategory::OPTIONAL);
                            
                            ClearScreen();
                            PrintHeader();
                            std::cout << "[*] Switched to new device: " << deviceModel << "\n\n";
                            continue;
                        }
                    }
                    std::cout << "[ERROR] Failed to switch device. Exiting...\n";
                    running = false;
                    break;
                }
                else
                {
                    std::cout << "[INFO] Waiting for original device...\n";
                    std::cout << "Press Enter to check again...";
                    std::getline(std::cin, switchChoice);
                    ClearScreen();
                    PrintHeader();
                    continue;
                }
            }
            else
            {
                std::cout << "[INFO] Waiting for device to be connected...\n";
                std::cout << "Press Enter to check again...";
                std::string waitChoice;
                std::getline(std::cin, waitChoice);
                ClearScreen();
                PrintHeader();
                continue;
            }
        }
        
        PrintMenu();
        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "1")
        {
            ClearScreen();
            PrintHeader();

            // Fetch updated classifications
            safeToRemoveList = classifier.ClassifyMultiple(allPkgs, PackageCategory::SAFE_TO_REMOVE);
            optionalList = classifier.ClassifyMultiple(allPkgs, PackageCategory::OPTIONAL);
            analyticsList = classifier.ClassifyMultiple(allPkgs, PackageCategory::ANALYTICS);

            if (safeToRemoveList.empty() && optionalList.empty() && analyticsList.empty())
            {
                std::cout << "[INFO] No removable packages found on your device.\n";
                std::cout << "Your device is already clean!\n\n";
            }
            else
            {
                if (!safeToRemoveList.empty())
                {
                    std::cout << "BLOATWARE (Safe to Remove) [" << safeToRemoveList.size() << "]:\n";
                    std::cout << std::string(60, '-') << "\n";

                    for (size_t i = 0; i < safeToRemoveList.size(); ++i)
                    {
                        std::cout << std::setw(3) << (i + 1) << ". "
                                 << std::setw(40) << safeToRemoveList[i].packageName
                                 << " (" << safeToRemoveList[i].safetyScore << "%)\n";
                    }
                    std::cout << "\n";
                }

                if (!optionalList.empty())
                {
                    std::cout << "OPTIONAL (Consider Removing) [" << optionalList.size() << "]:\n";
                    std::cout << std::string(60, '-') << "\n";

                    for (size_t i = 0; i < optionalList.size(); ++i)
                    {
                        std::cout << std::setw(3) << (i + 1) << ". "
                                 << std::setw(40) << optionalList[i].packageName
                                 << " (" << optionalList[i].safetyScore << "%)\n";
                    }
                    std::cout << "\n";
                }

                if (!analyticsList.empty())
                {
                    std::cout << "ANALYTICS (Data Collection) [" << analyticsList.size() << "]:\n";
                    std::cout << std::string(60, '-') << "\n";

                    for (size_t i = 0; i < analyticsList.size(); ++i)
                    {
                        std::cout << std::setw(3) << (i + 1) << ". "
                                 << std::setw(40) << analyticsList[i].packageName
                                 << " (" << analyticsList[i].safetyScore << "%)\n";
                    }
                    std::cout << "\n";
                }

                std::cout << std::string(60, '-') << "\n";
                std::cout << "Total: " << (safeToRemoveList.size() + optionalList.size() + analyticsList.size()) << " removable packages\n\n";
            }

            std::cout << "Press Enter to continue...";
            std::getline(std::cin, choice);
            ClearScreen();
            PrintHeader();
        }
        else if (choice == "2")
        {
            // Advanced search by category
            bool searchActive = true;
            while (searchActive)
            {
                ClearScreen();
                PrintHeader();
                PrintCategoryMenu();
                std::string catChoice;
                std::getline(std::cin, catChoice);

                std::vector<PackageClassification> categoryPackages;
                std::string categoryName;

                if (catChoice == "1")
                {
                    categoryPackages = classifier.ClassifyMultiple(allPkgs, PackageCategory::DO_NOT_TOUCH);
                    categoryName = "Critical";
                }
                else if (catChoice == "2")
                {
                    categoryPackages = classifier.ClassifyMultiple(allPkgs, PackageCategory::SAFE_TO_REMOVE);
                    categoryName = "Bloat";
                }
                else if (catChoice == "3")
                {
                    categoryPackages = classifier.ClassifyMultiple(allPkgs, PackageCategory::ANALYTICS);
                    categoryName = "Analytics";
                }
                else if (catChoice == "4")
                {
                    categoryPackages = classifier.ClassifyMultiple(allPkgs, PackageCategory::OPTIONAL);
                    categoryName = "Optional";
                }
                else if (catChoice == "5")
                {
                    categoryPackages = classifier.ClassifyMultiple(allPkgs, PackageCategory::USER_APP);
                    categoryName = "User";
                }
                else if (catChoice == "6")
                {
                    categoryPackages = classifier.ClassifyMultiple(allPkgs, PackageCategory::UNCATEGORIZED);
                    categoryName = "Other";
                }
                else if (catChoice == "7")
                {
                    categoryPackages.clear();
                    for (const auto& pkg : allPkgs)
                    {
                        categoryPackages.push_back(classifier.Classify(pkg));
                    }
                    categoryName = "All";
                }
                else if (catChoice == "8")
                {
                    searchActive = false;
                    break;
                }
                else
                {
                    continue;
                }

                if (!categoryPackages.empty())
                {
                    ClearScreen();
                    PrintHeader();
                    std::cout << "Category: " << categoryName << " (" << categoryPackages.size() << " packages)\n\n";
                    ManagePackageSelection(categoryPackages, selectedPackages);
                }
                else
                {
                    std::cout << "[INFO] No packages found in this category.\n";
                    std::cout << "Press Enter to continue...";
                    std::getline(std::cin, choice);
                }
            }
        }
        else if (choice == "3")
        {
            safeToRemoveList = classifier.ClassifyMultiple(allPkgs, PackageCategory::SAFE_TO_REMOVE);
            optionalList = classifier.ClassifyMultiple(allPkgs, PackageCategory::OPTIONAL);
            analyticsList = classifier.ClassifyMultiple(allPkgs, PackageCategory::ANALYTICS);

            std::cout << "\n[REMOVE OPTIONS]\n";
            std::cout << std::string(60, '-') << "\n";
            std::cout << "1. Remove Selected packages\n";
            std::cout << "2. Advanced removal (any classified package)\n";
            std::cout << "3. Cancel\n";
            std::cout << "Enter choice (1-3): ";

            std::string removalMode;
            std::getline(std::cin, removalMode);

            std::vector<PackageClassification> removalCandidates;
            std::set<std::string> advancedSelection;

            if (removalMode == "1")
            {
                if (!selectedPackages.empty())
                {
                    for (const auto& selectedName : selectedPackages)
                    {
                        Package pkg = pkgMgr.GetPackageByName(selectedName);
                        if (pkg.packageName.empty())
                        {
                            pkg = Package(selectedName);
                        }
                        removalCandidates.push_back(classifier.Classify(pkg));
                    }
                }
                else
                {
                    removalCandidates = safeToRemoveList;
                    removalCandidates.insert(removalCandidates.end(), optionalList.begin(), optionalList.end());
                    removalCandidates.insert(removalCandidates.end(), analyticsList.begin(), analyticsList.end());
                }
            }
            else if (removalMode == "2")
            {
                std::vector<PackageClassification> allClassified;
                for (const auto& pkg : allPkgs)
                {
                    allClassified.push_back(classifier.Classify(pkg));
                }

                ClearScreen();
                PrintHeader();
                std::cout << "Advanced removal allows selecting any classified package, including critical ones.\n";
                std::cout << "Toggle packages with their number, then press 'd' when done.\n\n";
                ManagePackageSelection(allClassified, advancedSelection, true);

                for (const auto& pkg : allClassified)
                {
                    if (advancedSelection.count(pkg.packageName))
                    {
                        removalCandidates.push_back(pkg);
                    }
                }
            }
            else
            {
                continue;
            }

            if (removalCandidates.empty())
            {
                std::cout << "[INFO] No selectable packages available for removal.\n\n";
                std::cout << "Press Enter to continue...";
                std::getline(std::cin, choice);
                ClearScreen();
                PrintHeader();
                continue;
            }

            std::cout << "\n[REMOVE PACKAGES]\n";
            std::cout << std::string(60, '-') << "\n";
            for (size_t i = 0; i < removalCandidates.size(); ++i)
            {
                std::cout << std::setw(3) << (i + 1) << ". "
                          << std::setw(45) << removalCandidates[i].packageName
                          << " [" << PackageClassifier::GetCategoryName(removalCandidates[i].category) << "]\n";
            }
            std::cout << std::string(60, '-') << "\n";
            std::cout << "1. Remove all packages (" << removalCandidates.size() << ")\n";
            std::cout << "2. Remove specific packages by number\n";
            std::cout << "3. Cancel\n";
            std::cout << "\nEnter choice (1-3): ";

            std::string subChoice;
            std::getline(std::cin, subChoice);

            std::vector<PackageClassification> selectedForRemoval;
            if (subChoice == "1")
            {
                selectedForRemoval = removalCandidates;
            }
            else if (subChoice == "2")
            {
                std::cout << "\nEnter numbers separated by commas (for example: 1,3,5), 'all', or 'c' to cancel: ";
                std::string input;
                std::getline(std::cin, input);

                if (input == "all")
                {
                    selectedForRemoval = removalCandidates;
                }
                else if (input == "c" || input == "C" || input == "cancel" || input == "CANCEL")
                {
                    // cancel removal selection
                }
                else
                {
                    size_t pos = 0;
                    while (pos < input.length())
                    {
                        size_t comma = input.find(',', pos);
                        std::string numStr = input.substr(pos, comma != std::string::npos ? comma - pos : std::string::npos);

                        size_t start = numStr.find_first_not_of(" \t");
                        size_t end = numStr.find_last_not_of(" \t");
                        if (start != std::string::npos && end != std::string::npos)
                        {
                            numStr = numStr.substr(start, end - start + 1);
                        }
                        else
                        {
                            numStr.clear();
                        }

                        try
                        {
                            int num = std::stoi(numStr);
                            if (num > 0 && num <= static_cast<int>(removalCandidates.size()))
                            {
                                selectedForRemoval.push_back(removalCandidates[num - 1]);
                            }
                        }
                        catch (...)
                        {
                            // ignore invalid entries
                        }

                        pos = (comma != std::string::npos ? comma + 1 : input.length());
                    }
                }
            }

            if (selectedForRemoval.empty())
            {
                std::cout << "\n[INFO] No packages selected for removal.\n";
                std::cout << "Press Enter to continue...";
                std::getline(std::cin, choice);
                ClearScreen();
                PrintHeader();
                continue;
            }

            std::cout << "\n[CONFIRM] Remove the following packages?\n";
            for (const auto& pkg : selectedForRemoval)
            {
                std::cout << "  - " << pkg.packageName << "\n";
            }
            std::cout << "Type 'yes' to proceed: ";
            std::string confirm;
            std::getline(std::cin, confirm);

            if (confirm == "yes")
            {
                RemovalEngine engine(targetDevice);
                int success = 0, failed = 0;

                std::cout << "\n[PROCESSING]\n";
                for (const auto& pkg : selectedForRemoval)
                {
                    auto result = engine.RemovePackage(pkg.packageName, RemovalAction::UNINSTALL_OR_DISABLE);
                    if (result.status == RemovalStatus::SUCCESS)
                    {
                        std::cout << "  [+] " << pkg.packageName << "\n";
                        success++;
                        logger.RemovalLog(pkg.packageName, "REMOVED", true);
                        selectedPackages.erase(pkg.packageName);
                    }
                    else
                    {
                        std::cout << "  [-] " << pkg.packageName << "\n";
                        failed++;
                        logger.RemovalLog(pkg.packageName, "FAILED", false, result.message);
                    }
                }

                std::cout << "\n[RESULT] " << success << " removed, " << failed << " failed\n";
                if (success > 0)
                {
                    pkgMgr.FetchAllPackages();
                    allPkgs = pkgMgr.GetAllPackages();
                    safeToRemoveList = classifier.ClassifyMultiple(allPkgs, PackageCategory::SAFE_TO_REMOVE);
                    optionalList = classifier.ClassifyMultiple(allPkgs, PackageCategory::OPTIONAL);
                }

                std::cout << "Press Enter to continue...";
                std::getline(std::cin, confirm);
            }

            ClearScreen();
            PrintHeader();
        }
        else if (choice == "4")
        {
            ClearScreen();
            PrintHeader();

            auto protectedPkgs = classifier.ClassifyMultiple(allPkgs, PackageCategory::DO_NOT_TOUCH);
            auto optionalPkgs = classifier.ClassifyMultiple(allPkgs, PackageCategory::OPTIONAL);
            auto analyticsPkgs = classifier.ClassifyMultiple(allPkgs, PackageCategory::ANALYTICS);
            auto safePkgs = classifier.ClassifyMultiple(allPkgs, PackageCategory::SAFE_TO_REMOVE);

            std::cout << "DEVICE ANALYSIS REPORT\n";
            std::cout << std::string(60, '=') << "\n";
            std::cout << "Device: " << deviceModel << "\n";
            std::cout << "Serial: " << targetDevice << "\n";
            std::cout << "Manufacturer: " << mfg << "\n";
            std::cout << "\nPACKAGE STATISTICS\n";
            std::cout << std::string(60, '-') << "\n";
            std::cout << "Total Packages: " << allPkgs.size() << "\n";
            std::cout << "  - System: " << pkgMgr.GetSystemPackageCount() << "\n";
            std::cout << "  - User: " << pkgMgr.GetUserPackageCount() << "\n";
            std::cout << "\nCATEGORY BREAKDOWN\n";
            std::cout << std::string(60, '-') << "\n";
            std::cout << "Protected (Do Not Remove):   " << std::setw(4) << protectedPkgs.size() << " packages\n";
            std::cout << "Bloatware (Safe to Remove):  " << std::setw(4) << safePkgs.size() << " packages\n";
            std::cout << "Optional (Consider Remove):  " << std::setw(4) << optionalPkgs.size() << " packages\n";
            std::cout << "Analytics (Data Collection): " << std::setw(4) << analyticsPkgs.size() << " packages\n";
            std::cout << "Total Removable:             " << std::setw(4) << (safePkgs.size() + optionalPkgs.size() + analyticsPkgs.size()) << " packages\n";

            std::cout << "\n\nPress Enter to continue...";
            std::getline(std::cin, choice);
            ClearScreen();
            PrintHeader();
        }
        else if (choice == "5")
        {
            running = false;
        }
        else if (choice == "6")
        {
            ClearScreen();
            PrintHeader();
            std::cout << "[*] Refreshing package scan...\n";
            if (pkgMgr.FetchAllPackages())
            {
                allPkgs = pkgMgr.GetAllPackages();
                safeToRemoveList = classifier.ClassifyMultiple(allPkgs, PackageCategory::SAFE_TO_REMOVE);
                optionalList = classifier.ClassifyMultiple(allPkgs, PackageCategory::OPTIONAL);

                std::set<std::string> currentPackages;
                for (const auto& pkg : allPkgs)
                {
                    currentPackages.insert(pkg.packageName);
                }
                for (auto it = selectedPackages.begin(); it != selectedPackages.end(); )
                {
                    if (currentPackages.count(*it) == 0)
                    {
                        it = selectedPackages.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }

                std::cout << "[✓] Scan refreshed. " << allPkgs.size() << " packages loaded." << "\n";
            }
            else
            {
                std::cout << "[ERROR] Failed to refresh package scan." << "\n";
            }
            std::cout << "Press Enter to continue...";
            std::getline(std::cin, choice);
            ClearScreen();
            PrintHeader();
        }
        else if (choice == "7")
        {
            ClearScreen();
            PrintHeader();
            
            std::string newDevice = SelectDevice(adbMgr);
            if (!newDevice.empty() && newDevice != targetDevice)
            {
                targetDevice = newDevice;
                
                allDevices = adbMgr.GetConnectedDevices();
                for (const auto& device : allDevices)
                {
                    if (device.serialNumber == targetDevice)
                    {
                        deviceModel = device.model;
                        break;
                    }
                }
                g_currentDevice = targetDevice;
                g_currentDeviceModel = deviceModel;
                
                PackageManager newPkgMgr(targetDevice);
                if (newPkgMgr.FetchAllPackages())
                {
                    pkgMgr = newPkgMgr;
                    allPkgs = pkgMgr.GetAllPackages();
                    selectedPackages.clear();
                    
                    std::string mfg2 = adbMgr.GetDeviceProperty(targetDevice, "ro.product.manufacturer");
                    OemType detectedOem2 = PackageClassifier::DetectOemType(mfg2, "", "");
                    classifier = PackageClassifier(deviceModel, detectedOem2);
                    
                    safeToRemoveList = classifier.ClassifyMultiple(allPkgs, PackageCategory::SAFE_TO_REMOVE);
                    optionalList = classifier.ClassifyMultiple(allPkgs, PackageCategory::OPTIONAL);
                    
                    std::cout << "[*] Switched to device: " << deviceModel << "\n\n";
                    std::cout << "Press Enter to continue...";
                    std::getline(std::cin, choice);
                }
                else
                {
                    std::cout << "[ERROR] Failed to fetch packages for new device!\n";
                    std::cout << "Press Enter to continue...";
                    std::getline(std::cin, choice);
                }
            }
            
            ClearScreen();
            PrintHeader();
        }
        else
        {
            std::cout << "Invalid choice. Please try again.\n";
        }
    }

    logger.EndSession();
    adbMgr.KillAdbServer();
    std::cout << "\nThank you for using DeBloat!\n";
    std::cout << "Logs saved to: logs/\n\n";

    return 0;
}
