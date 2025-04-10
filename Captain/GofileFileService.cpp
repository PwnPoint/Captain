#include "GofileFileService.h"
#include "DiscordHook.h"
#include <iostream>
#include <Windows.h>
#include <winhttp.h>
#include <regex>
#include <string>
#include <iostream>
#include <fstream>

#pragma comment(lib, "winhttp.lib")

std::string GofileFileService::UploadFileAsync(std::string& path)
{
    // Declare necessary WinHTTP handles
    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;

    // Declare variables for data size and downloaded data
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer = NULL;
    BOOL bResults = FALSE;

    // Get the Gofile URL from DiscordWebHook (assuming it's a function that provides the URL)
    std::wstring GofileUrl = DiscordHook::getBestgofile();

    // Convert the file path to a C-style string for API usage
    const char* pszFileName = path.c_str();

    // Open a WinHTTP session
    hSession = WinHttpOpen(L"WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession)
    {
        std::cerr << "Failed to open WinHTTP session." << std::endl;
        return "";
    }

    // Establish a connection to the Gofile server
    hConnect = WinHttpConnect(hSession, GofileUrl.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect)
    {
        std::cerr << "Failed to connect to server." << std::endl;
        WinHttpCloseHandle(hSession);
        return "";
    }

    // Create the HTTP POST request to upload the file
    hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/uploadFile", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest)
    {
        std::cerr << "Failed to create HTTP request." << std::endl;
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    // Prepare multipart/form-data headers and body
    std::string strHeaders = "Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW";
    std::string strBodyStart = "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"file\"; filename=\"";
    strBodyStart += pszFileName;
    strBodyStart += "\"\r\nContent-Type: application/octet-stream\r\n\r\n";
    std::string strBodyEnd = "\r\n------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";

    // Open the file for reading in binary mode
    HANDLE hFile = CreateFileA(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Failed to open file." << std::endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    // Get the file size and prepare a buffer to read the file into memory
    DWORD dwFileSize = GetFileSize(hFile, NULL);
    DWORD dwBytesRead = 0;
    LPBYTE pFileBuffer = new BYTE[dwFileSize];

    // Read the file into the buffer
    if (!ReadFile(hFile, pFileBuffer, dwFileSize, &dwBytesRead, NULL))
    {
        std::cerr << "Failed to read file." << std::endl;
        CloseHandle(hFile);
        delete[] pFileBuffer;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    CloseHandle(hFile);  // Close the file handle after reading

    // Prepare the complete request buffer combining headers, file data, and the multipart boundary
    DWORD dwBufferLength = strBodyStart.length() + dwFileSize + strBodyEnd.length();
    BYTE* pRequestBuffer = new BYTE[dwBufferLength];
    memcpy(pRequestBuffer, strBodyStart.c_str(), strBodyStart.length());
    memcpy(pRequestBuffer + strBodyStart.length(), pFileBuffer, dwFileSize);
    memcpy(pRequestBuffer + strBodyStart.length() + dwFileSize, strBodyEnd.c_str(), strBodyEnd.length());

    // Convert the headers to wide characters for the WinHTTP function
    int nSize = MultiByteToWideChar(CP_UTF8, 0, strHeaders.c_str(), -1, NULL, 0);
    if (nSize == 0)
    {
        std::cerr << "Failed to convert string." << std::endl;
        return "";
    }

    // Allocate memory for the wide-character headers
    wchar_t* pwszStr = new wchar_t[nSize];
    MultiByteToWideChar(CP_UTF8, 0, strHeaders.c_str(), -1, pwszStr, nSize);

    LPCWSTR lpwStr = pwszStr;

    // Send the HTTP request with the file data
    bResults = WinHttpSendRequest(hRequest, lpwStr, strHeaders.length(), pRequestBuffer, dwBufferLength, dwBufferLength, 0);
    if (!bResults)
    {
        std::cerr << "Failed to send HTTP request." << std::endl;
        delete[] pRequestBuffer;
        delete[] pFileBuffer;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    // Receive the HTTP response
    bResults = WinHttpReceiveResponse(hRequest, NULL);
    if (!bResults)
    {
        std::cerr << "Failed to receive HTTP response." << std::endl;
        delete[] pRequestBuffer;
        delete[] pFileBuffer;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    // Query the HTTP status code to ensure the request was successful
    DWORD dwStatusCode = 0;
    dwSize = sizeof(dwStatusCode);
    bResults = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwStatusCode, &dwSize, NULL);
    if (!bResults)
    {
        std::cerr << "Failed to get HTTP status code." << std::endl;
        delete[] pRequestBuffer;
        delete[] pFileBuffer;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    // Check if there's response data to read
    do
    {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
        {
            std::cerr << "Failed to query available data." << std::endl;
            delete[] pRequestBuffer;
            delete[] pFileBuffer;
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return "";
        }

        // Allocate memory for the response data buffer
        pszOutBuffer = new char[dwSize + 1];
        if (!pszOutBuffer)
        {
            std::cerr << "Failed to allocate memory." << std::endl;
            delete[] pRequestBuffer;
            delete[] pFileBuffer;
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return "";
        }

        ZeroMemory(pszOutBuffer, dwSize + 1);

        // Read the data into the buffer
        if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
        {
            std::cerr << "Failed to read data." << std::endl;
            delete[] pszOutBuffer;
            delete[] pRequestBuffer;
            delete[] pFileBuffer;
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return "";
        }

        // Convert the response data to a string
        std::string response = pszOutBuffer;

        // Use regex to find a Gofile URL in the response
        const std::regex url_regex("https://gofile\\.io/d/\\w+");
        std::smatch match;
        if (std::regex_search(response, match, url_regex)) {
            return match.str();  // Return the matched URL
        }

        std::cout << pszOutBuffer;  // Output the response to the console
        delete[] pszOutBuffer;  // Clean up the buffer

        return "";  // Return an empty string if no URL is found

    } while (dwSize > 0);  // Loop while there's data available

    // Clean up all resources
    delete[] pRequestBuffer;
    delete[] pFileBuffer;
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return 0;  // Return 0 if all steps are successful
}
