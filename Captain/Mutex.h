#pragma once
#include <Windows.h>
class Mutex
{
public:
	void Check();
private:
	HANDLE hMutex;
};