# Debloat Engine — DeBloat_AntiGrav

A modern C++ Android debloating engine powered by ADB, designed to safely classify, analyze, and remove unnecessary Android packages through a structured and safety-focused workflow.

---

## Features

* ADB-powered Android package management
* Intelligent package classification system
* Safe removal workflows with confirmations
* Logging and auditing support
* Modular and extensible C++ architecture
* Rule-based package analysis and filtering
* Windows-first development with CMake support
* Runtime diagnostics and command tracing
* Safety-focused package removal system
* Expandable architecture for future GUI integration

---

## Project Goals

Debloat Engine aims to provide:

* Safe Android debloating workflows
* Automated package classification
* Reliable removal orchestration
* Detailed logging and rollback planning
* Expandable classifier logic
* Research-friendly modular architecture

The project focuses heavily on safety, transparency, and maintainability rather than blindly removing packages.

---

# Requirements

## Android Platform Tools (ADB)

This application requires Android Platform Tools (ADB)
to be installed and available in system PATH.

ADB is developed by the Android Open Source Project.

Download Platform Tools:

https://developer.android.com/tools/releases/platform-tools

---

## Device Requirements

* Android device with USB Debugging enabled
* Authorized ADB connection
* USB cable or wireless ADB
* Some advanced removals may require root access

---

# Quick Start

## Clone Repository

```bash
git clone https://github.com/Atharv-sutar/Debloat-Engine.git
cd Debloat-Engine
```

---

# Building

## Windows Quick Build

Run:

```bat
build.bat
```

or:

```bat
auto-build.bat
```

---

## CMake Build (Cross Platform)

### Configure

```bash
cmake -S . -B build
```

### Build

```bash
cmake --build build --config Release
```

---

# Running

Execute the generated binary from the build folder.

Two entry points currently exist:

| Entry Point           | Purpose               |
| --------------------- | --------------------- |
| `main.cpp`            | Development / testing |
| `main_production.cpp` | Production execution  |

---

# Repository Structure

```text
Debloat-Engine/
│
├── include/                  # Public interfaces and headers
├── src/                      # Core implementations
├── build/                    # Build output
├── logs/                     # Runtime logs
│
├── CMakeLists.txt
├── build.bat
├── auto-build.bat
├── auto-build.ps1
│
├── AUTO_BUILD_GUIDE.md
├── QUICK_REFERENCE.md
├── FIX_PLAN.md
│
├── PHASE1_README.md
├── PHASE2_README.md
├── ...
├── PHASE8_COMPLETE.md
│
└── README.md
```

---

# Core Components

## AdbManager

Handles:

* Device discovery
* ADB command execution
* Shell communication
* Device connection management

---

## CommandExecutor

Responsible for:

* Running shell commands
* Capturing stdout/stderr
* Exit code handling
* Process management

---

## Logger

Centralized logging system:

* Runtime logs
* Debug traces
* Error reporting
* Audit logging

Logs are stored in:

```text
logs/
```

---

## Package

Core package data model containing:

* Package name
* Labels
* Classification metadata
* Flags and states

---

## PackageClassifier

Classification engine responsible for:

* Package categorization
* Heuristic analysis
* Safety evaluation
* Removability detection

Example classifications:

* Critical
* System
* User
* Removable
* Bloatware

---

## PackageManager

Higher-level package operations:

* Package enumeration
* Filtering
* Safe uninstall orchestration
* Query utilities

---

## RemovalEngine

Handles:

* Removal workflows
* Safety confirmations
* Rollback attempts
* Execution validation

---

## UserInterface

CLI interaction layer:

* Prompts
* Confirmations
* Progress display
* Dry-run flows
* User warnings

---

# Workflow Overview

```text
ADB Device Detection
        ↓
Package Enumeration
        ↓
Package Object Creation
        ↓
Classification Engine
        ↓
Safety Filtering
        ↓
User Confirmation
        ↓
Removal Execution
        ↓
Logging & Audit Storage
```

---

# Safety Features

* Confirmation prompts before removal
* Classification-aware safety checks
* Runtime validation
* Rollback planning architecture
* Detailed removal logging
* Dry-run compatible design

---

# Recommended Best Practices

* Test on emulator first
* Avoid primary personal devices initially
* Export package lists before removal
* Review suggested removals carefully
* Use dry-run/testing modes whenever possible

---

# Logging

Runtime logs are automatically stored inside:

```text
logs/
```

Logs include:

* Executed commands
* Errors
* Classification traces
* Removal operations
* Runtime diagnostics

---

# Development Notes

## Recommended Environment

* Windows 10/11
* Visual Studio 2022
* MSVC Compiler
* CMake
* VS Code + CMake Tools

---

# Extending The Project

Future improvements may include:

* GUI frontend
* JSON export/import
* AI-assisted classification
* Cloud-based package reputation
* Emulator integration
* Automated rollback snapshots
* Non-interactive batch mode
* Cross-device compatibility testing

---

# Suggested Testing Targets

Recommended areas for unit testing:

* PackageClassifier
* CommandExecutor
* RemovalEngine
* PackageManager

Framework suggestions:

* GoogleTest
* Catch2

---

# Documentation

Additional project documentation:

| File                                     | Purpose                     |
| ---------------------------------------- | --------------------------- |
| `AUTO_BUILD_GUIDE.md`                    | Build automation            |
| `QUICK_REFERENCE.md`                     | Quick usage notes           |
| `FIX_PLAN.md`                            | Project restructuring       |
| `DYNAMIC_CLASSIFICATION_IMPROVEMENTS.md` | Classification roadmap      |
| `CLASSIFICATION_CONTROL_TRANSFER.md`     | Classification architecture |
| `PHASE1_README.md → PHASE8_COMPLETE.md`  | Development progression     |

---

# Troubleshooting

## ADB Not Found

Ensure ADB is:

* Installed
* Added to system PATH

Verify:

```bash
adb version
```

---

## Device Not Detected

Check:

```bash
adb devices
```

Ensure:

* USB debugging enabled
* Device authorized
* Cable supports data transfer

---

## Permission Failures

Some operations:

* Require elevated ADB access
* May require root
* Can behave differently across OEMs

Testing on emulators is recommended.

---

# Contributing

Contributions are welcome.

Suggested workflow:

1. Fork repository
2. Create feature branch
3. Implement changes
4. Add tests
5. Open Pull Request

Please keep:

* Existing architecture consistency
* Code style consistency
* Modular design principles

---

# Acknowledgements

This project interacts with Android Debug Bridge (ADB),
part of Android Platform Tools by the Android Open Source Project.

Android and ADB are trademarks of Google LLC.

---

# License

Add your preferred license here.

Recommended:

* MIT License
* Apache License 2.0

---

# Author

Developed by Atharv Sutar

GitHub:
https://github.com/Atharv-sutar
