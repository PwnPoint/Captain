#pragma once
#include <string>

class ClipboardManager
{
public:
	static std::string _prevClipboard;
	static std::string ClipboardText;
	static void Run();
};

