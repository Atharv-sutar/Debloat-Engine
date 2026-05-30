#include "GuiApp.h"
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_opengl2.h"
#include <gl/GL.h>
#include <algorithm>
#include <cctype>
#include <iostream>

static const char *kWindowTitle = "DeBloat GUI";
static const int kWindowWidth = 1280;
static const int kWindowHeight = 800;
static const char *kWindowClassName = "DeBloatGuiWindowClass";

GuiApp::GuiApp()
    : pkgMgr(nullptr), pkgClassifier(nullptr), removalEngine(nullptr), hwnd(nullptr), hdc(nullptr), glContext(nullptr), selectedDeviceIndex(0), selectedCategoryIndex(0)
{
}

GuiApp::~GuiApp()
{
    delete pkgMgr;
    delete pkgClassifier;
    delete removalEngine;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            RECT rect;
            GetClientRect(hWnd, &rect);
            glViewport(0, 0, rect.right - rect.left, rect.bottom - rect.top);
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int GuiApp::Run()
{
    if (!Initialize())
    {
        return 1;
    }

    running = true;
    MSG msg;

    while (running)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                running = false;
            }
        }

        RenderFrame();
    }

    Shutdown();
    return 0;
}

bool GuiApp::Initialize()
{
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = kWindowClassName;

    if (!RegisterClassEx(&wc))
    {
        std::cerr << "Failed to register window class." << std::endl;
        return false;
    }

    HMONITOR monitor = MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monitorInfo = {sizeof(monitorInfo)};
    if (!GetMonitorInfo(monitor, &monitorInfo))
    {
        monitorInfo.rcWork.left = 0;
        monitorInfo.rcWork.top = 0;
        monitorInfo.rcWork.right = kWindowWidth;
        monitorInfo.rcWork.bottom = kWindowHeight;
    }

    int screenWidth = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
    int screenHeight = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;
    int windowWidth = std::min(kWindowWidth, screenWidth - 40);
    int windowHeight = std::min(kWindowHeight, screenHeight - 40);
    int windowX = monitorInfo.rcWork.left + (screenWidth - windowWidth) / 2;
    int windowY = monitorInfo.rcWork.top + (screenHeight - windowHeight) / 2;

    RECT windowRect = {0, 0, windowWidth, windowHeight};
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    hwnd = CreateWindowEx(
        0,
        kWindowClassName,
        kWindowTitle,
        WS_OVERLAPPEDWINDOW,
        windowX,
        windowY,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr);

    if (hwnd == nullptr)
    {
        std::cerr << "Failed to create window." << std::endl;
        return false;
    }

    hdc = GetDC(hwnd);
    if (hdc == nullptr)
    {
        std::cerr << "Failed to get device context." << std::endl;
        return false;
    }

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (pixelFormat == 0 || !SetPixelFormat(hdc, pixelFormat, &pfd))
    {
        std::cerr << "Failed to set pixel format." << std::endl;
        return false;
    }

    glContext = wglCreateContext(hdc);
    if (glContext == nullptr)
    {
        std::cerr << "Failed to create OpenGL context." << std::endl;
        return false;
    }

    if (!wglMakeCurrent(hdc, glContext))
    {
        std::cerr << "Failed to make OpenGL context current." << std::endl;
        return false;
    }

    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO &io = ImGui::GetIO();
    ImFont *font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 17.0f);
    if (font == nullptr)
    {
        io.Fonts->AddFontDefault();
    }

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplOpenGL2_Init();

    initialized = true;
    RefreshDeviceList();

    return true;
}

