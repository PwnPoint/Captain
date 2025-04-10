#pragma once
#include "Config.h"
#include <string>
#include <vector>

class FileGrabber
{
public:
	static std::string _savePath;
	static std::vector<std::string> targetDirs;
	static void Run(std::string savePath);
public:
	static void grabDirectory(const std::string& dir, std::string& savePath, int depth, int max_depth = 2);
	static void GrabFile(const std::string& filePath, const std::string& savePath);
	static std::string DetectFileType(const std::string& extensionName);
	static std::string RecordFileType(Config::FileType fileType);
	static std::vector<std::string> GetSpecialFolderPaths();
	static std::string GetSpecialFolderPath(int nFolder);
};





