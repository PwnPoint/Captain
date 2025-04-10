#pragma once
#include <string>
#include <vector>

class Wifi
{
public:
	static std::string RunCommand(const std::string& command);
	static std::vector<std::string> SplitString(const std::string& str, const std::string& delim);
	static std::vector<std::string> GetProfiles(const std::string& output);
	static std::string GetPassword(const std::string& profile);
	static void ScanningNetworks(std::string sSavePath);
	static void SavedNetworks(std::string sSavePath);
	static void WriteProductKey(std::string Path);
};

