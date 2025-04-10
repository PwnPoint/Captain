#include "DesktopScreenshot.h"
#include <windows.h>
#include <Psapi.h>
#include <fstream>
#include <iostream>
#include <dwmapi.h>
#include <olectl.h>
#include <ctime>

#pragma comment(lib,"dwmapi.lib")

#define _CRT_SECURE_NO_WARNINGS

//void WriteWindows(const std::string& sSavePath) {
//    // Open output file for appending. The file path includes "Windows.txt" in the provided directory.
//    std::ofstream outFile(sSavePath + "\\Windows.txt", std::ios_base::app);
//    if (!outFile) {
//        // If the file cannot be opened, log an error and exit the function.
//        std::cerr << "Error opening output file!" << std::endl;
//        return;
//    }
//
//    // Get a list of all process identifiers (PIDs).
//    DWORD aProcesses[1024], cbNeeded;
//    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
//        // If unable to enumerate processes, log an error and exit the function.
//        std::cerr << "Error enumerating processes!" << std::endl;
//        return;
//    }
//
//    // Calculate the number of processes by dividing the bytes returned by the size of a DWORD.
//    DWORD cProcesses = cbNeeded / sizeof(DWORD);
//
//    // Dynamically allocate an array of handles to hold the processes.
//    HANDLE* hProcesses = new HANDLE[cProcesses];
//    for (DWORD i = 0; i < cProcesses; ++i) {
//        // Open a handle to each process with specific permissions.
//        hProcesses[i] = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
//    }
//
//    // Iterate over each process to gather information.
//    char szProcessName[MAX_PATH] = "";  // Buffer to store process name.
//    for (DWORD i = 0; i < cProcesses; ++i) {
//        if (!hProcesses[i]) continue;  // Skip processes we failed to open.
//
//        // Get the executable path of the process.
//        if (GetModuleFileNameExA(hProcesses[i], NULL, szProcessName, MAX_PATH)) {
//            // Extract just the executable name from the full path.
//            char* p = strrchr(szProcessName, '\\');
//            if (p) strcpy_s(szProcessName, p + 1);
//        }
//        else {
//            // If unable to get the name, use a default string.
//            strcpy_s(szProcessName, "[unknown]");
//        }
//
//        // Get the window title of the process.
//        char szWindowTitle[MAX_PATH] = "";  // Buffer to store window title.
//        HWND hWnd = GetTopWindow(NULL);  // Start with the topmost window.
//        while (hWnd != NULL) {
//            DWORD dwProcessId;
//            // Get the process ID associated with the current window.
//            GetWindowThreadProcessId(hWnd, &dwProcessId);
//            if (dwProcessId == aProcesses[i]) {
//                // If the process ID matches, retrieve the window title.
//                GetWindowTextA(hWnd, szWindowTitle, sizeof(szWindowTitle));
//                break;
//            }
//            // Move to the next window in the Z-order.
//            hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
//        }
//
//        // Write the process information to the file if a window title is found.
//        if (strlen(szWindowTitle) > 0) {
//            outFile << "NAME: " << szProcessName
//                << "\n\tTITLE: " << szWindowTitle
//                << "\n\tPID: " << aProcesses[i]
//                << "\n\tEXE: " << szProcessName
//                << "\n\n";
//        }
//
//        // Close the handle to the process.
//        CloseHandle(hProcesses[i]);
//    }
//
//    // Free the dynamically allocated memory for process handles.
//    delete[] hProcesses;
//}

std::string getNowTime() {
    time_t now = time(nullptr);  // Get the current time.
    struct tm timeInfo;

    // Convert time to a local time structure.
    if (localtime_s(&timeInfo, &now) != 0) {
        // Handle errors in retrieving local time.
        std::cerr << "Failed to get local time!" << std::endl;
        return "";
    }

    char buf[80];  // Buffer to store formatted time.
    // Format the time as "YYYYMMDDHHMMSS".
    strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", &timeInfo);
    return std::string(buf);  // Return the formatted time as a string.
}

void DesktopScreenshot::Make(const std::string& sSavePath) {
    SetProcessDPIAware();  // Adjust DPI settings for high-resolution displays.

    try {
        // Initialize the COM library for use in this thread.
        HRESULT hr = OleInitialize(NULL);
        if (FAILED(hr)) {
            std::cerr << "Error initializing COM!" << std::endl;
            return;
        }

        // Retrieve screen dimensions.
        HDC hScreen = GetDC(NULL);
        int width = GetDeviceCaps(hScreen, HORZRES);  // Screen width.
        int height = GetDeviceCaps(hScreen, VERTRES);  // Screen height.

        // Create a compatible bitmap for the screen.
        HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);

        // Create a memory device context and select the bitmap into it.
        HDC hDC = CreateCompatibleDC(hScreen);
        HGDIOBJ hOld = SelectObject(hDC, hBitmap);

        // Capture the screen contents into the bitmap.
        BitBlt(hDC, 0, 0, width, height, hScreen, 0, 0, SRCCOPY);

        // Prepare to save the bitmap as a JPEG file.
        std::ofstream outFile(sSavePath + "\\" + getNowTime() + ".jpg", std::ios_base::binary);
        if (!outFile) {
            std::cerr << "Error opening output file!" << std::endl;
            return;
        }

        // Use the OLE Picture interface to save the bitmap.
        PICTDESC pictDesc;
        pictDesc.cbSizeofstruct = sizeof(PICTDESC);
        pictDesc.picType = PICTYPE_BITMAP;
        pictDesc.bmp.hbitmap = hBitmap;
        IPicture* pPicture;
        hr = OleCreatePictureIndirect(&pictDesc, IID_IPicture, FALSE, (void**)&pPicture);

        // Create a stream to save the picture.
        LPSTREAM lpStream;
        CreateStreamOnHGlobal(NULL, TRUE, &lpStream);
        LONG cbSize = 0;
        hr = pPicture->SaveAsFile(lpStream, TRUE, &cbSize);

        // Write the stream contents to the output file.
        BYTE* pBuffer = new BYTE[cbSize];
        LARGE_INTEGER liSeekPos = { 0 };
        lpStream->Seek(liSeekPos, STREAM_SEEK_SET, NULL);
        ULONG nBytesRead;
        lpStream->Read(pBuffer, cbSize, &nBytesRead);
        outFile.write((char*)pBuffer, nBytesRead);

        // Cleanup dynamically allocated memory and resources.
        delete[] pBuffer;
        outFile.close();
        pPicture->Release();
        lpStream->Release();
        SelectObject(hDC, hOld);
        DeleteDC(hDC);
        ReleaseDC(NULL, hScreen);
        DeleteObject(hBitmap);

        // Uninitialize the COM library.
        OleUninitialize();
    }
    catch (const std::exception& ex) {
        // Handle exceptions and log the error message.
        std::cerr << "DesktopScreenshot >> Failed to create\n" << ex.what() << std::endl;
    }
}

