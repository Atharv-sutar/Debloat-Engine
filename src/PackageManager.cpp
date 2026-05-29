#include "PackageManager.h"
#include "AdbManager.h"
#include "CommandExecutor.h"
#include <algorithm>
#include <sstream>
#include <cctype>
#include <unordered_set>

PackageManager::PackageManager(const std::string& deviceSerial)
    : deviceSerial(deviceSerial), lastError("")
{
}

bool PackageManager::FetchPackages()
{
    // Fetch both system and user packages
    return FetchSystemPackages() && FetchUserPackages();
}

bool PackageManager::FetchAllPackages()
{
    allPackages.clear();
    packageMap.clear();

    if (!FetchSystemPackages())
    {
        return false;
    }

    if (!FetchUserPackages())
    {
        return false;
    }

    std::unordered_set<std::string> disabledPackages = GetDisabledPackageSet();

    // Deduplicate any packages that may have been returned by both queries.
    std::unordered_set<std::string> seen;
    std::vector<Package> uniquePackages;
    uniquePackages.reserve(allPackages.size());

    for (auto& pkg : allPackages)
    {
        pkg.isEnabled = (disabledPackages.count(pkg.packageName) == 0);
        if (seen.insert(pkg.packageName).second)
        {
            uniquePackages.push_back(pkg);
        }
    }

    allPackages.swap(uniquePackages);

    // Rebuild lookup map
    packageMap.clear();
    for (const auto& pkg : allPackages)
    {
        packageMap[pkg.packageName] = pkg;
    }

    lastError.clear();
    return true;
}

bool PackageManager::FetchSystemPackages()
{
    AdbManager& adb = AdbManager::GetInstance();

    if (!adb.IsAdbInstalled())
    {
        lastError = "ADB is not installed";
        return false;
    }

    // Command: list system packages (-s flag)
    std::string command = adb.GetAdbPath() + " -s " + deviceSerial + " shell pm list packages -s";
    std::string output;

    if (!CommandExecutor::Execute(command, output))
    {
        lastError = "Failed to fetch system packages";
        return false;
    }

    auto packages = ParsePackageList(output, PackageType::SYSTEM);
    allPackages.insert(allPackages.end(), packages.begin(), packages.end());

    // Update map
    for (const auto& pkg : packages)
    {
        packageMap[pkg.packageName] = pkg;
    }

    lastError = "";
    return true;
}

bool PackageManager::FetchUserPackages()
{
    AdbManager& adb = AdbManager::GetInstance();

    if (!adb.IsAdbInstalled())
    {
        lastError = "ADB is not installed";
        return false;
    }

    // Command: list user packages (-3 flag)
    std::string command = adb.GetAdbPath() + " -s " + deviceSerial + " shell pm list packages -3";
    std::string output;

    if (!CommandExecutor::Execute(command, output))
    {
        lastError = "Failed to fetch user packages";
        return false;
    }

    auto packages = ParsePackageList(output, PackageType::USER);
    allPackages.insert(allPackages.end(), packages.begin(), packages.end());

    // Update map
    for (const auto& pkg : packages)
    {
        packageMap[pkg.packageName] = pkg;
    }

    lastError = "";
    return true;
}

std::vector<Package> PackageManager::ParsePackageList(const std::string& output, PackageType type)
{
    std::vector<Package> packages;
    auto lines = CommandExecutor::SplitLines(output);

    for (const auto& line : lines)
    {
        if (line.empty())
            continue;

        // Format: "package:com.package.name" or just "com.package.name"
        std::string packageName = line;

        // Remove "package:" prefix if present
        if (packageName.find("package:") == 0)
        {
            packageName = packageName.substr(8);
        }

        // Skip if already added (shouldn't happen but defensive)
        auto it = std::find_if(packages.begin(), packages.end(),
                               [&packageName](const Package& p)
                               { return p.packageName == packageName; });
        if (it != packages.end())
            continue;

        // Create package with correct type
        Package pkg(packageName, type);
        packages.push_back(pkg);
    }

    return packages;
}

std::unordered_set<std::string> PackageManager::GetDisabledPackageSet() const
{
    std::unordered_set<std::string> result;
    AdbManager& adb = AdbManager::GetInstance();
    std::string command = adb.GetAdbPath() + " -s " + deviceSerial + " shell pm list packages -d";
    std::string output;

    if (!CommandExecutor::Execute(command, output))
    {
        return result;
    }

    auto lines = CommandExecutor::SplitLines(output);
    for (auto line : lines)
    {
        if (line.find("package:") == 0)
        {
            line = line.substr(8);
        }

        if (!line.empty())
        {
            result.insert(line);
        }
    }

    return result;
}

std::vector<Package> PackageManager::GetSystemPackages() const
{
    std::vector<Package> result;
    for (const auto& pkg : allPackages)
    {
        if (pkg.type == PackageType::SYSTEM || pkg.type == PackageType::SYSTEM_UPDATE)
        {
            result.push_back(pkg);
        }
    }
    return result;
}

