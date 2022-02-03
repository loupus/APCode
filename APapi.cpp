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
    //ht->ClearHeaders();
    ht->AddHeader("Content-Type: application/x-www-form-urlencoded");
    StopFlag = false;
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

BackObject apapi::Search() 
{
    BackObject back;
    UpdateSearchTime(LastSearch);
    std::string strQuery = Globals::replaceStringAll(Config::apiQueryUrl, QueryReplaceString ,LastSearch.asTString());
    strQuery = ht->UrlEncode(strQuery);
    strQuery = "content/search?q=" + strQuery;
    std::string strurl = Config::apiRootUrl + strQuery + "&in_my_plan=true&apikey=" + Config::apiKey; 
    back = ht->DoGet(strurl);
    return back;
}

void apapi::UpdateSearchTime(AssetTime& pAssetTime) 
{
    if(pAssetTime.GetTime() == 0)
    {
        time_t now_t = 0;
        time_t gnow_t = 0;
        time_t assetgmtime = 0;
        time(&now_t);
        gnow_t = now_t - AssetTime::gmtdiff;
        if(pAssetTime.fromTString(Config::lastSearchTime.c_str()))
        { 
            assetgmtime = pAssetTime.GetTime(false);
            double dif = difftime(gnow_t,assetgmtime);
            if(dif > Config::SearchInterval) // searchinterval den büyükse, searchinterval dakika öncesine çek yani resetle
            {
                now_t -= Config::SearchInterval;
                pAssetTime.SetTime(now_t);
            }
        }
        else
        {
             now_t -= Config::SearchInterval; // konfigten çekemedi resetle
             pAssetTime.SetTime(now_t);
        }

    }
    else
    {
        pAssetTime.AddTime(Config::SearchInterval);
    }
}

BackObject apapi::ParseSearch(std::string& pjson) 
{
    BackObject back;
     nlohmann::json j = nlohmann::json::parse(pjson);
    if (j.is_discarded())
    {
        back.ErrDesc = "failed to parse discovery result";
        back.Success = false;
        return back;
    }
    /*  hata mesajı içeren json gelirse diye
       if (!IsSuccess(j))
    {
        Logger::WriteLog("failed to get discovery result", LogType::error);
        return false;
    }
    */
   for (auto &m : j["data"]["items"].items())
    {

        cAsset tmp;
        tmp.Clear();
        tmp.AgencySource = Agencies::a_aa;
        for (auto &d : m.value().items())
        {
            if (d.key() == "id")
                tmp.Id = (d.value());
            else if (d.key() == "type")
            {
                std::string strtype = d.value().get<std::string>();
                tmp.MediaType = Globals::to_lower(strtype) == "video" ? MediaTypes::mt_video : MediaTypes::mt_text;
            }
            else if (d.key() == "date")
                tmp.OnDate.fromTString(d.value().get<std::string>().c_str());
            else if (d.key() == "title")
                tmp.HeadLine = (d.value());
            else if (d.key() == "group_id")
                tmp.GroupId = (d.value());
        }

        BackObject back = Assets.Add(tmp);
        if (back.Success == false)
           return back;
    }

    back.Success = true;
    return back;
}
