
#include "winsock2.h"
#include "Logger.hpp"
#include "HttpManager.hpp"


std::ofstream cHttpManager::pagefile;

cHttpManager::cHttpManager()
{

}

cHttpManager::~cHttpManager()
{
	curl_easy_cleanup(curl_handle);
	curl_slist_free_all(headers);
	curl_global_cleanup();
}

void cHttpManager::AddHeader(std::string pStrHeader, bool ClearAllPrevious)
{
	if (ClearAllPrevious)ClearHeaders();
	headers = curl_slist_append(headers, pStrHeader.c_str());
}

void cHttpManager::ClearHeaders()
{
	curl_slist_free_all(headers);
	InitHeaders(&headers);
}

BackObject cHttpManager::DoGet(std::string pUrl)
{
	BackObject back;
	//std::cout << pUrl.c_str() << std::endl;
	if(pUrl.empty())
	{
		back.ErrDesc = "Url string is empty";
		back.Success = false;
		return back;
	}
	readBuffer.clear();
	headerBuffer.clear();
	curl_easy_reset(curl_handle);
	res = curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1L);
	res = curl_easy_setopt(curl_handle, CURLOPT_URL, pUrl.c_str());
	res = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
	res = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &readBuffer);
	res = curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
	//res = curl_easy_setopt(curl_handle, CURLOPT_HEADER, 1L);
	res = curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
	res = curl_easy_setopt(curl_handle, CURLOPT_ACCEPT_ENCODING, "");
	res = curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	res = curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, header_callback);
	res = curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, &headerBuffer);
	res = curl_easy_setopt(curl_handle, CURLOPT_BUFFERSIZE, 2200000L);
	//res = curl_easy_setopt(curl_handle, CURLOPT_PATH_AS_IS, 1L);

#ifdef _DEBUG
	res = curl_easy_setopt(curl_handle, CURLOPT_DEBUGFUNCTION, my_trace);
	res = curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
#endif // DEBUG


	try
	{
		res = curl_easy_perform(curl_handle);
	}
	catch (const std::exception& e)
	{
		back.ErrDesc.append(e.what());
		back.Success = false;
		return back;
	}
	
	if (res != CURLE_OK)
	{
		back.ErrDesc = (curl_easy_strerror(res));
		back.Success = false;
	}

	long rvcode = 0;
	res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &rvcode);
	if (!(rvcode == 200 && rvcode != CURLE_ABORTED_BY_CALLBACK))
	{
		back.ErrDesc.append(" Http Response Code : " + std::to_string(rvcode) + " Header : " + (headerBuffer));
		back.Success = false;
	}

	back.StrValue = readBuffer;
	Sleep(500);
	return back;
}

BackObject cHttpManager::DoGetWritefile(std::string pUrl, std::string pFile)
{
	BackObject back;
	if(pUrl.empty())
	{
		back.ErrDesc = "Url string is empty";
		back.Success = false;
		return back;
	}
	curl_easy_reset(curl_handle);
	pagefile.open(pFile.c_str(), std::ios::binary);
	if (!pagefile)
	{
		back.ErrDesc.append("Cannot open destination file " + (pFile));
		back.Success = false;
		pagefile.clear();

		return back;
	}

	Logger::WriteLog("Downloading file: " + pFile);

	res = curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1L);
	res = curl_easy_setopt(curl_handle, CURLOPT_URL, pUrl.c_str());
	res = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeFile);
	res = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&pagefile));
	res = curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0);
	res = curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
	res = curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
	res = curl_easy_setopt(curl_handle, CURLOPT_ACCEPT_ENCODING, "");
	res = curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	//res = curl_easy_setopt(curl_handle, CURLOPT_BUFFERSIZE, 2200000L);



	res = curl_easy_perform(curl_handle);
	if (res != CURLE_OK)
	{
		back.ErrDesc = (curl_easy_strerror(res));
		back.Success = false;
	}
	
	//fclose(pagefile);
	pagefile.flush();
	pagefile.close();
	//pagefile.clear();

	long rvcode = 0;
	res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &rvcode);
	if (!(rvcode == 200 && rvcode != CURLE_ABORTED_BY_CALLBACK))
	{
		back.ErrDesc.append(" Http Response Code : " + std::to_string(rvcode));
		back.Success = false;
	}

	return back;
}

BackObject cHttpManager::DoPost(std::string pUrl, std::string pPostFields)
{
	//curl_easy_setopt(pCurl, CURLOPT_HEADER, 0L);  //When enabled, the header file information will be output as a data stream
	//curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);//Allow redirection
	//curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);

	BackObject back;
	if(pUrl.empty())
	{
		back.ErrDesc = "Url string is empty";
		back.Success = false;
		return back;
	}
	readBuffer.clear();
	curl_easy_reset(curl_handle);
	res = curl_easy_setopt(curl_handle, CURLOPT_URL, pUrl.c_str());
	res = curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
	res = curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &readBuffer);
	res = curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
	res = curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
	res = curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
	res = curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, pPostFields.c_str());
	res = curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDSIZE, -1L);
	res = curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	res = curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, header_callback);
	res = curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, &headerBuffer);
	res = curl_easy_setopt(curl_handle, CURLOPT_BUFFERSIZE, 2200000L);
	res = curl_easy_setopt(curl_handle, CURLOPT_PATH_AS_IS, 1L);
