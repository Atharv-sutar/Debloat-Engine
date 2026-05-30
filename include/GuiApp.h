#pragma once

#include <windows.h>
#ifdef OPTIONAL
#undef OPTIONAL
#endif
#include <string>
#include <vector>
#include <set>
#include "Package.h"
#include "PackageClassifier.h"
#include "PackageManager.h"
#include "RemovalEngine.h"
#include "AdbManager.h"

class GuiApp
{
public:
    GuiApp();
    ~GuiApp();

    int Run();

private:
    bool Initialize();
    void Shutdown();
    void RenderFrame();
    void RefreshDeviceList();
    void StartScan();
    void ClassifyPackages();
    void PerformRemoval(RemovalAction action);
    void RestoreDisabledPackages();
    void EnableSelectedPackages();
    void PushLog(const std::string& message);
    const std::string& GetSelectedDeviceSerial() const;
    std::string GetAuthStatusName(AuthStatus status) const;
    std::string GetCategoryName(PackageCategory category) const;

    bool initialized = false;
    bool running = false;
    bool hasDevice = false;
    bool hasScanResults = false;
    bool showRemovalResults = false;
    bool showRestoreResults = false;

    std::string selectedDeviceSerial;
    std::string selectedDeviceModel;
    std::vector<AndroidDevice> connectedDevices;
    std::vector<Package> allPackages;
    std::vector<PackageClassification> classifiedPackages;
    std::unordered_set<std::string> selectedPackageNames;
    char packageSearchBuffer[256] = "";
    std::vector<RemovalResult> lastRemovalResults;
    std::vector<std::string> activityLog;

    PackageManager* pkgMgr = nullptr;
    PackageClassifier* pkgClassifier = nullptr;
    RemovalEngine* removalEngine = nullptr;
    AdbManager& adbMgr = AdbManager::GetInstance();

    HWND hwnd = nullptr;
    HDC hdc = nullptr;
    HGLRC glContext = nullptr;
    int selectedDeviceIndex = 0;
    int selectedCategoryIndex = 0;

    PackageCategory activeCategory = PackageCategory::UNCATEGORIZED;
    bool showOnlyDisabled = false;
};