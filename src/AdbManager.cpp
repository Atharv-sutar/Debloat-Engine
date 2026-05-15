#include "AdbManager.h"
#include "CommandExecutor.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <filesystem>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

// Static instance
AdbManager* AdbManager::instance = nullptr;

AdbManager::AdbManager() : adbPath(""), lastError("")
{
}

AdbManager& AdbManager::GetInstance()
{
    if (instance == nullptr)
    {
        instance = new AdbManager();
    }
    return *instance;
}

bool AdbManager::IsAdbInstalled()
{
    if (!adbPath.empty())
    {
        return true; // Already found
    }

    adbPath = FindAdbInPath();
    if (!adbPath.empty())
    {
        return true;
    }

    adbPath = FindAdbInCommonLocations();
    return !adbPath.empty();
}

std::string AdbManager::FindAdbInPath()
{
    std::string output;

    // Try to find adb in PATH using where.exe
    bool found = CommandExecutor::Execute("where.exe adb.exe", output);
    if (found && !output.empty())
    {
        auto lines = CommandExecutor::SplitLines(output);
        for (const auto& line : lines)
        {
            if (line.rfind("INFO:", 0) == 0)
            {
                continue;
            }

            if (!line.empty())
            {
                return line;
            }
        }
    }

    // If where.exe fails, scan each PATH entry explicitly.
    const char* pathEnv = std::getenv("PATH");
    if (pathEnv != nullptr)
    {
        std::string pathValue(pathEnv);
        size_t start = 0;
        while (start < pathValue.size())
        {
            size_t end = pathValue.find(';', start);
            if (end == std::string::npos)
                end = pathValue.size();

            std::string dir = pathValue.substr(start, end - start);
            if (!dir.empty())
            {
                std::string candidate = dir;
                if (candidate.back() != '\\' && candidate.back() != '/')
                {
                    candidate += "\\";
                }
                candidate += "adb.exe";

                try
                {
                    if (fs::exists(candidate))
                    {
                        return candidate;
                    }
                }
                catch (const std::exception&)
                {
                    // ignore invalid path entries
                }
            }

            start = end + 1;
        }
    }

    return "";
}

std::string AdbManager::FindAdbInCommonLocations()
{
    // Common ADB locations on Windows
    const std::vector<std::string> commonPaths = {
        "C:\\Android\\platform-tools\\adb.exe",
        "C:\\platform-tools-latest-windows\\platform-tools\\adb.exe",
        "C:\\platform-tools\\adb.exe",
        "C:\\Program Files\\Android\\platform-tools\\adb.exe",
        "C:\\Program Files (x86)\\Android\\android-sdk\\platform-tools\\adb.exe",
        "%LOCALAPPDATA%\\Android\\Sdk\\platform-tools\\adb.exe",
        "%PROGRAMFILES%\\Android SDK\\platform-tools\\adb.exe",
        "%USERPROFILE%\\platform-tools\\adb.exe"
    };

    for (const auto& path : commonPaths)
    {
        // Expand environment variables
        std::string expandedPath = path;
        
        // Simple environment variable expansion
        if (expandedPath.find("%LOCALAPPDATA%") != std::string::npos)
        {
            const char* localAppData = std::getenv("LOCALAPPDATA");
            if (localAppData)
            {
                size_t pos = expandedPath.find("%LOCALAPPDATA%");
                expandedPath.replace(pos, 14, localAppData);
            }
        }

        if (expandedPath.find("%PROGRAMFILES%") != std::string::npos)
        {
            const char* programFiles = std::getenv("PROGRAMFILES");
            if (programFiles)
            {
                size_t pos = expandedPath.find("%PROGRAMFILES%");
                expandedPath.replace(pos, 14, programFiles);
            }
        }

        if (expandedPath.find("%USERPROFILE%") != std::string::npos)
        {
            const char* userProfile = std::getenv("USERPROFILE");
            if (userProfile)
            {
                size_t pos = expandedPath.find("%USERPROFILE%");
                expandedPath.replace(pos, 13, userProfile);
            }
        }

        try
        {
            if (fs::exists(expandedPath))
            {
                return expandedPath;
            }
        }
        catch (const std::exception& e)
        {
            // Continue to next path
        }
    }

    return "";
}

std::string AdbManager::GetAdbPath()
{
    if (adbPath.empty())
    {
        IsAdbInstalled();
    }
    return adbPath;
}