#ifdef _DEBUG
	res = curl_easy_setopt(curl_handle, CURLOPT_DEBUGFUNCTION, my_trace);
	res = curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
#endif // DEBUG
	res = curl_easy_perform(curl_handle);
	if (res != CURLE_OK)
	{
		back.ErrDesc = (curl_easy_strerror(res));
		back.Success = false;
	}

	long rvcode = 0;
	res = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &rvcode);
	if (!(rvcode == 200 && rvcode != CURLE_ABORTED_BY_CALLBACK))
	{
		back.ErrDesc.append(" Http Response Code : " + std::to_string(rvcode) + " Header : " + (headerBuffer));
		back.Success = false;
	}

	back.StrValue = readBuffer;
	Sleep(500);
	return back;
}

std::string cHttpManager::UrlEncode(std::string pstr)
{
	std::string back = "";
	char* nstr = curl_easy_escape(curl_handle, pstr.c_str(), pstr.length());
	if (nstr == nullptr) return back;
	back.append(nstr);
	return back;
}

std::string cHttpManager::UrlDecode(std::string pstr)
{
	std::string back = "";
	char* nstr = curl_easy_unescape(curl_handle, pstr.c_str(), pstr.length(),nullptr);
	if (nstr == nullptr) return back;
	back.append(nstr);
	return back;
}

size_t cHttpManager::WriteCallback(char* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

size_t cHttpManager::header_callback(char* buffer, size_t size, size_t nitems, void* userdata)
{
	/* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */
	/* 'userdata' is set with CURLOPT_HEADERDATA */
	((std::string*)userdata)->append((char*)buffer, size * nitems);
	return nitems * size;
}

size_t cHttpManager::writeFile(void* ptr, size_t size, size_t nmemb, void* stream)
{
	//size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
	//return written;

	size_t realsize = size * nmemb;
	auto file = reinterpret_cast<std::ofstream*>(stream);
	file->write(reinterpret_cast<const char*>(ptr), realsize);
	return realsize; 
}

void cHttpManager::InitHeaders(curl_slist** pheaders)
{
	*pheaders = curl_slist_append(*pheaders, NULL);
}

void cHttpManager::dumpp(const char* text,	FILE* stream, unsigned char* ptr, size_t size, std::string& ss)
{
	size_t i;
	size_t c;
	unsigned int width = 0x10;

	ss.append(text);
	ss.append(" -- ");
	//fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n", text, (long)size, (long)size);

	for (i = 0; i < size; i += width) {
	//	fprintf(stream, "%4.4lx: ", (long)i);

		/* show hex to the left */
		/*
		for (c = 0; c < width; c++) {
			if (i + c < size)
				fprintf(stream, "%02x ", ptr[i + c]);
			else
				fputs("   ", stream);
		}
		*/

		/* show data on the right */
		for (c = 0; (c < width) && (i + c < size); c++) {
			char x = (ptr[i + c] >= 0x20 && ptr[i + c] < 0x80) ? ptr[i + c] : '.';
			//fputc(x, stream);
			ss.push_back(x);
		}

		//fputc('\n', stream); /* newline */
	}
}

int cHttpManager::my_trace(CURL* handle, curl_infotype type, char* data, size_t size,void* userp)
{
	const char* text;
	(void)handle; /* prevent compiler warning */
	(void)userp;

	switch (type) {
	case CURLINFO_TEXT:
	{
		//fprintf(stderr, "== Info: %s", data);
		std::string aa(data);
		Logger::WriteLog(aa);
	}
		
	default: /* in case a new one is introduced to shock us */
		return 0;

	case CURLINFO_HEADER_OUT:
		text = "=> Send header";
		break;
	case CURLINFO_DATA_OUT:
		text = "=> Send data";
		break;
	case CURLINFO_SSL_DATA_OUT:
		text = "=> Send SSL data";
		break;
	case CURLINFO_HEADER_IN:
		text = "<= Recv header";
		break;
	case CURLINFO_DATA_IN:
		text = "<= Recv data";
		break;
	case CURLINFO_SSL_DATA_IN:
		text = "<= Recv SSL data";
		break;
	}
	std::string strback;
	dumpp(text, stderr, (unsigned char*)data, size, strback);
	
	//std::wstring back = Globals::utf8_toWstring(strback);
	Logger::WriteLog(strback);
	return 0;
}

void cHttpManager::Initiliaze()
{
	curl_handle = nullptr;
	res = CURLE_OK;
	headers = nullptr;
	res = curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	InitHeaders(&headers);
}