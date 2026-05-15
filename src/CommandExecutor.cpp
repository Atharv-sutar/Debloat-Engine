#include "CommandExecutor.h"
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <memory>
#include <array>

int CommandExecutor::lastExitCode = 0;

bool CommandExecutor::Execute(const std::string& command, std::string& output)
{
    output.clear();

    // Create a pipe to capture output
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command.c_str(), "r"), &_pclose);

    if (!pipe)
    {
        lastExitCode = -1;
        return false;
    }

    // Read output from the pipe
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        output += buffer.data();
    }

    // Close pipe and get exit code
    lastExitCode = _pclose(pipe.get());

    // Normalize line endings (Windows uses \r\n)
    size_t pos = 0;
    while ((pos = output.find("\r\n", pos)) != std::string::npos)
    {
        output.replace(pos, 2, "\n");
    }

    return lastExitCode == 0;
}

bool CommandExecutor::ExecuteQuiet(const std::string& command)
{
    lastExitCode = std::system(command.c_str());
    return lastExitCode == 0;
}

int CommandExecutor::GetLastExitCode()
{
    return lastExitCode;
}

std::vector<std::string> CommandExecutor::SplitLines(const std::string& output)
{
    std::vector<std::string> lines;
    std::stringstream ss(output);
    std::string line;

    while (std::getline(ss, line))
    {
        // Remove trailing whitespace
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        // Only add non-empty lines
        if (!line.empty())
        {
            lines.push_back(line);
        }
    }

    return lines;
}