bool AdbManager::StartAdbServer()
{
    if (!IsAdbInstalled())
    {
        lastError = "ADB is not installed or not found in PATH";
        return false;
    }

    if (IsAdbServerRunning())
    {
        return true; // Already running
    }

    std::string command = adbPath + " start-server";
    std::string output;

    bool success = CommandExecutor::Execute(command, output);
    if (!success)
    {
        lastError = "Failed to start ADB server";
        return false;
    }

    // Wait a moment for server to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return true;
}

bool AdbManager::KillAdbServer()
{
    if (!IsAdbInstalled())
    {
        lastError = "ADB is not installed";
        return false;
    }

    std::string command = adbPath + " kill-server";
    std::string output;

    bool success = CommandExecutor::Execute(command, output);
    if (!success)
    {
        lastError = "Failed to kill ADB server";
        return false;
    }

    return true;
}

bool AdbManager::IsAdbServerRunning()
{
    if (!IsAdbInstalled())
    {
        return false;
    }

    std::string command = adbPath + " devices";
    std::string output;

    bool success = CommandExecutor::Execute(command, output);
    return success; // If command succeeds, server is running
}

std::vector<AndroidDevice> AdbManager::GetConnectedDevices()
{
    std::vector<AndroidDevice> devices;

    if (!StartAdbServer())
    {
        lastError = "Cannot start ADB server";
        return devices;
    }

    std::string command = adbPath + " devices -l";
    std::string output;

    if (!CommandExecutor::Execute(command, output))
    {
        lastError = "Failed to get device list";
        return devices;
    }

    auto lines = CommandExecutor::SplitLines(output);

    // Parse output (skip header "List of attached devices")
    for (size_t i = 1; i < lines.size(); ++i)
    {
        const auto& line = lines[i];
        if (line.empty())
            continue;

        // Format: <serial> <status> [device info]
        // Example: emulator-5554             device usb:1-1 product:sdk_google_phone_x86_64 model:AOSP_on_x86_64 device:generic_x86_64
        // Example: FA8K20005345              device usb:1-1 product:blueline model:Pixel3 device:blueline

        AndroidDevice device;
        std::istringstream iss(line);
        
        // Get serial number
        iss >> device.serialNumber;
        
        // Get auth status
        std::string statusStr;
        iss >> statusStr;
        device.authStatus = ParseAuthStatus(statusStr);

        // Skip USB info if present
        std::string token;
        while (iss >> token)
        {
            if (token.find("product:") == 0)
            {
                device.product = token.substr(8);
            }
            else if (token.find("model:") == 0)
            {
                device.model = token.substr(6);
            }
            else if (token.find("device:") == 0)
            {
                device.device = token.substr(7);
            }
        }

        if (!device.serialNumber.empty())
        {
            devices.push_back(device);
        }
    }

    return devices;
}

AuthStatus AdbManager::GetDeviceAuthStatus(const std::string& serialNumber)
{
    std::string command = adbPath + " -s " + serialNumber + " get-state";
    std::string output;

    if (!CommandExecutor::Execute(command, output))
    {
        return AuthStatus::UNKNOWN;
    }

    output.erase(output.find_last_not_of(" \n\r\t") + 1);
    return ParseAuthStatus(output);
}

AuthStatus AdbManager::ParseAuthStatus(const std::string& statusString)
{
    std::string status = statusString;
    
    // Convert to lowercase for comparison
    std::transform(status.begin(), status.end(), status.begin(), ::tolower);

    if (status == "device")
    {
        return AuthStatus::AUTHORIZED;
    }
    else if (status == "unauthorized")
    {
        return AuthStatus::UNAUTHORIZED;
    }
    else if (status == "offline")
    {
        return AuthStatus::OFFLINE;
    }

    return AuthStatus::UNKNOWN;
}

std::string AdbManager::GetDeviceProperty(const std::string& serialNumber, 
                                          const std::string& property)
{
    std::string command = adbPath + " -s " + serialNumber + " shell getprop " + property;
    std::string output;

    if (!CommandExecutor::Execute(command, output))
    {
        lastError = "Failed to get device property: " + property;
        return "";
    }

    // Remove trailing whitespace and newlines
    output.erase(output.find_last_not_of(" \n\r\t") + 1);
    return output;
}

std::string AdbManager::GetLastError()
{
    return lastError;
}
