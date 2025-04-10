#include "InstalledApps.h"
#include <iostream>
#include <Windows.h>
#include <Msi.h>
#include <fstream>
#include <iomanip>
#include <string>

#pragma comment(lib,"msi.lib")

void InstalledApps::WriteAppsList(std::string sSavePath)
{
    // Create the full path to the file where the apps list will be saved
    std::string filePath = sSavePath + "\\Software_Info.txt";

    // Open a file stream in output mode to write to the file
    std::ofstream file(filePath);

    // Configure the file stream for left alignment (applies to output formatting)
    file << std::left;

    // Write the column headers for the data table into the file
    // These headers include: APP, VERSION, INSTALL DATE, IDENTIFYING NUMBER
    // Each column is given a specific width using `std::setw` for proper alignment
    file << std::setw(80) << "APP"
        << std::setw(20) << "VERSION"
        << std::setw(20) << "INSTALL DATE"
        << std::setw(20) << "IDENTIFYING NUMBER"
        << std::endl;

    // Add a blank line for readability
    file << std::endl;

    // Check if the file was successfully opened
    if (file.is_open())
    {
        // Initialize a counter to keep track of the product index
        DWORD index = 0;

        // Buffer to hold the product code, which is a unique identifier for each product
        char productCode[39]; // Product codes are GUIDs, so they need to be 39 characters long (including null terminator)

        // Loop through all installed products using `MsiEnumProductsExA`
        // Continue as long as `MsiEnumProductsExA` returns `ERROR_SUCCESS`
        while (MsiEnumProductsExA(NULL, NULL, MSIINSTALLCONTEXT_ALL, index, productCode, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            // Increment the index for the next product
            ++index;

            // Buffers to store information about the current product
            char productName[256];  // Buffer for the product name
            char productVersion[256];  // Buffer for the version string
            char installDate[32];  // Buffer for the installation date
            DWORD buflen;  // Variable to hold the size of the buffers

            // Retrieve the product name and store it in `productName`
            buflen = sizeof(productName);
            MsiGetProductInfoA(productCode, INSTALLPROPERTY_PRODUCTNAME, productName, &buflen);

            // Retrieve the product version and store it in `productVersion`
            buflen = sizeof(productVersion);
            MsiGetProductInfoA(productCode, INSTALLPROPERTY_VERSIONSTRING, productVersion, &buflen);

            // Retrieve the installation date and store it in `installDate`
            buflen = sizeof(installDate);
            MsiGetProductInfoA(productCode, INSTALLPROPERTY_INSTALLDATE, installDate, &buflen);

            // Write the retrieved product information into the file in a formatted table
            // Each field is aligned with a fixed column width
            file << std::setw(80) << productName
                << std::setw(20) << productVersion
                << std::setw(20) << installDate
                << std::setw(20) << productCode
                << std::endl;
        }

        // Close the file stream to save changes and release resources
        file.close();
    }

    // Function returns, no explicit return value as it is void
    return;
}
