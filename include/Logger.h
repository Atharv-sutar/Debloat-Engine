#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>

enum class LogLevel
{
    INFO,
    WARNING,
    ERROR,
    SUCCESS,
    REMOVAL
};

struct LogEntry
{
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::string message;
    std::string details;
};

class Logger
{
public:
    static Logger& GetInstance();

    // Logging methods
    void Info(const std::string& message, const std::string& details = "");
    void Warning(const std::string& message, const std::string& details = "");
    void Error(const std::string& message, const std::string& details = "");
    void Success(const std::string& message, const std::string& details = "");
    void RemovalLog(const std::string& packageName, const std::string& action, 
                    bool success, const std::string& error = "");

    // File operations
    bool StartSession(const std::string& logsDirectory = "logs");
    bool EndSession();
    
    // Getters
    std::string GetCurrentLogFile() const { return currentLogFile; }
    const std::vector<LogEntry>& GetHistory() const { return entries; }
    size_t GetEntryCount() const { return entries.size(); }

    // Statistics
    size_t GetRemovalCount() const { return removalCount; }
    size_t GetSuccessfulRemovals() const { return successfulRemovals; }
    size_t GetFailedRemovals() const { return failedRemovals; }

    // Summary
    std::string GetSessionSummary() const;

private:
    Logger();
    ~Logger();

    void WriteToFile(const LogEntry& entry);
    std::string GetLevelString(LogLevel level) const;
    std::string GetTimestampString(const std::chrono::system_clock::time_point& tp) const;
    std::string GetFormattedDateTime() const;

    std::vector<LogEntry> entries;
    std::ofstream logStream;
    std::string currentLogFile;
    std::string logsDirectory;
    
    size_t removalCount = 0;
    size_t successfulRemovals = 0;
    size_t failedRemovals = 0;
};
