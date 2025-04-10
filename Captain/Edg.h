#pragma once
#include <string>

class Edg
{
public:
	static std::string Replace(std::string str, std::string substr, std::string newsubstr);
	static std::string FindSubstring(std::string str);
	static std::string BrowserPathToAppName(std::string sLoginData);
	static bool PathExists(const std::string& path);
	static void Run(std::string sSavePath);
};


