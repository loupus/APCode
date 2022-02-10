#pragma once

//#pragma comment(lib, "ws2_32.lib")
#include "winsock2.h"

#include "curl\curl.h"
#include "Globals.hpp"

class cHttpManager
{
public:
	cHttpManager();
	~cHttpManager();
	void AddHeader(std::string pStrHeader, bool ClearAllPrevious = false);
	void ClearHeaders();
	BackObject DoGet(std::string pUrl);
	BackObject DoGetWritefile(std::string pUrl, std::string pFile);
	BackObject DoPost(std::string pUrl, std::string pPostFields);
	std::string UrlEncode(std::string pstr);
	std::string UrlDecode(std::string pstr);
	void Initiliaze();

private:
	CURL *curl_handle;
	CURLcode res;
	struct curl_slist *headers;
	void InitHeaders(struct curl_slist **pheaders);
	std::string readBuffer;
	std::string headerBuffer;
	static std::ofstream pagefile;

	static size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp);
	static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata);
	static int my_trace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp);
	static void dumpp(const char *text, FILE *stream, unsigned char *ptr, size_t size, std::string &ss);
	static size_t writeFile(void *ptr, size_t size, size_t nmemb, void *stream);
};
