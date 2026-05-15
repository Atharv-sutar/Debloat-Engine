#include "RemovalEngine.h"
#include "CommandExecutor.h"
#include "AdbManager.h"
#include <sstream>
#include <algorithm>

RemovalEngine::RemovalEngine(const std::string& deviceSerial)
    : deviceSerial(deviceSerial), lastStatus(RemovalStatus::UNKNOWN), lastError("")
{
}

RemovalResult RemovalEngine::RemovePackage(const std::string& packageName, RemovalAction action)
{
    if (!IsPackageInstalled(packageName))
    {
        RemovalResult result(packageName, action, RemovalStatus::NOT_INSTALLED,
                           "Package not found on device");
        LogResult(result);
        return result;
    }

    RemovalResult result(packageName, action, RemovalStatus::FAILED, "");

    // Try uninstall first if requested
    if (action == RemovalAction::UNINSTALL || action == RemovalAction::UNINSTALL_OR_DISABLE)
    {
        if (ExecuteUninstall(packageName))
        {
            result.status = RemovalStatus::SUCCESS;
            result.message = "Package uninstalled successfully";
            result.action = RemovalAction::UNINSTALL;
            lastStatus = RemovalStatus::SUCCESS;
            LogResult(result);
            return result;
        }
    }

    // Try disable if uninstall failed or if disable was requested
    if (action == RemovalAction::DISABLE || action == RemovalAction::UNINSTALL_OR_DISABLE)
    {
        if (ExecuteDisable(packageName))
        {
            result.status = RemovalStatus::SUCCESS;
            result.message = "Package disabled successfully (uninstall not available)";
            result.action = RemovalAction::DISABLE;
            result.canRollback = true;
            disabledPackages.insert(packageName);
            lastStatus = RemovalStatus::SUCCESS;
            LogResult(result);
            return result;
        }
    }

    // Both failed
    result.status = RemovalStatus::FAILED;
    result.message = "Failed to uninstall or disable package";
    result.canRollback = false;
    lastStatus = RemovalStatus::FAILED;
    lastError = "ADB command failed for package: " + packageName;
    LogResult(result);
    return result;
}

std::vector<RemovalResult> RemovalEngine::RemoveMultiple(const std::vector<std::string>& packageNames,
                                                         RemovalAction action,
                                                         bool stopOnError)
{
    std::vector<RemovalResult> results;
    int successCount = 0;
    int failureCount = 0;

    for (const auto& packageName : packageNames)
    {
        auto result = RemovePackage(packageName, action);
        results.push_back(result);

        if (result.status == RemovalStatus::SUCCESS)
        {
            successCount++;
        }
        else
        {
            failureCount++;
            if (stopOnError)
            {
                break;
            }
        }
    }

    // Set overall status
    if (failureCount == 0)
    {
        lastStatus = RemovalStatus::SUCCESS;
    }
    else if (successCount == 0)
    {
        lastStatus = RemovalStatus::FAILED;
    }
    else
    {
        lastStatus = RemovalStatus::PARTIAL;
    }

    return results;
}

RemovalResult RemovalEngine::DisablePackage(const std::string& packageName)
{
    if (!IsPackageInstalled(packageName))
    {
        return RemovalResult(packageName, RemovalAction::DISABLE, RemovalStatus::NOT_INSTALLED,
                           "Package not found on device");
    }

    if (ExecuteDisable(packageName))
    {
        disabledPackages.insert(packageName);
        RemovalResult result(packageName, RemovalAction::DISABLE, RemovalStatus::SUCCESS,
                           "Package disabled successfully");
        result.canRollback = true;
        lastStatus = RemovalStatus::SUCCESS;
        LogResult(result);
        return result;
    }
    else
    {
        RemovalResult result(packageName, RemovalAction::DISABLE, RemovalStatus::FAILED,
                           "Failed to disable package");
        lastStatus = RemovalStatus::FAILED;
        lastError = "ADB disable command failed";
        LogResult(result);
        return result;
    }
}

RemovalResult RemovalEngine::UninstallPackage(const std::string& packageName)
{
    if (!IsPackageInstalled(packageName))
    {
        return RemovalResult(packageName, RemovalAction::UNINSTALL, RemovalStatus::NOT_INSTALLED,
                           "Package not found on device");
    }

    if (ExecuteUninstall(packageName))
    {
        RemovalResult result(packageName, RemovalAction::UNINSTALL, RemovalStatus::SUCCESS,
                           "Package uninstalled successfully");
        lastStatus = RemovalStatus::SUCCESS;
        LogResult(result);
        return result;
    }
    else
    {
        RemovalResult result(packageName, RemovalAction::UNINSTALL, RemovalStatus::FAILED,
                           "Failed to uninstall package");
        lastStatus = RemovalStatus::FAILED;
        lastError = "ADB uninstall command failed";
        LogResult(result);
        return result;
    }
}

