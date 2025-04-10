#pragma once
#include <string>
#include <map>
#include <regex>

class Clipper
{
public:
	static bool Replace();
	static std::map<std::string, std::regex> PatternsList;
};

