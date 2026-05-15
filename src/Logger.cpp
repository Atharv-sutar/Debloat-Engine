#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

Logger::Logger()
    : removalCount(0), successfulRemovals(0), failedRemovals(0)
{
}

Logger::~Logger()
{
    EndSession();
}

Logger& Logger::GetInstance()
{
    static Logger instance;
    return instance;
}

bool Logger::StartSession(const std::string& logsDir)
{
    logsDirectory = logsDir;

    try
    {
        // Create logs directory if it doesn't exist
        if (!fs::exists(logsDirectory))
        {
            fs::create_directories(logsDirectory);
        }

        // Generate unique filename with timestamp
        auto now = std::chrono::system_clock::now();
        auto timePoint = std::chrono::system_clock::to_time_t(now);
        std::tm* tm_info = std::localtime(&timePoint);

        std::ostringstream oss;
        oss << logsDirectory << "/debloat_"
            << std::put_time(tm_info, "%Y%m%d_%H%M%S")
            << ".log";

        currentLogFile = oss.str();

        // Open log file
        logStream.open(currentLogFile, std::ios::app);
        if (!logStream.is_open())
        {
            std::cerr << "[ERROR] Failed to open log file: " << currentLogFile << "\n";
            return false;
        }

        // Write session start header
        logStream << "================================================================================\n";
        logStream << "DeBloat Session Log - " << GetFormattedDateTime() << "\n";
        logStream << "================================================================================\n";
        logStream << "\n";
        logStream.flush();

        Info("Logging session started", currentLogFile);
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ERROR] Exception during session start: " << e.what() << "\n";
        return false;
    }
}

bool Logger::EndSession()
{
    if (!logStream.is_open())
        return false;

    try
    {
        logStream << "\n";
        logStream << "================================================================================\n";
        logStream << "Session Summary\n";
        logStream << "================================================================================\n";
        logStream << GetSessionSummary();
        logStream << "================================================================================\n";
        logStream.close();
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ERROR] Exception during session end: " << e.what() << "\n";
        return false;
    }
}

void Logger::Info(const std::string& message, const std::string& details)
{
    LogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.level = LogLevel::INFO;
    entry.message = message;
    entry.details = details;

    entries.push_back(entry);
    WriteToFile(entry);

    std::cout << "[INFO] " << message;
    if (!details.empty())
        std::cout << " (" << details << ")";
    std::cout << "\n";
}

void Logger::Warning(const std::string& message, const std::string& details)
{
    LogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.level = LogLevel::WARNING;
    entry.message = message;
    entry.details = details;

    entries.push_back(entry);
    WriteToFile(entry);

    std::cout << "[WARNING] " << message;
    if (!details.empty())
        std::cout << " (" << details << ")";
    std::cout << "\n";
}

void Logger::Error(const std::string& message, const std::string& details)
{
    LogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.level = LogLevel::ERROR;
    entry.message = message;
    entry.details = details;

    entries.push_back(entry);
    WriteToFile(entry);

    std::cout << "[ERROR] " << message;
    if (!details.empty())
        std::cout << " (" << details << ")";
    std::cout << "\n";
}

void Logger::Success(const std::string& message, const std::string& details)
{
    LogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.level = LogLevel::SUCCESS;
    entry.message = message;
    entry.details = details;

    entries.push_back(entry);
    WriteToFile(entry);

    std::cout << "[SUCCESS] " << message;
    if (!details.empty())
        std::cout << " (" << details << ")";
    std::cout << "\n";
}

void Logger::RemovalLog(const std::string& packageName, const std::string& action,
                        bool success, const std::string& error)
{
    removalCount++;

    LogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.level = success ? LogLevel::SUCCESS : LogLevel::ERROR;
    entry.message = (success ? "REMOVAL SUCCESS" : "REMOVAL FAILED");
    entry.details = "Package: " + packageName + " | Action: " + action;

    if (!error.empty())
        entry.details += " | Error: " + error;

    entries.push_back(entry);
    WriteToFile(entry);

    if (success)
    {
        successfulRemovals++;
        std::cout << "[REMOVAL OK] " << packageName << " (" << action << ")\n";
    }
    else
    {
        failedRemovals++;
        std::cout << "[REMOVAL FAILED] " << packageName << " (" << action << ")\n";
        if (!error.empty())
            std::cout << "  Error: " << error << "\n";
    }
}

void Logger::WriteToFile(const LogEntry& entry)
{
    if (!logStream.is_open())
        return;

    logStream << "[" << GetTimestampString(entry.timestamp) << "] "
              << "[" << GetLevelString(entry.level) << "] "
              << entry.message;

    if (!entry.details.empty())
        logStream << " - " << entry.details;

    logStream << "\n";
    logStream.flush();
}

std::string Logger::GetLevelString(LogLevel level) const
{
    switch (level)
    {
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARNING:
        return "WARN";
    case LogLevel::ERROR:
        return "ERR";
    case LogLevel::SUCCESS:
        return "OK";
    case LogLevel::REMOVAL:
        return "REMOVAL";
    default:
        return "UNKNOWN";
    }
}

std::string Logger::GetTimestampString(const std::chrono::system_clock::time_point& tp) const
{
    auto time = std::chrono::system_clock::to_time_t(tp);
    std::tm* tm_info = std::localtime(&time);

    std::ostringstream oss;
    oss << std::put_time(tm_info, "%H:%M:%S");
    return oss.str();
}

std::string Logger::GetFormattedDateTime() const
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm_info = std::localtime(&time);

    std::ostringstream oss;
    oss << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::GetSessionSummary() const
{
    std::ostringstream oss;

    oss << "Timestamp: " << GetFormattedDateTime() << "\n";
    oss << "Log File: " << currentLogFile << "\n";
    oss << "Total Entries: " << entries.size() << "\n";
    oss << "\nRemoval Statistics:\n";
    oss << "  Total Removal Attempts: " << removalCount << "\n";
    oss << "  Successful Removals: " << successfulRemovals << "\n";
    oss << "  Failed Removals: " << failedRemovals << "\n";

    if (removalCount > 0)
    {
        float successRate = (static_cast<float>(successfulRemovals) / removalCount) * 100.0f;
        oss << "  Success Rate: " << std::fixed << std::setprecision(1) << successRate << "%\n";
    }

    oss << "\nAction Summary:\n";
    size_t infoCount = 0, warnCount = 0, errCount = 0, successCount = 0;

    for (const auto& entry : entries)
    {
        switch (entry.level)
        {
        case LogLevel::INFO:
            infoCount++;
            break;
        case LogLevel::WARNING:
            warnCount++;
            break;
        case LogLevel::ERROR:
            errCount++;
            break;
        case LogLevel::SUCCESS:
        case LogLevel::REMOVAL:
            successCount++;
            break;
        }
    }

    oss << "  Info Messages: " << infoCount << "\n";
    oss << "  Warnings: " << warnCount << "\n";
    oss << "  Errors: " << errCount << "\n";
    oss << "  Successes: " << successCount << "\n";
    oss << "\n";

    return oss.str();
}
