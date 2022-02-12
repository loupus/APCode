#pragma once
#include <ctime>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include "Globals.hpp"

enum AssetState
{
	astat_NONE = 0,
	astat_DOCUMENT_GET,
	astat_VIDEO_DOWNLOADED,
	astat_VIDEO_CONVERTED,
	astat_PROXY_SENT,
	astat_PROXY_CREATED,
	astat_MOVED_2_INGEST,
	astat_UPDATED
};

enum AssetSuccess
{
	asSuc_NoProblem = 0,
	asSuc_Completed = 1,
	asSuc_Failed = 2
};

enum Agencies
{
	a_none = 0,
	a_aa = 1,
	a_dha = 2,
	a_reuters = 3,
	a_aptn = 4
};

enum MediaTypes
{
	mt_none = 0,
	mt_text = 1,
	mt_video = 2
};

class AssetTime
{
private:
	time_t localtimet = 0;

public:
	static const int gmtdiff = 10800; // türkiye için 3 saat: 60*60*3

	AssetTime() = default;

	// copy constructors
	AssetTime(const AssetTime &other)
	{
		localtimet = other.localtimet;
	}

	AssetTime(const time_t &other)
	{
		localtimet = other;
	}
	//////////////////////////////////////

	// assignment operators;
	AssetTime &operator=(AssetTime other)
	{

		std::swap(localtimet, other.localtimet);
		return *this;
	}

	AssetTime &operator=(time_t other)
	{
		localtimet = other;
		return *this;
	}
	/////////////////////////////////

	std::string asString()
	{
		std::tm *tmpFirstTime = std::localtime(&localtimet);
		std::ostringstream dtss;
		dtss << std::put_time(tmpFirstTime, "%Y-%m-%d %H:%M:%S");
		return dtss.str();
	}

	std::string asTString()
	{
		std::tm *tmpFirstTime = std::gmtime(&localtimet);
		std::ostringstream dtss;
		dtss << std::put_time(tmpFirstTime, "%Y-%m-%dT%H:%M:%SZ");
		return dtss.str();
	}

	bool fromString(const char *pStr)
	{
		if (pStr == nullptr)
			return false;
		time_t tmptime;
		std::tm t;
		std::istringstream ss(pStr);
		ss >> std::get_time(&t, "%Y-%m-%d %H:%M:%S");
		tmptime = mktime(&t);
		if (tmptime != -1)
		{
			localtimet = tmptime;
			return true;
		}
		else
			return false;
	}

	bool fromTString(const char *pStr)
	{
		if (pStr == nullptr)
			return false;
		time_t tmptime;
		std::tm t;
		std::istringstream ss(pStr);
		ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%SZ");
		t.tm_hour += 3; // anadolu ajansı gmt 0 çalışıyor
		tmptime = mktime(&t);
		if (tmptime != -1)
		{
			localtimet = tmptime;
			return true;
		}
		else
			return false;
	}

	time_t GetTime(bool isLocal = true)
	{
		if (isLocal)
			return localtimet;
		else
			return localtimet - gmtdiff;
	}

	void SetTime(time_t pTime, bool isLocal = true)
	{
		if (isLocal)
			localtimet = pTime;
		else
			localtimet = pTime + gmtdiff;
	}

	void AddTime(time_t pTime)
	{
		localtimet += pTime;
	}
};

class cAsset
{
public:
	cAsset()
	{
		State = AssetState::astat_NONE;
		FirstTime = time(0);
		LastTime = time(0);
		OnDate = time(0);
		AgencySource = static_cast<int>(Agencies::a_none);
		MediaType = static_cast<int>(MediaTypes::mt_none);
		State = static_cast<int>(AssetState::astat_NONE);
		Success = static_cast<int>(AssetSuccess::asSuc_NoProblem);
		IsDeleted = false;
	}

	~cAsset()
	{
	}

	std::string Id;
	std::string GroupId;
	int AgencySource;
	int MediaType;
	AssetTime OnDate;
	AssetTime FirstTime;
	AssetTime LastTime;
	int State;
	int Success;
	std::string MediaFile;
	int MediaFileSize;
	std::string MediaPath;
	std::string Body;
	std::string HeadLine;
	std::string ErrMessage;
	std::string ProxyFile;
	std::string Language;
	bool IsDeleted;

	std::string itemUrl;
	std::string bodyLink;
	std::string videoLink;

	std::string toJsonStr()
	{
		nlohmann::json j = nlohmann::json{
			{"assetid", this->Id},
			{"assetsource", this->AgencySource},
			{"headline", this->HeadLine},
			{"body", this->Body},
			{"mediatype", this->MediaType},
			{"mediafile", this->MediaFile},
			{"mediapath", this->MediaPath},
			{"assetstate", this->State},
			{"assetsuccess", this->Success},
			{"firsttime", this->FirstTime.asString()},
			{"lasttime", this->LastTime.asString()},
			{"ondate", this->OnDate.asString()},
			{"errmessage", this->ErrMessage},
			{"proxyfile", this->ProxyFile},
			{"itemurl", this->itemUrl}};

		return j.dump();
	}

	nlohmann::json toJson()
	{
		nlohmann::json j = nlohmann::json{
			{"assetid", this->Id},
			{"assetsource", this->AgencySource},
			{"headline", this->HeadLine},
			{"body", this->Body},
			{"mediatype", this->MediaType},
			{"mediafile", this->MediaFile},
			{"mediapath", this->MediaPath},
			{"assetstate", this->State},
			{"assetsuccess", this->Success},
			{"firsttime", this->FirstTime.asString()},
			{"lasttime", this->LastTime.asString()},
			{"ondate", this->OnDate.asString()},
			{"errmessage", this->ErrMessage},
			{"proxyfile", this->ProxyFile},
			{"itemurl", this->itemUrl}};

		return j;
	}

	bool operator==(const cAsset &other) const
	{
		return (Id == other.Id);
	}
	bool operator!=(const cAsset &other) const
	{
		return !operator==(other);
	}
	void Clear()
	{
		// hepsini temizle
		Id.clear();
		HeadLine.clear();
		GroupId.clear();
		ErrMessage.clear();
		MediaFile.clear();
		MediaPath.clear();
		ProxyFile.clear();
		Language.clear();
		State = AssetState::astat_NONE;
		FirstTime = time(0);
		LastTime = time(0);
		OnDate = time(0);
		AgencySource = static_cast<int>(Agencies::a_none);
		MediaType = static_cast<int>(MediaTypes::mt_none);
		State = static_cast<int>(AssetState::astat_NONE);
		Success = static_cast<int>(AssetSuccess::asSuc_NoProblem);
		IsDeleted = false;
	}
	std::string GetProxyName()
	{
		std::string back = "";
		std::string dotchar(".");
		std::size_t LastPointPos = Globals::GetLastIndexOf(MediaFile, dotchar);
		std::size_t strlen = Globals::GetCharLen(MediaFile);
		if (MediaFile.empty() || LastPointPos == std::string::npos || strlen < 5)
			return back;

		std::string ext = Globals::GetSubString(MediaFile, LastPointPos);
		back = Globals::GetSubString(MediaFile, 0, strlen - (strlen - LastPointPos));
		back.append("_Proxy");
		back.append(ext);
		return back;
	}
};
