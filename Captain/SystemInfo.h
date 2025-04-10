#pragma once

#include <string>
using namespace std;

class SystemInfo
{
public:
	static std::string getComputerName();
	static std::string getDateTime();
	static std::string getUserName();
	static std::string getLanguage();
	static std::string getScreenMetrics();
	static std::string getBattery();
	static std::string getWindowsVersionName();
	static std::string getBitVersion();
	static std::string getSystemVersion();
	static std::string getDefaultGateway();
	static std::string getAntivirus();
	static std::string getLocalIp();
	static std::string getPublicIpAsync();
	static std::string getCpuName();
	static std::string getGpuName();
	static std::string getRamAmount();
};