bool RemovalEngine::IsPackageInstalled(const std::string& packageName)
{
    std::string adbPath = AdbManager::GetInstance().GetAdbPath();
    std::string cmd = adbPath + " -s " + deviceSerial + " shell pm list packages";
    std::string output;
    CommandExecutor::Execute(cmd, output);
    
    // Check if package is in the list
    std::stringstream ss(output);
    std::string line;
    while (std::getline(ss, line))
    {
        // Package lines start with "package:"
        if (line.find("package:" + packageName) != std::string::npos)
        {
            return true;
        }
    }
    
    return false;
}

bool RemovalEngine::IsPackageEnabled(const std::string& packageName)
{
    std::string adbPath = AdbManager::GetInstance().GetAdbPath();
    std::string cmd = adbPath + " -s " + deviceSerial + " shell pm dump " + packageName + 
                      " | findstr mEnabled";
    std::string output;
    CommandExecutor::Execute(cmd, output);
    
    // Check if mEnabled=true in output
    if (output.find("mEnabled=true") != std::string::npos)
    {
        return true;
    }
    
    return false;
}

RemovalResult RemovalEngine::EnablePackage(const std::string& packageName)
{
    if (!IsPackageInstalled(packageName))
    {
        return RemovalResult(packageName, RemovalAction::DISABLE, RemovalStatus::NOT_INSTALLED,
                           "Package not found on device");
    }

    if (ExecuteEnable(packageName))
    {
        disabledPackages.erase(packageName);
        RemovalResult result(packageName, RemovalAction::DISABLE, RemovalStatus::SUCCESS,
                           "Package enabled successfully");
        lastStatus = RemovalStatus::SUCCESS;
        LogResult(result);
        return result;
    }
    else
    {
        RemovalResult result(packageName, RemovalAction::DISABLE, RemovalStatus::FAILED,
                           "Failed to enable package");
        lastStatus = RemovalStatus::FAILED;
        lastError = "ADB enable command failed";
        LogResult(result);
        return result;
    }
}

std::vector<std::string> RemovalEngine::GetDisabledPackages() const
{
    std::vector<std::string> result(disabledPackages.begin(), disabledPackages.end());
    return result;
}

size_t RemovalEngine::RollbackAllDisabled()
{
    size_t enabledCount = 0;
    std::vector<std::string> toEnable(disabledPackages.begin(), disabledPackages.end());

    for (const auto& packageName : toEnable)
    {
        if (ExecuteEnable(packageName))
        {
            enabledCount++;
            disabledPackages.erase(packageName);
        }
    }

    return enabledCount;
}

bool RemovalEngine::ExecuteUninstall(const std::string& packageName)
{
    // Try user uninstall first (works for user-installed packages)
    std::string adbPath = AdbManager::GetInstance().GetAdbPath();
    std::string cmd = adbPath + " -s " + deviceSerial + " shell pm uninstall --user 0 " + packageName;
    std::string output;
    CommandExecutor::Execute(cmd, output);

    if (ParseAdbOutput(output, RemovalAction::UNINSTALL))
    {
        return true;
    }

    // If user uninstall fails, it might be a system package - that's expected
    return false;
}

bool RemovalEngine::ExecuteDisable(const std::string& packageName)
{
    std::string adbPath = AdbManager::GetInstance().GetAdbPath();
    std::string cmd = adbPath + " -s " + deviceSerial + " shell pm disable-user --user 0 " + packageName;
    std::string output;
    CommandExecutor::Execute(cmd, output);

    return ParseAdbOutput(output, RemovalAction::DISABLE);
}

bool RemovalEngine::ExecuteEnable(const std::string& packageName)
{
    std::string adbPath = AdbManager::GetInstance().GetAdbPath();
    std::string cmd = adbPath + " -s " + deviceSerial + " shell pm enable " + packageName;
    std::string output;
    CommandExecutor::Execute(cmd, output);

    return ParseAdbOutput(output, RemovalAction::DISABLE);
}

bool RemovalEngine::ParseAdbOutput(const std::string& output, RemovalAction action)
{
    // ADB success indicators
    // For uninstall: "Success" or contains no "Error"
    // For disable: "new state: disabled"
    // For enable: "new state: enabled"

    std::string lowerOutput = output;
    std::transform(lowerOutput.begin(), lowerOutput.end(), lowerOutput.begin(), ::tolower);

    // Check for success indicators
    if (lowerOutput.find("success") != std::string::npos)
    {
        return true;
    }

    if (action == RemovalAction::DISABLE)
    {
        if (lowerOutput.find("new state: disabled") != std::string::npos ||
            lowerOutput.find("disabled") != std::string::npos)
        {
            return true;
        }
    }

    if (lowerOutput.find("new state: enabled") != std::string::npos ||
        lowerOutput.find("enabled") != std::string::npos)
    {
        return true;
    }

    // Check for error indicators
    if (lowerOutput.find("error") != std::string::npos ||
        lowerOutput.find("failed") != std::string::npos ||
        lowerOutput.find("not found") != std::string::npos)
    {
        return false;
    }

    // If output is empty or contains typical success strings
    if (output.empty() || lowerOutput.find("shell") != std::string::npos)
    {
        // Empty response often means success for some ADB commands
        return false;  // Be conservative
    }

    return false;
}

void RemovalEngine::LogResult(const RemovalResult& result)
{
    removalHistory.push_back(result);
}
