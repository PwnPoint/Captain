#pragma once
#include <string>
#include <vector>

typedef void (*Action)();
class WindowManager
{
public:
	static void myThreadFunc();
	static std::string GetActiveWindowTitle();
	static void Run();
	static std::vector<Action> functions;
};

