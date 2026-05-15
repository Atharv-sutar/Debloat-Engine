#pragma once

#include <string>
#include <vector>

/**
 * CommandExecutor - Utility class for executing system commands and capturing output
 * Platform: Windows
 */
class CommandExecutor
{
public:
    /**
     * Execute a command and capture its output
     * @param command The command to execute
     * @param output Reference to store command output
     * @return true if command executed successfully (exit code 0), false otherwise
     */
    static bool Execute(const std::string& command, std::string& output);

    /**
     * Execute a command without capturing output
     * @param command The command to execute
     * @return true if command executed successfully, false otherwise
     */
    static bool ExecuteQuiet(const std::string& command);

    /**
     * Get the exit code of the last executed command
     * @return Exit code from the last Execute/ExecuteQuiet call
     */
    static int GetLastExitCode();

    /**
     * Split output by lines
     * @param output The output string
     * @return Vector of strings, one per line
     */
    static std::vector<std::string> SplitLines(const std::string& output);

private:
    static int lastExitCode;
};