std::vector<Package> PackageManager::GetUserPackages() const
{
    std::vector<Package> result;
    for (const auto& pkg : allPackages)
    {
        if (pkg.type == PackageType::USER)
        {
            result.push_back(pkg);
        }
    }
    return result;
}

size_t PackageManager::GetSystemPackageCount() const
{
    return GetSystemPackages().size();
}

size_t PackageManager::GetUserPackageCount() const
{
    return GetUserPackages().size();
}

std::vector<Package> PackageManager::GetCriticalPackages() const
{
    std::vector<Package> result;
    for (const auto& pkg : allPackages)
    {
        if (pkg.IsCriticalSystem())
        {
            result.push_back(pkg);
        }
    }
    return result;
}

std::vector<Package> PackageManager::GetBloatwarePackages() const
{
    std::vector<Package> result;
    for (const auto& pkg : allPackages)
    {
        if (!pkg.IsCriticalSystem() && pkg.IsKnownBloatware())
        {
            result.push_back(pkg);
        }
    }
    return result;
}

std::vector<Package> PackageManager::GetAnalyticsPackages() const
{
    std::vector<Package> result;
    for (const auto& pkg : allPackages)
    {
        if (!pkg.IsCriticalSystem() && pkg.IsAnalytics())
        {
            result.push_back(pkg);
        }
    }
    return result;
}

std::vector<Package> PackageManager::GetDisabledPackages() const
{
    std::vector<Package> result;
    for (const auto& pkg : allPackages)
    {
        if (!pkg.isEnabled)
        {
            result.push_back(pkg);
        }
    }
    return result;
}

std::vector<Package> PackageManager::GetEnabledPackages() const
{
    std::vector<Package> result;
    for (const auto& pkg : allPackages)
    {
        if (pkg.isEnabled)
        {
            result.push_back(pkg);
        }
    }
    return result;
}

size_t PackageManager::GetDisabledPackageCount() const
{
    return GetDisabledPackages().size();
}

size_t PackageManager::GetEnabledPackageCount() const
{
    return GetEnabledPackages().size();
}

std::vector<Package> PackageManager::SearchPackages(const std::string& query) const
{
    std::vector<Package> result;

    // Split query into tokens
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(query);
    while (ss >> token)
    {
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        tokens.push_back(token);
    }

    if (tokens.empty()) return allPackages;

    for (const auto& pkg : allPackages)
    {
        std::string lowerName = pkg.packageName;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

        std::string lowerDisplay = pkg.displayName;
        std::transform(lowerDisplay.begin(), lowerDisplay.end(), lowerDisplay.begin(), ::tolower);

        bool allMatch = true;
        for (const auto& t : tokens)
        {
            if (lowerName.find(t) == std::string::npos && 
                lowerDisplay.find(t) == std::string::npos)
            {
                allMatch = false;
                break;
            }
        }

        if (allMatch)
        {
            result.push_back(pkg);
        }
    }

    return result;
}

Package PackageManager::GetPackageByName(const std::string& packageName) const
{
    auto it = packageMap.find(packageName);
    if (it != packageMap.end())
    {
        return it->second;
    }

    // Return empty package if not found
    return Package();
}

Package PackageManager::GetPackageDetails(const std::string& packageName)
{
    AdbManager& adb = AdbManager::GetInstance();

    if (!adb.IsAdbInstalled())
    {
        lastError = "ADB is not installed";
        return Package();
    }

    // Try to get from cache first
    auto cached = GetPackageByName(packageName);
    if (!cached.packageName.empty())
    {
        // Get additional details via dumpsys
        std::string command = adb.GetAdbPath() + " -s " + deviceSerial + 
                            " shell dumpsys package " + packageName;
        std::string output;

        if (CommandExecutor::Execute(command, output))
        {
            cached.versionCode = ExtractVersionCode(output);
            cached.versionName = ExtractVersionName(output);
        }

        return cached;
    }

    return Package();
}

bool PackageManager::IsPackageInstalled(const std::string& packageName) const
{
    return packageMap.find(packageName) != packageMap.end();
}

std::string PackageManager::ExtractVersionCode(const std::string& dumpsysOutput)
{
    // Look for "versionCode=" in dumpsys output
    size_t pos = dumpsysOutput.find("versionCode=");
    if (pos != std::string::npos)
    {
        size_t start = pos + 12; // Length of "versionCode="
        size_t end = dumpsysOutput.find(' ', start);
        if (end == std::string::npos)
            end = dumpsysOutput.find('\n', start);

        if (end != std::string::npos)
        {
            return dumpsysOutput.substr(start, end - start);
        }
    }
    return "";
}

std::string PackageManager::ExtractVersionName(const std::string& dumpsysOutput)
{
    // Look for "versionName=" in dumpsys output
    size_t pos = dumpsysOutput.find("versionName=");
    if (pos != std::string::npos)
    {
        size_t start = pos + 12; // Length of "versionName="
        size_t end = dumpsysOutput.find(' ', start);
        if (end == std::string::npos)
            end = dumpsysOutput.find('\n', start);

        if (end != std::string::npos)
        {
            return dumpsysOutput.substr(start, end - start);
        }
    }
    return "";
}
