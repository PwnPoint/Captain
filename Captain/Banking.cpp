#include "Banking.h"
#include "Config.h"
#include "Counter.h"

#include <Windows.h>
#include <wininet.h>
#include <Shlwapi.h>
#include <iostream>
#include <algorithm>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <mutex>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "Shlwapi.lib")

// Declare a static map to associate credit card type names with their corresponding regex patterns.
std::map<std::string, std::regex> Banking::CreditCardTypes = {
    // Each entry contains a string for the credit card name and a regex pattern for matching the card number.
    { "Amex Card", std::regex("^3[47][0-9]{13}$") },  // American Express (starts with 34 or 37, followed by 13 digits)
    { "BCGlobal", std::regex("^(6541|6556)[0-9]{12}$") },  // BCGlobal (starts with 6541 or 6556, followed by 12 digits)
    { "Carte Blanche Card", std::regex("^389[0-9]{11}$") },  // Carte Blanche (starts with 389, followed by 11 digits)
    { "Diners Club Card", std::regex("^3(?:0[0-5]|[68][0-9])[0-9]{11}$") },  // Diners Club (starts with 30-35 or 36-39, followed by 11 digits)
    { "Discover Card", std::regex("6(?:011|5[0-9]{2})[0-9]{12}$") },  // Discover (starts with 6011 or 6 followed by 3 digits, then 12 digits)
    { "Insta Payment Card", std::regex("^63[7-9][0-9]{13}$") },  // Insta Payment (starts with 63, followed by 7-9, then 13 digits)
    { "JCB Card", std::regex("^(?:2131|1800|35\\d{3})\\d{11}$") },  // JCB (starts with 2131, 1800, or 35, followed by 3 digits and 11 more)
    { "KoreanLocalCard", std::regex("^9[0-9]{15}$") },  // Korean local (starts with 9, followed by 15 digits)
    { "Laser Card", std::regex("^(6304|6706|6709|6771)[0-9]{12,15}$") },  // Laser (starts with 6304, 6706, 6709, or 6771, followed by 12-15 digits)
    { "Maestro Card", std::regex("^(5018|5020|5038|6304|6759|6761|6763)[0-9]{8,15}$") },  // Maestro (starts with specific prefixes, followed by 8-15 digits)
    { "Mastercard", std::regex("5[1-5][0-9]{14}$") },  // Mastercard (starts with 51-55, followed by 14 digits)
    { "Solo Card", std::regex("^(6334|6767)[0-9]{12}|(6334|6767)[0-9]{14}|(6334|6767)[0-9]{15}$") },  // Solo (starts with 6334 or 6767, followed by 12, 14, or 15 digits)
    { "Switch Card", std::regex("^(4903|4905|4911|4936|6333|6759)[0-9]{12}|(4903|4905|4911|4936|6333|6759)[0-9]{14}|(4903|4905|4911|4936|6333|6759)[0-9]{15}|564182[0-9]{10}|564182[0-9]{12}|564182[0-9]{13}|633110[0-9]{10}|633110[0-9]{12}|633110[0-9]{13}$") },  // Switch (starts with specific prefixes and followed by 12, 14, or 15 digits)
    { "Union Pay Card", std::regex("^(62[0-9]{14,17})$") },  // UnionPay (starts with 62, followed by 14-17 digits)
    { "Visa Card", std::regex("4[0-9]{12}(?:[0-9]{3})?$") },  // Visa (starts with 4, followed by 12 digits, optionally with 3 more)
    { "Visa Master Card", std::regex("^(?:4[0-9]{12}(?:[0-9]{3})?|5[1-5][0-9]{14})$") },  // Visa or Mastercard (starts with Visa's or Mastercard's prefix)
    { "Express Card", std::regex("^3[47][0-9]{13}$") }  // Another type for American Express, same pattern as "Amex Card"
};

// Detect credit card type based on number, using the CreditCardTypes map
std::string Banking::detectCreditCardType(std::string cardNumber)
{
    // Iterate through the CreditCardTypes map
    for (auto& item : CreditCardTypes) {
        // Check if cardNumber matches the regex pattern for each card type
        if (std::regex_match(cardNumber, item.second)) {
            return item.first;  // Return the name of the card type if matched
        }
    }
    return "Unknown";  // Return "Unknown" if no match is found
}

