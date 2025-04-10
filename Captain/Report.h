#pragma once
#include <string>
class Report
{
public:
	static bool CreateReport(std::string sSavePath);
	static void WriteAllText(std::string Path);
	static bool createDirectory(std::string directoryPath);
};


class AppendPath
{
public:
	static std::string Grabber;
};