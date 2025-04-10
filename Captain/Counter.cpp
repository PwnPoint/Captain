#include "Counter.h"
#include <algorithm>
#include <mutex>

// Browsers: These are integer variables that track the count of different browser-related items.
int Counter::Passwords = 0;      // Track the number of passwords found.
int Counter::CreditCards = 0;    // Track the number of credit cards found.
int Counter::AutoFill = 0;       // Track the number of autofill entries found.
int Counter::Cookies = 0;        // Track the number of cookies found.
int Counter::History = 0;        // Track the number of browser history entries found.
int Counter::Bookmarks = 0;      // Track the number of bookmarks found.
int Counter::Downloads = 0;      // Track the number of downloads found.

// Applications: These are integer variables for counting items found in specific applications.
int Counter::Vpn = 0;            // Track VPN-related items.
int Counter::Pidgin = 0;         // Track Pidgin chat application items.
int Counter::Wallets = 0;        // Track wallet-related items.
int Counter::BrowserWallets = 0; // Track browser wallet items.
int Counter::FtpHosts = 0;       // Track FTP host items.

// Sessions & tokens: These are boolean variables that track whether certain sessions are active or not.
bool Counter::Discord = false;   // Check if the Discord session is active.
bool Counter::Outlook = false;   // Check if the Outlook session is active.

// System Data: These variables track system-related information.
int Counter::SavedWifiNetworks = 0;  // Track the number of saved Wi-Fi networks.
bool Counter::ProductKey = false;    // Track if a product key is found.
bool Counter::DesktopScreenshot = false; // Track if a desktop screenshot was found.

// Grabber stats: These variables track statistics for a grabber function.
int Counter::GrabberImages = 0;      // Track the number of images grabbed.
int Counter::GrabberDocuments = 0;   // Track the number of documents grabbed.
int Counter::GrabberDatabases = 0;   // Track the number of databases grabbed.
int Counter::GrabberSourceCodes = 0; // Track the number of source codes grabbed.

// Banking & Cryptocurrency services detection: These variables track the status of specific services.
bool Counter::BankingServices = false; // Detect if banking services are found.
bool Counter::CryptoServices = false;  // Detect if cryptocurrency services are found.

// Detected services: These vectors store the names of detected services.
std::vector<std::string> Counter::DetectedBankingServices;    // Store detected banking services.
std::vector<std::string> Counter::DetectedCryptoServices;     // Store detected cryptocurrency services.


// Get string value: A helper function to return a formatted string for boolean values (found or not found).
std::string Counter::getSValue(const std::string& application, bool value)
{
    // If value is true, return a message indicating the application was found, otherwise not found.
    return value ? "\n" + application + ":" + "  Found" : "\n" + application + ":" + "  Not found";
}

// Get integer value: A helper function to return a formatted string for integer values (such as counts).
std::string Counter::getIValue(const std::string& application, int value)
{
    return "\n" + application + ": " + std::to_string(value); // Return the application name and its integer value.
}

// Get list of string values: A helper function to return a formatted string for a list of detected services.
std::string Counter::getLValue(std::string application, std::vector<std::string> value, char separator)
{
    std::sort(value.begin(), value.end());  // Sort the values alphabetically.

    std::string result; // Initialize an empty string to hold the result.

    if (value.empty()) { // If no services were detected, return a message indicating "No data."
        result = "\n" + std::string(1, separator) + " " + application + " (No data)";
    }
    else { // Otherwise, return each service in the list.
        result = "\n   " + std::string(1, separator) + " " + application + ":\n";
        for (const auto& v : value) {
            result += "\t\t" + std::string(1, separator) + " " + v + "\n"; // Format each item with tab indentation.
        }
    }

    return result; // Return the formatted string.
}

// Get boolean value: A helper function to return a custom success or failure message based on a boolean value.
std::string Counter::getBValue(bool value, const std::string& success, const std::string& failed)
{
    return value ? success : failed; // Return success message if value is true, otherwise return failed message.
}