// Append a domain value to a list if it's valid and not already in the list
bool Banking::AppendValue(std::string value, std::vector<std::string>& domains)
{
    char hostName[256] = { 0 };  // Initialize a buffer for the host name
    URL_COMPONENTS urlComponents = { sizeof(URL_COMPONENTS) };  // Initialize URL_COMPONENTS struct
    urlComponents.dwHostNameLength = 256;  // Set maximum length for host name

    // Filter out common search engine results (Google, Bing, Yandex, DuckDuckGo)
    if (
        value.find("google") != std::string::npos ||
        value.find("bing") != std::string::npos ||
        value.find("yandex") != std::string::npos ||
        value.find("duckduckgo") != std::string::npos) {
        return false;  // Return false if value contains any of these search engines
    }

    // Handle cookie values (start with '.')
    if (value.front() == '.') {
        value.erase(0, 1);  // Remove the leading dot
    }

    // Attempt to parse the URL and extract components
    if (!InternetCrackUrlA(value.c_str(), value.length(), 0, &urlComponents)) {
        return false;  // Return false if URL parsing fails
    }
    memcpy(hostName, urlComponents.lpszHostName, urlComponents.dwHostNameLength);  // Copy host name into buffer

    // Extract domain from the URL (remove .com, .org, etc.)
    std::string domain = PathFindFileNameA(hostName);

    // Remove "www." if present and convert to lowercase
    size_t wwwPos = domain.find("www.");
    if (wwwPos != std::string::npos) {
        domain.replace(wwwPos, 4, "");  // Remove "www."
    }

    // Remove ".com" if present
    size_t comPos = domain.find(".com");
    if (comPos != std::string::npos) {
        domain.erase(comPos, 4);  // Remove ".com"
    }

    // Remove ".org" if present
    size_t orgPos = domain.find(".org");
    if (orgPos != std::string::npos) {
        domain.erase(orgPos, 4);  // Remove ".org"
    }

    // Check if the domain is already in the list
    for (const auto& domainValue : domains) {
        if (domain.find(domainValue) != std::string::npos) {
            return false;  // Return false if domain already exists in the list
        }
    }

    // Convert first character of the domain to uppercase and add it to the list
    domain[0] = std::toupper(domain[0]);
    domains.push_back(domain);  // Add the domain to the list
    return true;  // Return true if value was successfully added
}

// Detect services in a value by checking against predefined lists
void Banking::DetectServices(std::string value, std::vector<std::string> toscan, std::vector<std::string>& detected, bool& ondetect)
{
    // Iterate through each service in the "toscan" list
    for (auto& service : toscan)
    {
        // Convert value to lowercase for case-insensitive comparison
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::tolower(c); });

        // Check if service is found in the value and the length of value is less than 125
        if (value.find(service) != std::string::npos && value.length() < 125)
        {
            // If valid, append the value to the detected list and set "ondetect" flag
            if (AppendValue(value, detected))
            {
                ondetect = true;  // Set flag to true if service detected
                return;
            }
        }
    }
}

// Scan a value and detect various services (e.g., Banking, Crypto)
void Banking::ScanData(std::string value)
{
    try
    {
        // Detect services related to banking, and crypto by calling DetectServices for each category
        DetectServices(value, Config::BankingServices, Counter::DetectedBankingServices, Counter::BankingServices);
        DetectServices(value, Config::CryptoServices, Counter::DetectedCryptoServices, Counter::CryptoServices);
    }
    catch (const std::exception&)
    {
        // Catch any exceptions that might occur during service detection
    }
}

// A second implementation of DetectCreditCardType (repeated, but with whitespace removal)
std::string Banking::DetectCreditCardType(std::string number)
{
    // Remove spaces from the card number
    number.erase(std::remove(number.begin(), number.end(), ' '), number.end());

    // Iterate through the CreditCardTypes map and check if the number matches any pattern
    for (auto& dictonary : CreditCardTypes)
    {
        if (std::regex_match(number, dictonary.second))
        {
            return dictonary.first;  // Return the name of the matched card type
        }
    }

    return "Unknown";  // Return "Unknown" if no match is found
}

