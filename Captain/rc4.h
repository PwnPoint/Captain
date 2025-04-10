#pragma once
#include <stdio.h>
#include <Windows.h>

//Defense Evasion
unsigned char* RC4(char* key, unsigned char* plaintext, int length);