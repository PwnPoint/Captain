#include <iostream>
#include <fstream>
#include "SelfDestruct.h"

void SelfDestruct::Melt()
{
    // Declare a buffer to store the path of the current executable file.
    char buffer[MAX_PATH];

    // Retrieve the full path of the current executable and store it in the buffer.
    // `GetModuleFileName` is a Windows API function that returns the path of the executable file of the current process.
    GetModuleFileName(nullptr, buffer, MAX_PATH);

    // Create a new VBS (Visual Basic Script) file named "delete_me.vbs" for later execution.
    std::ofstream vbsFile("delete_me.vbs");

    // Write the script content into the VBS file.
    // The script will perform the following actions:
    // - Create a FileSystemObject (FSO) to interact with the file system.
    // - Delete the current executable file (the one from which this code is running).
    // - Delete the VBS script itself after execution.
    vbsFile //<< "WScript.Sleep 2000\n"   // Optional line to add a delay (commented out here).
        << "Set objFSO = CreateObject(\"Scripting.FileSystemObject\")\n"   // Create FSO object.
        << "objFSO.DeleteFile \"" << buffer << "\"\n"   // Delete the executable file.
        << "strPath = WScript.ScriptFullName\n"   // Get the full path of the VBS script.
        << "objFSO.DeleteFile strPath";   // Delete the VBS script itself after execution.

    // Close the VBS file after writing the content.
    vbsFile.close();

    // Prepare the startup information and process information structures for process creation.
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // Zero out the memory of both structures to ensure they are properly initialized.
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    // Set the size of the STARTUPINFO structure to ensure compatibility with `CreateProcess`.
    si.cb = sizeof(si);

    // Set the `dwFlags` to use the `SW_HIDE` flag to hide the window when executing the script.
    si.dwFlags = STARTF_USESHOWWINDOW;

    // Set `wShowWindow` to SW_HIDE to ensure the script runs without showing a window.
    si.wShowWindow = SW_HIDE;

    // Create a new process to run the "delete_me.vbs" script using Windows' `CreateProcess` function.
    // This launches the VBS script that will delete the executable and itself.
    CreateProcess(nullptr, (char*)"wscript delete_me.vbs", nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);

    // Call ExitProcess to terminate the current process immediately, after initiating the script.
    ExitProcess(0);

    // Close the thread and process handles (cleanup).
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
}
