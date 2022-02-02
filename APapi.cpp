#include <iostream>
#include "APapi.hpp"
#include "Config.hpp"
#include "Logger.hpp"


apapi::apapi()
{
    ht = nullptr;
}

apapi::~apapi()
{
    if (ht)
    {
        delete ht;
        ht = nullptr;
    }
}

bool apapi::Initiliaze() 
{
    ht = new cHttpManager();
    if (ht == nullptr)
        return false;
    StopFlag = false;
    if(Config::lastSearchTime.empty())

    LastSearch.fromTString(Config::lastSearchTime.c_str());
    return true;
}

BackObject apapi::GetAccountInfo() 
{
    BackObject back;
    // get account info
    std::string strurl = Config::apiRootUrl + Config::apiAccountUrl + "?apikey=" + Config::apiKey;   
    std::wstring strback;
    ht = new cHttpManager();
       if (ht == nullptr)
        return back;
    ht->ClearHeaders();
    back = ht->DoGet(strurl);
    if(!back.Success) return back;
    Logger::WriteLog(back.StrValue);

    // get account plans
    strurl = Config::apiRootUrl + Config::apiAccountUrl + "/plans" +  "?apikey=" + Config::apiKey;
    back = ht->DoGet(strurl);
    if(!back.Success) return back;
    Logger::WriteLog(back.StrValue);

    // get account quotas
    strurl = Config::apiRootUrl + Config::apiAccountUrl + "/quotas" +  "?apikey=" + Config::apiKey;
    back = ht->DoGet(strurl);
    if(!back.Success) return back;
    Logger::WriteLog(back.StrValue);

    // get account downloads
    strurl = Config::apiRootUrl + Config::apiAccountUrl + "/downloads" +  "?apikey=" + Config::apiKey;
    back = ht->DoGet(strurl);
    if(!back.Success) return back;
    Logger::WriteLog(back.StrValue);

    return back;
}

void apapi::GetNextSearchTime(AssetTime& pAssetTime) 
{
    if(pAssetTime.GetTime() == 0)
    {
        pAssetTime.fromTString(Config::lastSearchTime.c_str());
        time_t now_t = 0;
        time_t gnow_t = 0;
        time(&now_t);
        gnow_t = now_t - AssetTime::gmtdiff;

        double dif = difftime(pAssetTime.GetTime(false), gnow_t);
        if(dif > Config::SearchInterval) // searchinterval den büyükse, searchinterval dakika öncesine çek
        {
            now_t -= Config::SearchInterval;
            pAssetTime.SetTime(now_t);
        }
    }
    else
    {
        
    }
}
