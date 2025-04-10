#include "ProcessList.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <iomanip> 

void ProcessList::saveProcessInfoToFile(const std::string& filePath) {

    // Create a snapshot of all currently running processes on the system.
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    // Check if the snapshot creation was successful.
    if (snapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create process snapshot." << std::endl;
        return;
    }

    // Declare and initialize a PROCESSENTRY32 structure, which will hold process information.
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(entry);  // Set the size of the structure to be compatible with the Windows API.

    // Retrieve the first process from the snapshot.
    if (!Process32First(snapshot, &entry)) {
        std::cerr << "Failed to get first process entry." << std::endl;
        CloseHandle(snapshot);  // Close the snapshot handle before returning.
        return;
    }

    // Construct the full file path to save the process information.
    std::string savePath = filePath + "\\Process_Info.txt";

    // Open the file where the process information will be written.
    std::ofstream file(savePath);

    // Check if the file was successfully opened.
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        CloseHandle(snapshot);  // Close the snapshot handle before returning.
        return;
    }

    // Set the output formatting for the file.
    file << std::left;  // Left-justify the output for columns.

    // Write the header line with column names for process information.
    file << std::setw(40) << "Process Name" << std::setw(20) << "Process ID" << "Process Path" << std::endl;
    file << std::endl;  // Add an extra line for better readability.

    // Loop through all processes in the snapshot.
    do {
        // Write the process name and process ID to the file.
        file << std::setw(40) << entry.szExeFile << std::setw(20) << entry.th32ProcessID;

        // Attempt to open the process with query information and read access.
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, entry.th32ProcessID);

        // If the process was successfully opened, retrieve the executable file path.
        if (hProcess != NULL) {
            CHAR path[MAX_PATH];  // Buffer to hold the process path.
            DWORD dwSize = MAX_PATH;  // Size of the buffer.

            // Get the full path of the process executable.
            if (GetModuleFileNameExA(hProcess, NULL, path, dwSize)) {
                file << path;  // Write the path to the file.
            }
            else {
                file << "N/A";  // If the path cannot be retrieved, write "N/A".
            }

            // Close the process handle.
            CloseHandle(hProcess);
        }
        else {
            file << "N/A";  // If the process could not be opened, write "N/A".
        }

        file << std::endl;  // Add a new line after each process entry.

    } while (Process32Next(snapshot, &entry));  // Move to the next process in the snapshot.

    // Close the file after all data has been written.
    file.close();

    // Close the snapshot handle.
    CloseHandle(snapshot);
}
