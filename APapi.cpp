#include <iostream>
#include "APapi.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "pugixml.hpp"
#include "htmlDic.hpp"

//ctrl K ctrl S   shortcuts

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
    if (!back.Success)
        return back;
    Logger::WriteLog(back.StrValue);

    // get account plans
    strurl = Config::apiRootUrl + Config::apiAccountUrl + "/plans" + "?apikey=" + Config::apiKey;
    back = ht->DoGet(strurl);
    if (!back.Success)
        return back;
    Logger::WriteLog(back.StrValue);

    // get account quotas
    strurl = Config::apiRootUrl + Config::apiAccountUrl + "/quotas" + "?apikey=" + Config::apiKey;
    back = ht->DoGet(strurl);
    if (!back.Success)
        return back;
    Logger::WriteLog(back.StrValue);

    // get account downloads
    strurl = Config::apiRootUrl + Config::apiAccountUrl + "/downloads" + "?apikey=" + Config::apiKey;
    back = ht->DoGet(strurl);
    if (!back.Success)
        return back;
    Logger::WriteLog(back.StrValue);

    return back;
}

BackObject apapi::Search()
{
    BackObject back;
    UpdateSearchTime(LastSearch);
    std::string strQuery = Globals::replaceStringAll(Config::apiQueryUrl, QueryReplaceString, LastSearch.asTString());
    strQuery = ht->UrlEncode(strQuery);
    strQuery = "content/search?q=" + strQuery;
    std::string strurl = Config::apiRootUrl + strQuery + "&in_my_plan=true&apikey=" + Config::apiKey;
    back = ht->DoGet(strurl);
    if (back.Success)
    {
        std::string rawjson = back.StrValue;
        ParseSearch(rawjson);
    }
    return back;
}

