#include "ProductKey.h"
#include <iostream>
#include <windows.h>
#include "Counter.h"

std::string ProductKey::GetWindowsProductKeyFromRegistry()
{
    // Declare a handle for the registry key.
    HKEY hKey;

    // Specify the registry subkey path that holds the Windows product key.
    LPCSTR lpSubKey = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SoftwareProtectionPlatform";

    // Define the options for opening the registry key (not used here, set to 0).
    DWORD dwOptions = 0;

    // Define the desired access rights to the registry key (KEY_QUERY_VALUE means read-only access).
    REGSAM samDesired = KEY_QUERY_VALUE;

    // Declare a variable to hold the result of the registry function calls.
    LONG lResult;

    // Open the registry key using the specified subkey and access rights.
    lResult = RegOpenKeyExA(HKEY_LOCAL_MACHINE, lpSubKey, dwOptions, samDesired, &hKey);

    // Check if opening the key was successful.
    if (lResult != ERROR_SUCCESS) {
        // If it failed, print an error message and return an empty string.
        std::cout << "Failed to open key." << std::endl;
        return "";
    }

    // Define a buffer size to hold the registry value name (1024 characters).
    const int bufferSize = 1024;

    // Declare an array to store the name of the registry value.
    char valueName[bufferSize];

    // Set the size of the value name buffer.
    DWORD valueNameSize = bufferSize;

    // Declare a variable to store the type of the registry value.
    DWORD valueType;

    // Declare a buffer to store the data read from the registry.
    BYTE buffer[bufferSize];

    // Set the initial size of the buffer.
    DWORD bufferLength = bufferSize;

    // Query the registry key for the value named "BackupProductKeyDefault", which stores the product key.
    lResult = RegQueryValueExA(hKey, "BackupProductKeyDefault", NULL, &valueType, buffer, &bufferLength);

    // Check if the query was successful.
    if (lResult != ERROR_SUCCESS) {
        // If the query failed, close the registry key and print an error message.
        RegCloseKey(hKey);
        std::cout << "Failed to get the value of the 'BackupProductKeyDefault' registry key." << std::endl;
        return "";
    }

    // Check if the retrieved registry value is a string (REG_SZ type).
    if (valueType == REG_SZ) {
        // If it is a string, interpret the buffer as a string and return it.
        std::string productKey(reinterpret_cast<char*>(buffer), bufferLength);

        // Set a counter flag (this part seems to be for internal use).
        Counter::ProductKey = true;

        // Return the product key as a string.
        return productKey;
    }
    else {
        // If the registry value is not a string, do nothing (you can handle this case as needed).
    }

    // Close the registry key after processing.
    RegCloseKey(hKey);

    // If something goes wrong, return an empty string.
    return "";
}
