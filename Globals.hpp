#pragma once
#include <windows.h>
#include <string>
#include <codecvt>
#include <locale>
#include <ctype.h>
#include <vector>

class Globals
{
public:

	static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8conv;
	static std::wstring to_wstring(std::string const& s);
	static std::string to_string(std::wstring const& s);
	static std::string to_lower(std::string const& s);

	static std::string utf8_to_string(const char *utf8str, const std::locale& loc);
	static std::string UrlDecode(const std::string &src);
	static std::string UrlEncode(const std::string &src);
	static char hexToChar(char first, char second);
	static std::string charToHex(char c);
	static std::string replaceStringAll(std::string str, const std::string &replace, const std::string &with);
	static std::string tmToStr(tm* ptm);
	static void strToTm(tm* ptm, std::string pstr);
	static void splitString(std::string pstr, std::string pdelimiter, std::vector<std::string>* strvec);
	static std::string joinString(std::string pdelimiter, std::vector<std::string>* strvec);

	static std::size_t GetCharLen(std::string& pstr);
	static std::size_t GetFirstIndexOf(std::string& pstr, std::string& psrch);
	static std::size_t GetLastIndexOf(std::string& pstr, std::string& psrch);
	static std::string GetSubString(std::string& pstr, std::size_t  pbegin, std::size_t  plength = std::string::npos);

	static std::string GetNowStr();

};

class BackObject
{
public:
	bool Success = true;
	std::string ErrDesc = "";
	std::string StrValue = "";
	std::wstring GetWStrValue();
};