void apapi::UpdateSearchTime(AssetTime &pAssetTime)
{
    if (pAssetTime.GetTime() == 0)
    {
        time_t now_t = 0;
        time_t gnow_t = 0;
        time_t assetgmtime = 0;
        time(&now_t);
        gnow_t = now_t - AssetTime::gmtdiff;
        if (pAssetTime.fromTString(Config::lastSearchTime.c_str()))
        {
            assetgmtime = pAssetTime.GetTime(false);
            double dif = difftime(gnow_t, assetgmtime);
            if (dif > Config::SearchInterval) // searchinterval den büyükse, searchinterval dakika öncesine çek yani resetle
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

BackObject apapi::ParseSearch(std::string &pjson)
{
    BackObject back;

    nlohmann::json j = nlohmann::json::parse(pjson);
    nlohmann::json uriJson;
    std::string itemurl, itemtype, itemurijson, itemdate, videolink;
    nlohmann::json jtmp;
    cAsset tmp;

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
    if (j["data"]["items"].is_null())
    {
        back.ErrDesc = "jsonparse:items not found!";
        back.Success = false;
        return back;
    }
    for (auto &m : j["data"]["items"])
    {
        tmp.Clear();
        jtmp = m["item"]["type"];
        itemtype = GetJsonValue<std::string>(jtmp);
        if (itemtype == "text")
            tmp.MediaType = MediaTypes::mt_text;
        else if (itemtype == "video")
        {
            tmp.MediaType = MediaTypes::mt_video;
        }
        else
        {
            continue;
        }

        tmp.Id = GetJsonValue<std::string>(m["item"]["altids"]["itemid"]);
        //tmp.Id = m["item"]["altids"]["itemid"].get<std::string>();
        tmp.AgencySource = Agencies::a_aptn;
        tmp.HeadLine = GetJsonValue<std::string>(m["item"]["headline"]);
        // tmp.HeadLine = m["item"]["headline"].get<std::string>();
        itemdate = GetJsonValue<std::string>(m["item"]["versioncreated"]);
        if (!itemdate.empty())
            tmp.OnDate.fromTString(itemdate.c_str());

        itemurl = GetJsonValue<std::string>(m["item"]["uri"]);
        // itemurl = m["item"]["uri"].get<std::string>();
        itemurl.append("&apikey=");
        itemurl.append(Config::apiKey);
        back = ht->DoGet(itemurl);
        if (!back.Success)
            return back;
        else
            itemurijson = back.StrValue;

       // std::cout << itemurijson << std::endl;

        uriJson = nlohmann::json::parse(itemurijson);
        if (uriJson.is_discarded())
        {
            back.ErrDesc = "failed to parse uri result";
            back.Success = false;
            return back;
        }

        jtmp = uriJson["data"]["item"]["language"];
        tmp.Language = GetJsonValue<std::string>(jtmp);
        if (!((tmp.Language == "en") || (tmp.Language == "tr"))) // ingilizce veya turkce degil
            continue;

        if (tmp.MediaType == MediaTypes::mt_text)
        {
            if (m["item"]["renditions"]["nitf"]["href"].is_null())
            {
                back.ErrDesc = "jsonparse:nitf href not found!";
                back.Success = false;
                return back;
            }
            std::string bodyUrl = m["item"]["renditions"]["nitf"]["href"].get<std::string>();
            bodyUrl.append("&apikey=");
            bodyUrl.append(Config::apiKey);
            back = ht->DoGet(bodyUrl);
            if (!back.Success)
                return back;
            else
            {
                back = ParseTextXml(back.StrValue, &tmp);
            }
        }
        else if (tmp.MediaType == MediaTypes::mt_video)
        {
            videolink = GetJsonValue<std::string>(m["item"]["renditions"]["main_1080_25"]["href"]);
            if (!videolink.empty())
            {
                videolink.append("&apikey=");
                videolink.append(Config::apiKey);
                tmp.MediaFile = GetJsonValue<std::string>(m["item"]["renditions"]["main_1080_25"]["originalfilename"]);
                if (tmp.MediaFile.empty())
                    tmp.MediaFile = tmp.Id + ".mp4";
                tmp.MediaFileSize = GetJsonValue<int32_t>(m["item"]["renditions"]["main_1080_25"]["sizeinbytes"]);
                back = ht->DoGetWritefile(videolink, Config::videoDownloadFolder + tmp.MediaFile);
                if (back.Success)
                {
                    tmp.State = AssetState::astat_VIDEO_DOWNLOADED;
                    tmp.MediaPath = Config::videoDownloadFolder;
                    tmp.LastTime = time(0);
                }
                else
                {
                    tmp.Success = AssetSuccess::asSuc_Failed;
                    tmp.ErrMessage = back.ErrDesc;
                }
            }

            if (m["item"]["renditions"]["nitf"]["href"].is_null())
            {
                back.ErrDesc = "jsonparse:nitf href not found!";
                back.Success = false;
                return back;
            }
            std::string bodyUrl = GetJsonValue<std::string>(m["item"]["renditions"]["script_nitf"]["href"]);
            if(bodyUrl.empty())
            {
                back.Success = false;
                back.ErrDesc = "failed to get nitf path";
                return back;
            }
            bodyUrl.append("&apikey=");
            bodyUrl.append(Config::apiKey);
            back = ht->DoGet(bodyUrl);
            if (!back.Success)
                return back;
            else
            {
                back = ParseTextXml(back.StrValue, &tmp);
            }
        }
        back = Assets.Add(tmp);
        DumpAsset(tmp);
    }

    back.Success = true;
    return back;
}

BackObject apapi::ParseTextXml(std::string &pXml, cAsset *pAsset)
{
    BackObject back;
    if (pAsset == nullptr || pXml.empty())
    {
        back.Success = false;
        back.ErrDesc = "ParseTextXml: invalid argument";
        return back;
    }
    //std::cout << pXml << std::endl;
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(pXml.c_str());
    if (!result)
    {
        std::string strErrdesc(result.description());
        back.ErrDesc = strErrdesc;
        back.Success = false;
        return back;
    }
    pugi::xpath_node node = doc.select_node("//nitf/body/body.content/block");
    if (!node)
    {
        back.ErrDesc = "failed to get nitf xml data";
        back.Success = false;
        return back;
    }

    std::stringstream ss;
    node.node().print(ss);

    std::string bodydata = ss.str();

    ReplaceHtml(bodydata);
    pAsset->Body = bodydata;
    back.Success = true;
    return back;
}

void apapi::ReplaceHtml(std::string &pstr)
{

    std::map<std::string, std::string>::iterator it;

    for (it = map1.begin(); it != map1.end(); it++)
    {

        pstr = Globals::replaceStringAll(pstr, it->first, it->second);
    }

    for (it = map2.begin(); it != map2.end(); it++)
    {

        pstr = Globals::replaceStringAll(pstr, it->first, it->second);
    }
}

void apapi::DumpAsset(cAsset &pAsset)
{
    std::cout << "==================================================================" << std::endl;
    std::cout << "Dumping Asset" << std::endl;
    std::cout << "id: " << pAsset.Id << std::endl;
    std::cout << "type: " << pAsset.MediaType << std::endl;
    std::cout << "ondate: " << pAsset.OnDate.asString() << std::endl;
    std::cout << "headline: " << pAsset.HeadLine << std::endl;
    std::cout << "body: " << pAsset.Body << std::endl;
    std::cout << "source: " << pAsset.AgencySource << std::endl;
    std::cout << std::endl;
}

void apapi::IsSuccess(nlohmann::json &pJon, BackObject &pBack)
{
    nlohmann::json jtmp = pJon["error"];
    if (jtmp.is_null())
        return;
    pBack.ErrDesc.append(" HttpStatus:");
    pBack.ErrDesc.append(jtmp["status"].get<std::string>());
    pBack.ErrDesc.append(" AP Code:");
    pBack.ErrDesc.append(jtmp["code"].get<std::string>());
    pBack.ErrDesc.append(" message:");
    pBack.ErrDesc.append(jtmp["message"].get<std::string>());
    pBack.Success = false;
}

template <typename T>
T apapi::GetJsonValue(nlohmann::json &pJson)
{
    T back;
    if (!(pJson.is_null()))
    {
        back = pJson.get<T>();
    }
    return back;
}
