#pragma once
#include <string>

class ClipperEventManager
{
public:
	static std::string GetActiveWindows();
	static bool Detect();
	static bool Action();
	static bool find_to_str(const std::string& str, const std::string& pattern);
};

