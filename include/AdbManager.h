#pragma once

#include <string>
#include <vector>
#include <map>

/**
 * Enum for device authorization status
 */
enum class AuthStatus
{
    UNAUTHORIZED,      // Device is unauthorized (needs approval)
    AUTHORIZED,        // Device is authorized
    OFFLINE,           // Device is offline
    UNKNOWN            // Status could not be determined
};

/**
 * Struct to represent a connected Android device
 */
struct AndroidDevice
{
    std::string serialNumber;
    AuthStatus authStatus;
    std::string model;
    std::string product;
    std::string device;

    AndroidDevice() : serialNumber(""), authStatus(AuthStatus::UNKNOWN), 
                      model(""), product(""), device("") {}
};

/**
 * AdbManager - Manages ADB (Android Debug Bridge) interactions
 * Handles:
 * - ADB installation detection
 * - ADB server lifecycle (start/stop)
 * - Device detection and listing
 * - Authorization status
 * - Device property queries
 */
class AdbManager
{
public:
    /**
     * Get singleton instance of AdbManager
     */
    static AdbManager& GetInstance();

    /**
     * Check if ADB is installed on the system
     * @return true if adb.exe is in PATH or at common locations
     */
    bool IsAdbInstalled();

    /**
     * Get the path to ADB executable
     * @return Path to adb.exe, empty string if not found
     */
    std::string GetAdbPath();

    /**
     * Start ADB server if not already running
     * @return true if server started successfully or was already running
     */
    bool StartAdbServer();

    /**
     * Kill ADB server
     * @return true if server killed successfully
     */
    bool KillAdbServer();

    /**
     * Get list of connected Android devices
     * @return Vector of AndroidDevice structures
     */
    std::vector<AndroidDevice> GetConnectedDevices();

    /**
     * Get authorization status of a specific device
     * @param serialNumber Device serial number
     * @return AuthStatus enum value
     */
    AuthStatus GetDeviceAuthStatus(const std::string& serialNumber);

    /**
     * Get device property using getprop command
     * @param serialNumber Device serial number
     * @param property Property name (e.g., "ro.product.model")
     * @return Property value as string
     */
    std::string GetDeviceProperty(const std::string& serialNumber, 
                                   const std::string& property);

    /**
     * Check if ADB server is running
     * @return true if server is responding to commands
     */
    bool IsAdbServerRunning();

    /**
     * Get last error message
     * @return Error message from last operation
     */
    std::string GetLastError();

private:
    AdbManager();
    ~AdbManager() = default;

    // Prevent copying
    AdbManager(const AdbManager&) = delete;
    AdbManager& operator=(const AdbManager&) = delete;

    // Helper methods
    std::string FindAdbInPath();
    std::string FindAdbInCommonLocations();
    AuthStatus ParseAuthStatus(const std::string& statusString);

    std::string adbPath;
    std::string lastError;
    static AdbManager* instance;
};
