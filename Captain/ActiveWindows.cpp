#include "ActiveWindows.h"
#include <iostream>
#include <iomanip> 

void ActiveWindows::WriteWindows(const std::string& sSavePath) {
    // Open the output file at the specified path in append mode.
    // This ensures that new data is added to the end of the file without overwriting existing data.
    std::ofstream outFile(sSavePath + "\\Windows_Info.txt", std::ios_base::app);

    // Check if the file was opened successfully. If not, log an error and exit the function.
    if (!outFile) {
        std::cerr << "Error opening output file!" << std::endl;
        return;
    }

    // Set the output format for the file to align text to the left.
    // Write column headers for the table to make the data readable.
    outFile << std::left;
    outFile << std::setw(40) << "WindowName"
        << std::setw(40) << "WindowTitle"
        << std::setw(40) << "WindowID"
        << std::setw(20) << "ProcessName" << std::endl;
    outFile << std::endl;

    // Array to store process IDs and variable to store the size of the array.
    DWORD aProcesses[1024], cbNeeded;

    // Retrieve the list of all process IDs currently running on the system.
    // EnumProcesses fills the `aProcesses` array with process IDs.
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        std::cerr << "Error enumerating processes!" << std::endl;
        return;
    }

    // Calculate the number of processes by dividing the number of bytes returned (`cbNeeded`)
    // by the size of a single process ID (DWORD).
    DWORD cProcesses = cbNeeded / sizeof(DWORD);

    // Dynamically allocate an array of handles to hold handles to each process.
    HANDLE* hProcesses = new HANDLE[cProcesses];

    // Loop through all process IDs and attempt to open a handle to each process.
    for (DWORD i = 0; i < cProcesses; ++i) {
        hProcesses[i] = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
    }

    // Temporary variable to store the process name.
    char szProcessName[MAX_PATH] = "";

    // Iterate over each process to retrieve its information.
    for (DWORD i = 0; i < cProcesses; ++i) {
        // Skip processes for which a handle could not be opened.
        if (!hProcesses[i]) continue;

        // Attempt to retrieve the full path of the executable for the current process.
        if (GetModuleFileNameExA(hProcesses[i], NULL, szProcessName, MAX_PATH)) {
            // If successful, extract only the file name from the full path by finding the last backslash.
            char* p = strrchr(szProcessName, '\\');
            if (p) strcpy_s(szProcessName, p + 1);
        }
        else {
            // If the executable name cannot be retrieved, mark it as unknown.
            strcpy_s(szProcessName, "[unknown]");
        }

        // Temporary variable to store the title of the main window of the process.
        char szWindowTitle[MAX_PATH] = "";

        // Start with the first top-level window in the system.
        HWND hWnd = GetTopWindow(NULL);

        // Iterate over all top-level windows to find the one associated with the current process.
        while (hWnd != NULL) {
            DWORD dwProcessId;

            // Retrieve the process ID associated with the current window.
            GetWindowThreadProcessId(hWnd, &dwProcessId);

            // If the process ID matches, retrieve the window's title.
            if (dwProcessId == aProcesses[i]) {
                GetWindowTextA(hWnd, szWindowTitle, sizeof(szWindowTitle));
                break; // Stop searching once a match is found.
            }

            // Move to the next top-level window.
            hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
        }

        // Write the collected information about the process to the file.
        // Include the process name, window title, process ID, and process name again for alignment.
        outFile << std::setw(40) << szProcessName
            << std::setw(40) << szWindowTitle
            << std::setw(40) << aProcesses[i]
            << std::setw(20) << szProcessName << std::endl;

        // Close the handle to the current process to free system resources.
        CloseHandle(hProcesses[i]);
    }

    // Free the memory allocated for the array of process handles.
    delete[] hProcesses;
}
