#pragma once
#include <string>


struct Password {
	std::string Website;
	std::string LoginUrl;
	std::string Username;
	std::string Pass;
};

struct Site {
	std::string Url;
	std::wstring Title;
	int Count;
};

struct Download {
	std::string Tag_Url;
	std::string Target_Path;

};


struct CreditCard {
	std::string Number;
	std::string ExpYear;
	std::string ExpMonth;
	std::string Name;
};

struct Cookie {
	std::string HostKey;
	std::string Name;
	std::string Path;
	std::string ExpiresUtc;
	std::string encrypted_value;
	std::string IsSecure;
};


struct AutoFill {
	std::string Name;
	std::string Value;
};


struct Bookmark
{
	std::string Url;
	std::string Title;
};
