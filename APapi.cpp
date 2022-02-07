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
    strApiKey = GetApiKey();
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
    std::string strurl = Config::apiRootUrl + strQuery + "&page_size=" + std::to_string(PageSize) + "&in_my_plan=true&apikey=" + Config::apiKey;
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
    std::string itemurl, itemtype, itemdate;
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
        tmp.AgencySource = Agencies::a_aptn;
        tmp.HeadLine = GetJsonValue<std::string>(m["item"]["headline"]);
        itemdate = GetJsonValue<std::string>(m["item"]["versioncreated"]);
        if (!itemdate.empty())
            tmp.OnDate.fromTString(itemdate.c_str());

        itemurl = GetJsonValue<std::string>(m["item"]["uri"]);
        back = GetLanguage(itemurl, &tmp);
        if (!back.Success)
            return back;

        if (!((tmp.Language == "en") || (tmp.Language == "tr"))) // ingilizce veya turkce degil
            continue;

        if (tmp.MediaType == MediaTypes::mt_text)
        {

            tmp.bodyLink = GetJsonValue<std::string>(m["item"]["renditions"]["nitf"]["href"]);
        }
        else if (tmp.MediaType == MediaTypes::mt_video)
        {
            tmp.videoLink = GetJsonValue<std::string>(m["item"]["renditions"]["main_1080_25"]["href"]);
            tmp.bodyLink = GetJsonValue<std::string>(m["item"]["renditions"]["script_nitf"]["href"]);
            tmp.MediaFile = GetJsonValue<std::string>(m["item"]["renditions"]["main_1080_25"]["originalfilename"]);
            if (tmp.MediaFile.empty())
                tmp.MediaFile = tmp.Id + ".mp4";
            tmp.MediaFileSize = GetJsonValue<int32_t>(m["item"]["renditions"]["main_1080_25"]["sizeinbytes"]);
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

void apapi::IsSuccess(nlohmann::json &pJson, BackObject &pBack)
{
    nlohmann::json jtmp = pJson["error"];
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

std::string apapi::GetApiKey()
{
    std::string back = "&apikey=" + Config::apiKey;
    return back;
}

BackObject apapi::GetLanguage(std::string &strurl, cAsset *pAsset)
{
    BackObject back;
    std::string responsejson;
    strurl.append(strApiKey);

    back = ht->DoGet(strurl);
    if (!back.Success)
        return back;
    else
        responsejson = back.StrValue;

    nlohmann::json uriJson = nlohmann::json::parse(responsejson);
    if (uriJson.is_discarded())
    {
        back.ErrDesc = "failed to parse uri result";
        back.Success = false;
        return back;
    }

    nlohmann::json jtmp = uriJson["data"]["item"]["language"];
    pAsset->Language = GetJsonValue<std::string>(jtmp);
    return back;
}

BackObject apapi::GetBody(cAsset *pAsset)
{
    BackObject back;
    if (pAsset == nullptr)
    {
        back.ErrDesc = "asset is null";
        back.Success = false;
        return back;
    }
    if (pAsset->bodyLink.empty())
    {
        back.ErrDesc = "body link is empty!";
        back.Success = false;
        return back;
    }
    std::string strurl = pAsset->bodyLink + strApiKey;
    back = ht->DoGet(strurl);
    if (!back.Success)
        return back;
    else
    {
        back = ParseTextXml(back.StrValue, pAsset);
    }
    return back;
}

BackObject apapi::GetVideo(cAsset *pAsset)
{
    BackObject back;
        if (pAsset == nullptr)
    {
        back.ErrDesc = "asset is null";
        back.Success = false;
        return back;
    }
     if (pAsset->videoLink.empty())
    {
        back.ErrDesc = "video link is empty!";
        back.Success = false;
        return back;
    }
    std::string strurl =  pAsset->videoLink + strApiKey;

    back = ht->DoGetWritefile(strurl, Config::videoDownloadFolder + pAsset->MediaFile);
    if (back.Success)
    {
        pAsset->State = AssetState::astat_VIDEO_DOWNLOADED;
        pAsset->MediaPath = Config::videoDownloadFolder;
        pAsset->LastTime = time(0);
    }
    else
    {
        pAsset->Success = AssetSuccess::asSuc_Failed;
        pAsset->ErrMessage = back.ErrDesc;
    }

    return back;
}

void *apapi::ProcessFunc(void *parg)
{
    apapi *pObj = reinterpret_cast<apapi *>(parg);
    if (pObj == nullptr)
        return nullptr;
    std::cout << "Anadolu Agency process is starting..." << std::endl;
    BackObject back;
    int QueueSize = 0;
    int QueueCapacity = 0;

    QueueCapacity = pObj->Assets.GetBuffSize();

    /*
    back = pObj->GetNonCompletedAssets(QueueCapacity);
    if (!back.Success)
    {
        Logger::WriteLog(back.ErrDesc, LogType::warning);
    }

    QueueSize = pObj->Assets.GetSize();
    if (QueueSize > 0) // önce yarım kalanları bitir
    {
        pObj->WorkList();
        pObj->Assets.RemoveCompleted();
    }
*/
    //===============================================

    while (true)
    {
        Logger::WriteLog("Looping");
        if (pObj->StopFlag)
            goto cikis;

        back = pObj->Search();
        if (!back.Success)
        {
            Logger::WriteLog(back.ErrDesc);
            // if (++failureCount > 3)
            //   goto cikis;
        }

        /*
        // update db
        back = pObj->db.SaveAssets(pObj->Assets.GetPointer());
        if (!back.Success)
        {
            Logger::WriteLog(back.ErrDesc);
            goto cikis;
        }
        //
        */

        pObj->WorkList();

        int asize = pObj->Assets.GetSize();
        pObj->Assets.RemoveCompleted();
        int asize2 = pObj->Assets.GetSize();
        Logger::WriteLog(std::to_string(asize - asize2) + " completed assets removed from list");
        Sleep(Config::DownloadWaitSeconds * 1000);
    }

cikis:

    Logger::WriteLog("exiting download loop ...");

    return 0;
}

void apapi::WorkList()
{
    cAsset *pAsset = nullptr;
    BackObject back;

    pAsset = Assets.GetFirstWithSState(AssetSuccess::asSuc_NoProblem);

    while (pAsset != nullptr)
    {
        switch (pAsset->State)
        {
        case AssetState::astat_NONE:

            back = GetBody(pAsset );
            if (!back.Success)
                Logger::WriteLog(back.ErrDesc, LogType::error);

            /*
            // update db
            back = db.SaveAsset(pAsset);
            if (!back.Success)
            {
                pAsset->Success = AssetSuccess::asSuc_Failed; // todo bunu düşünelim
                pAsset->ErrMessage = back.ErrDesc;
                Logger::WriteLog(back.ErrDesc, LogType::error);
            }
            */

            break;

        case AssetState::astat_DOCUMENT_GET:

            back = GetVideo(pAsset);
            if (!back.Success)
                Logger::WriteLog(back.ErrDesc, LogType::error);

            /*
            // update db
            back = db.SaveAsset(pAsset);
            if (!back.Success)
            {
                pAsset->Success = AssetSuccess::asSuc_Failed;
                pAsset->ErrMessage = back.ErrDesc;
                Logger::WriteLog(back.ErrDesc, LogType::error);
            }
            */
            break;

        case AssetState::astat_VIDEO_DOWNLOADED:

            break;
        }

        if (pAsset->Success == AssetSuccess::asSuc_Failed)
            pAsset->IsDeleted = true;

        if (pAsset->MediaType == MediaTypes::mt_text && pAsset->State == AssetState::astat_DOCUMENT_GET)
            pAsset->Success = AssetSuccess::asSuc_Completed;

        // todo burda mı callbackde mi?
        if (pAsset->MediaType == MediaTypes::mt_video && pAsset->State == AssetState::astat_PROXY_CREATED)
            pAsset->Success = AssetSuccess::asSuc_Completed;

        if (StopFlag)
            break;
        Sleep(1000 * 10);
        if (StopFlag)
            break;
        pAsset = Assets.GetFirstWithSState(AssetSuccess::asSuc_NoProblem);
    }
}