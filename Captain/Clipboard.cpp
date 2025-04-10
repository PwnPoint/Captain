#include "Clipboard.h"
#include <Windows.h>
#include <iostream>
#include <cstring>

#define _CRT_SECURE_NO_WARNINGS

std::string Clipboard::GetText()
{
    try
    {
        // Attempt to open the clipboard. nullptr means open it for the current process
        if (!OpenClipboard(nullptr)) {
            std::cerr << "Failed to open clipboard" << std::endl;
            return "";  // Return an empty string if opening the clipboard fails
        }

        // Retrieve the clipboard data in CF_TEXT format (plain text)
        HANDLE handle = GetClipboardData(CF_TEXT);
        if (handle == nullptr) {
            CloseClipboard();  // Close the clipboard if retrieving data fails
            std::cerr << "Failed to get clipboard data" << std::endl;
            return "";  // Return an empty string if no text data is available
        }

        // Lock the global memory handle to access the text data
        char* text = static_cast<char*>(GlobalLock(handle));
        if (text == nullptr) {
            CloseClipboard();  // Close clipboard if locking the data fails
            std::cerr << "Failed to lock clipboard data" << std::endl;
            return "";  // Return an empty string if the memory lock fails
        }

        // Unlock the global memory handle after use
        GlobalUnlock(handle);
        CloseClipboard();  // Close the clipboard after done

        // Return the text as a std::string
        return std::string(text);
    }
    catch (const std::exception&)
    {
        // In case of any exception (e.g., memory issues or access violations),
        // the catch block is here, but nothing is done in this example.
    }
}

void Clipboard::SetText(std::string text)
{
    // Attempt to open the clipboard for writing
    if (!OpenClipboard(nullptr)) {
        std::cerr << "Failed to open clipboard" << std::endl;
        return;  // Return if opening the clipboard fails
    }

    // Empty the clipboard before setting new data
    EmptyClipboard();

    // Allocate global memory for the text to be copied to the clipboard
    HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);  // Include space for null-terminator
    if (handle == nullptr) {
        CloseClipboard();  // Close clipboard if memory allocation fails
        std::cerr << "Failed to allocate memory" << std::endl;
        return;  // Return if memory allocation fails
    }

    // Lock the global memory handle to copy the text into it
    char* data = static_cast<char*>(GlobalLock(handle));
    if (data == nullptr) {
        GlobalFree(handle);  // Free allocated memory if lock fails
        CloseClipboard();  // Close clipboard after failure
        std::cerr << "Failed to lock memory" << std::endl;
        return;  // Return if locking the allocated memory fails
    }

    // Safely copy the string into the allocated memory using strcpy_s
    strcpy_s(data, text.size() + 1, text.c_str());  // +1 for the null-terminator

    // Unlock the global memory handle after use
    GlobalUnlock(handle);

    // Set the clipboard data as CF_TEXT (plain text) and associate it with the allocated handle
    SetClipboardData(CF_TEXT, handle);

    // Close the clipboard after setting the data
    CloseClipboard();

    // Output to console that the text has been successfully copied to the clipboard
    std::cout << "Text \"" << text << "\" has been copied to clipboard." << std::endl;
}
