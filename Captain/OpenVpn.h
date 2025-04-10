#pragma once
#include <string>

class OpenVpn
{
public:
	static void Save(const std::string destinationDir);
	static void copyOVPNFiles(const std::string& sourceDir, const std::string& destinationDir);
};

