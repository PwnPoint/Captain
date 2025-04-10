#include "registry.h"  // Includes the header file which contains the declarations for registry manipulation functions.

int UpdateRegistry(HKEY HKey, char* Key, char* ValueName, char* Data, DWORD Len, DWORD Type, bool CreateNew)
{
    LSTATUS ret = 0;  // Variable to store the status of registry operations. LSTATUS is typically used for status codes returned by Windows API registry functions.
    HKEY OpenKey = NULL;  // HKEY handle that will represent the opened or created registry key.

    // Check if we are opening an existing key or creating a new one.
    if (!CreateNew)
        // If CreateNew is false, we attempt to open an existing registry key using RegOpenKeyEx.
        ret = (int)RegOpenKeyEx(HKey, Key, 0, KEY_ALL_ACCESS, &OpenKey);
    else
        // If CreateNew is true, we attempt to create a new registry key using RegCreateKey.
        ret = (int)RegCreateKey(HKey, Key, &OpenKey);

    // If the key opening/creation operation failed (i.e., ret != 0), return the error code.
    if (ret != 0)
        return ret;

    // Set the value in the registry for the specified key.
    // RegSetValueEx sets a value for the open registry key, assigning it the provided data.
    // Parameters:
    // - OpenKey: handle to the open key.
    // - ValueName: the name of the value to set (can be null if setting the default value).
    // - Type: type of data to store (e.g., REG_SZ for string, REG_DWORD for number).
    // - Data: actual data to be stored in the value.
    // - Len: size of the data to be stored.
    ret = RegSetValueEx(OpenKey, ValueName, NULL, Type, (BYTE*)Data, Len);

    // If setting the value failed, return the error code.
    if (ret != 0)
        return ret;

    // If everything is successful, the function will implicitly return 0 (success).
}