void GuiApp::Shutdown()
{
    if (!initialized)
    {
        return;
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    if (glContext)
    {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(glContext);
        glContext = nullptr;
    }
    if (hwnd && hdc)
    {
        ReleaseDC(hwnd, hdc);
        hdc = nullptr;
    }
    if (hwnd)
    {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

void GuiApp::RenderFrame()
{
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(clientRect.right), static_cast<float>(clientRect.bottom)), ImGuiCond_Once);
    ImGui::Begin("DeBloat Dashboard", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
    ImGui::Text("Device Management");
    ImGui::Separator();

    if (ImGui::Button("Refresh Device List"))
    {
        RefreshDeviceList();
    }

    ImGui::SameLine();
    if (ImGui::Button("Scan Device"))
    {
        StartScan();
    }

    ImGui::Spacing();

    if (connectedDevices.empty())
    {
        ImGui::TextWrapped("No Android devices detected. Connect a device and press Refresh.");
    }
    else
    {
        std::vector<const char *> deviceLabels;
        for (auto &device : connectedDevices)
        {
            deviceLabels.push_back(device.serialNumber.c_str());
        }

        ImGui::Combo("Connected Devices", &selectedDeviceIndex, deviceLabels.data(), static_cast<int>(deviceLabels.size()));

        if (ImGui::Button("Select Device") && selectedDeviceIndex >= 0 && selectedDeviceIndex < static_cast<int>(connectedDevices.size()))
        {
            selectedDeviceSerial = connectedDevices[selectedDeviceIndex].serialNumber;
            selectedDeviceModel = connectedDevices[selectedDeviceIndex].model;
            hasDevice = true;
            PushLog("Selected device: " + selectedDeviceModel + " (" + selectedDeviceSerial + ")");
        }
    }

    ImGui::Spacing();
    const char *categoryNames[] = {"All Categories", "Critical", "Bloatware", "Analytics", "Optional", "User", "Unknown"};
    if (ImGui::Combo("Category Filter", &selectedCategoryIndex, categoryNames, IM_ARRAYSIZE(categoryNames)))
    {
        switch (selectedCategoryIndex)
        {
        case 1:
            activeCategory = PackageCategory::DO_NOT_TOUCH;
            break;
        case 2:
            activeCategory = PackageCategory::SAFE_TO_REMOVE;
            break;
        case 3:
            activeCategory = PackageCategory::ANALYTICS;
            break;
        case 4:
            activeCategory = PackageCategory::OPTIONAL;
            break;
        case 5:
            activeCategory = PackageCategory::USER_APP;
            break;
        case 6:
            activeCategory = PackageCategory::UNCATEGORIZED;
            break;
        default:
            activeCategory = PackageCategory::UNCATEGORIZED;
            break;
        }
    }

    ImGui::Text("Selected Device: %s", hasDevice ? selectedDeviceModel.c_str() : "None");
    ImGui::Text("Device Serial: %s", hasDevice ? selectedDeviceSerial.c_str() : "None");
    ImGui::Separator();

    ImGui::InputText("Search packages", packageSearchBuffer, IM_ARRAYSIZE(packageSearchBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Clear"))
    {
        packageSearchBuffer[0] = '\0';
    }
    ImGui::Separator();

    if (hasScanResults)
    {
        ImGui::Text("Scan Results");
        ImGui::Separator();
        ImGui::Text("Package count: %d", static_cast<int>(allPackages.size()));
        ImGui::Text("Selected category: %s", selectedCategoryIndex == 0 ? "All Categories" : GetCategoryName(activeCategory).c_str());
        ImGui::Checkbox("Show only disabled packages", &showOnlyDisabled);

        const auto toLower = [](const std::string &value)
        {
            std::string lower = value;
            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c)
                           { return std::tolower(c); });
            return lower;
        };

        std::string searchFilter = packageSearchBuffer;
        std::string searchLower = toLower(searchFilter);

        ImGui::BeginChild("PackageList", ImVec2(0, 360), true);
        ImGui::Columns(3, "PackageColumns", false);
        ImGui::SetColumnWidth(0, 520);
        ImGui::TextUnformatted("Package");
        ImGui::NextColumn();
        ImGui::TextUnformatted("Category");
        ImGui::NextColumn();
        ImGui::TextUnformatted("Status");
        ImGui::NextColumn();
        ImGui::Separator();

        int selectedCount = 0;
        for (size_t i = 0; i < classifiedPackages.size(); ++i)
        {
            const auto &pkg = classifiedPackages[i];
            if (selectedCategoryIndex != 0 && pkg.category != activeCategory)
            {
                continue;
            }
            if (showOnlyDisabled && pkg.isEnabled)
            {
                continue;
            }
            if (!searchLower.empty())
            {
                std::string packageNameLower = toLower(pkg.packageName);
                if (packageNameLower.find(searchLower) == std::string::npos)
                {
                    continue;
                }
            }

            bool selected = selectedPackageNames.count(pkg.packageName) > 0;
            if (ImGui::Checkbox(pkg.packageName.c_str(), &selected))
            {
                if (selected)
                    selectedPackageNames.insert(pkg.packageName);
                else
                    selectedPackageNames.erase(pkg.packageName);
            }
            ImGui::NextColumn();

            ImGui::Text("%s", GetCategoryName(pkg.category).c_str());
            ImGui::NextColumn();
            ImGui::Text("%s", pkg.isEnabled ? "ENABLED" : "DISABLED");
            ImGui::NextColumn();

            if (selected)
            {
                selectedCount++;
            }
        }
        ImGui::Columns(1);
        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::Text("Selected Packages (%d)", selectedCount);
        ImGui::BeginChild("SelectedPackages", ImVec2(0, 120), true);
        if (selectedCount == 0)
        {
            ImGui::TextDisabled("No packages selected.");
        }
        else
        {
            for (size_t i = 0; i < classifiedPackages.size(); ++i)
            {
                if (i < selectedPackageFlags.size() && selectedPackageFlags[i])
                {
                    ImGui::TextWrapped("%s", classifiedPackages[i].packageName.c_str());
                }
            }
        }
        ImGui::EndChild();

        ImGui::Spacing();
        if (ImGui::Button("Disable Selected"))
        {
            PerformRemoval(RemovalAction::DISABLE);
        }
        ImGui::SameLine();
        if (ImGui::Button("Uninstall Selected"))
        {
            PerformRemoval(RemovalAction::UNINSTALL);
        }
        ImGui::SameLine();
        if (ImGui::Button("Enable Selected"))
        {
            EnableSelectedPackages();
        }
        ImGui::SameLine();
        if (ImGui::Button("Restore Disabled"))
        {
            RestoreDisabledPackages();
        }
    }

    ImGui::Separator();
    ImGui::Text("Activity Log");
    ImGui::BeginChild("LogPanel", ImVec2(0, 180), true);
    for (const auto &entry : activityLog)
    {
        ImGui::TextWrapped("%s", entry.c_str());
    }
    ImGui::EndChild();

    ImGui::End();

    ImGui::Render();
    GetClientRect(hwnd, &clientRect);
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;
    glViewport(0, 0, width, height);
    glClearColor(0.11f, 0.12f, 0.15f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    SwapBuffers(hdc);
}

void GuiApp::RefreshDeviceList()
{
    connectedDevices = adbMgr.GetConnectedDevices();
    selectedDeviceIndex = 0;
    activityLog.clear();
    if (connectedDevices.empty())
    {
        PushLog("No connected devices detected.");
    }
    else
    {
        PushLog("Detected " + std::to_string(connectedDevices.size()) + " device(s).");
    }
}

void GuiApp::StartScan()
{
    if (!hasDevice || selectedDeviceSerial.empty())
    {
        PushLog("No device selected. Please select a device first.");
        return;
    }

    delete pkgMgr;
    delete pkgClassifier;
    delete removalEngine;

    pkgMgr = new PackageManager(selectedDeviceSerial);
    removalEngine = new RemovalEngine(selectedDeviceSerial);

    if (!pkgMgr->FetchAllPackages())
    {
        PushLog("Failed to fetch packages: " + pkgMgr->GetLastError());
        hasScanResults = false;
        return;
    }

    allPackages = pkgMgr->GetAllPackages();
    std::string manufacturer = adbMgr.GetDeviceProperty(selectedDeviceSerial, "ro.product.manufacturer");
    OemType oem = PackageClassifier::DetectOemType(manufacturer, "", "");
    pkgClassifier = new PackageClassifier(selectedDeviceModel, oem);
    ClassifyPackages();
    hasScanResults = true;
    PushLog("Scan complete. Found " + std::to_string(allPackages.size()) + " packages.");
}

void GuiApp::ClassifyPackages()
{
    if (!pkgMgr || !pkgClassifier)
    {
        return;
    }

    classifiedPackages.clear();
    auto safeList = pkgClassifier->ClassifyMultiple(allPackages, PackageCategory::SAFE_TO_REMOVE);
    auto optionalList = pkgClassifier->ClassifyMultiple(allPackages, PackageCategory::OPTIONAL);
    auto analyticsList = pkgClassifier->ClassifyMultiple(allPackages, PackageCategory::ANALYTICS);
    auto userList = pkgClassifier->ClassifyMultiple(allPackages, PackageCategory::USER_APP);
    auto unknownList = pkgClassifier->ClassifyMultiple(allPackages, PackageCategory::UNCATEGORIZED);
    auto criticalList = pkgClassifier->ClassifyMultiple(allPackages, PackageCategory::DO_NOT_TOUCH);

    classifiedPackages.insert(classifiedPackages.end(), safeList.begin(), safeList.end());
    classifiedPackages.insert(classifiedPackages.end(), optionalList.begin(), optionalList.end());
    classifiedPackages.insert(classifiedPackages.end(), analyticsList.begin(), analyticsList.end());
    classifiedPackages.insert(classifiedPackages.end(), userList.begin(), userList.end());
    classifiedPackages.insert(classifiedPackages.end(), unknownList.begin(), unknownList.end());
    classifiedPackages.insert(classifiedPackages.end(), criticalList.begin(), criticalList.end());
    if (selectedPackageFlags.size() < classifiedPackages.size())
    {
        selectedPackageFlags.resize(classifiedPackages.size(), false);
    }
}

void GuiApp::PerformRemoval(RemovalAction action)
{
    if (!hasScanResults)
    {
        PushLog("Scan results required before removal.");
        return;
    }

    auto findPackageByName = [&](const std::string &name) -> const Package *
    {
        for (const auto &pkg : allPackages)
        {
            if (pkg.packageName == name)
            {
                return &pkg;
            }
        }
        return nullptr;
    };

    std::vector<std::string> packagesToRemove;
    bool skippedSystemPackage = false;

    for (size_t i = 0; i < classifiedPackages.size(); ++i)
    {
        if (!(i < selectedPackageFlags.size() && selectedPackageFlags[i]))
        {
            continue;
        }

        const auto &classifiedPackage = classifiedPackages[i];
        const Package *originalPackage = findPackageByName(classifiedPackage.packageName);
        if (!originalPackage)
        {
            PushLog("Skipping unknown package: " + classifiedPackage.packageName);
            continue;
        }

        if (action == RemovalAction::DISABLE)
        {
            if (classifiedPackage.category == PackageCategory::DO_NOT_TOUCH)
            {
                PushLog("Skipping critical package for disable: " + classifiedPackage.packageName);
                skippedSystemPackage = true;
                continue;
            }
            if (!classifiedPackage.canBeDisabled)
            {
                PushLog("Skipping package that cannot be disabled safely: " + classifiedPackage.packageName);
                skippedSystemPackage = true;
                continue;
            }
        }

        packagesToRemove.push_back(classifiedPackage.packageName);
    }

    if (packagesToRemove.empty())
    {
        if (skippedSystemPackage)
        {
            PushLog("No valid user packages selected for disable.");
        }
        else
        {
            PushLog("No packages selected for removal.");
        }
        return;
    }

    lastRemovalResults = removalEngine->RemoveMultiple(packagesToRemove, action, false);
    for (const auto &result : lastRemovalResults)
    {
        PushLog(result.packageName + ": " + (result.status == RemovalStatus::SUCCESS ? "Success" : "Failed") + " - " + result.message);
    }
}

void GuiApp::RestoreDisabledPackages()
{
    if (!removalEngine)
    {
        PushLog("No removal engine initialized.");
        return;
    }

    auto disabledPackages = removalEngine->GetDisabledPackages();
    if (disabledPackages.empty())
    {
        PushLog("No disabled packages available for restore.");
        return;
    }

    for (const auto &packageName : disabledPackages)
    {
        auto result = removalEngine->EnablePackage(packageName);
        PushLog("Restore " + packageName + ": " + (result.status == RemovalStatus::SUCCESS ? "Success" : "Failed") + " - " + result.message);
    }
}

void GuiApp::EnableSelectedPackages()
{
    if (!removalEngine)
    {
        PushLog("No removal engine initialized.");
        return;
    }

    std::vector<std::string> packagesToEnable;
    for (size_t i = 0; i < classifiedPackages.size(); ++i)
    {
        if (i < selectedPackageFlags.size() && selectedPackageFlags[i])
        {
            packagesToEnable.push_back(classifiedPackages[i].packageName);
        }
    }

    if (packagesToEnable.empty())
    {
        PushLog("No packages selected to enable.");
        return;
    }

    for (const auto &packageName : packagesToEnable)
    {
        auto result = removalEngine->EnablePackage(packageName);
        PushLog("Enable " + packageName + ": " + (result.status == RemovalStatus::SUCCESS ? "Success" : "Failed") + " - " + result.message);
    }
}

void GuiApp::PushLog(const std::string &message)
{
    activityLog.push_back(message);
    if (activityLog.size() > 50)
    {
        activityLog.erase(activityLog.begin());
    }
}

const std::string &GuiApp::GetSelectedDeviceSerial() const
{
    return selectedDeviceSerial;
}

std::string GuiApp::GetAuthStatusName(AuthStatus status) const
{
    switch (status)
    {
    case AuthStatus::AUTHORIZED:
        return "AUTHORIZED";
    case AuthStatus::UNAUTHORIZED:
        return "UNAUTHORIZED";
    case AuthStatus::OFFLINE:
        return "OFFLINE";
    default:
        return "UNKNOWN";
    }
}

std::string GuiApp::GetCategoryName(PackageCategory category) const
{
    return PackageClassifier::GetCategoryName(category);
}
