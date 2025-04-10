#pragma once
#include <string>

class Clipboard
{
public:
	static std::string GetText();
	static void SetText(std::string text);
};
