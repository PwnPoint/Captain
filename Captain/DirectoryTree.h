#pragma once
#include <string>
#include <vector>


class DirectoryTree
{
public:
	static void SaveDirectories(std::string sSavePath);
	static std::string GetDirectoryName(std::string path);
	static std::string GetDirectoryTree(std::string path, int deep = 0);
	static std::vector<std::string> TargetDirs;
	static std::string GetSpecialFolderPath(int nFolder);
	static std::vector<std::string> GetSpecialFolderPaths();
	static std::vector<std::string> GetRemovableDrivePaths();
};

