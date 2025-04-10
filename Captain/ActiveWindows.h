#pragma once
#include <windows.h>
#include <Psapi.h>
#include <fstream>
#include <iostream>


class ActiveWindows
{
public:
	static void WriteWindows(const std::string& sSavePath);
};

