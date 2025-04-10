#pragma once

#include <vector>
#include <string>

class Wallets
{
public:
    static void getWallets(const std::string& saveDir);
public:
    static const std::vector<std::vector<std::string>> walletsDirectories_;
    static const std::vector<std::string> walletsRegistry_;
    static void copyWalletFromDirectoryTo(const std::string& saveDir, const std::string& walletDir, const std::string& walletName);
    static void copyWalletFromRegistryTo(const std::string& saveDir, const std::string& walletRegistry);
};

