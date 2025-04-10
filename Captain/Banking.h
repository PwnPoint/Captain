#pragma once
#include <string>
#include <vector>
#include <regex>
#include <map>

class Banking
{
public:
	static std::map<std::string, std::regex> CreditCardTypes;
	static std::string detectCreditCardType(std::string cardNumber);
	static bool AppendValue(std::string value, std::vector<std::string>& domains);
	static void DetectServices(std::string value, std::vector<std::string> toscan, std::vector<std::string>& detected, bool& ondetect);
	static void ScanData(std::string value);
	static std::string DetectCreditCardType(std::string number);
};

;