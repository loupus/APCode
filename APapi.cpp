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
    std::cout << pjson << std::endl  << std::endl;
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
    if (j["data"]["items"].is_null())
    {
        back.ErrDesc = "jsonparse:items not found!";
        back.Success = false;
        return back;
    }
    for (auto &m : j["data"]["items"])
    {
        nlohmann::json jtype = m["item"]["type"];
        if (jtype.is_null())
        {
            back.ErrDesc = "jsonparse:item type not found!";
            back.Success = false;
            return back;
        }
        cAsset tmp;
        tmp.Id = m["item"]["altids"]["itemid"].get<std::string>();
        tmp.AgencySource = Agencies::a_aptn;
        tmp.HeadLine = m["item"]["headline"].get<std::string>();
        tmp.OnDate.fromTString(m["item"]["versioncreated"].get<std::string>().c_str());
        if (jtype.get<std::string>() == "text")
        {
            tmp.MediaType = MediaTypes::mt_text;
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
        else
        {
            std::cout << "text degil" << std::endl;
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
    std::cout << pXml << std::endl;
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
    std::cout << "==================================================================" <<  std::endl;
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