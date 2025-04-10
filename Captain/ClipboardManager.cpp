#include "ClipboardManager.h"
#include "ClipperEventManager.h"
#include <Windows.h>
#include "Clipboard.h"

// Initialize a static member variable to hold the previous clipboard content, initially set to an empty string.
std::string ClipboardManager::_prevClipboard = "";

// Initialize a static member variable to hold the current clipboard content, initially set to an empty string.
std::string ClipboardManager::ClipboardText = "";

// This function continuously monitors and processes clipboard changes.
void ClipboardManager::Run()
{
    // Infinite loop to keep running the clipboard checking process.
    while (true)
    {
        // Sleep for 2000 milliseconds (2 seconds) to avoid constantly polling and to reduce CPU usage.
        Sleep(2000);

        // Fetch the current text content from the clipboard and store it in the ClipboardText variable.
        ClipboardText = Clipboard::GetText();

        // If the current clipboard content is the same as the previous one, continue to the next iteration (skip the rest of the code).
        if (ClipboardText == _prevClipboard)
        {
            continue;
        }

        // Update the previous clipboard content to the current clipboard content (since it has changed).
        _prevClipboard = ClipboardText;

        // Call the Action method of ClipperEventManager, which is assumed to handle some event related to the clipboard.
        // If the action returns false, clear the previous clipboard content (indicating no relevant event was triggered).
        if (!ClipperEventManager::Action())
        {
            _prevClipboard.clear(); // Clear the stored clipboard text since the action didn't succeed.
        }
    }
}
