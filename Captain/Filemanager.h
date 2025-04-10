#pragma once
#include <string>

class Filemanager
{
public:
    static void DeleteFileOrDir(std::string path);
    static void CopyDirectoryMultiByte(const std::string& source_dir, const std::string& dest_dir);
    static double DirectorySize(const std::string& path);
    static bool zip_add_to_archive(std::string& path, bool password = false);
    static void writeFileSystemInfo(std::string& path);
    static bool killprocess(const std::string& process_name);
};

