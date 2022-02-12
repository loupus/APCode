#include "winsock2.h"
#include <iostream>
#include "Globals.hpp"
#include "APapi.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "pugixml.hpp"
#include "htmlDic.hpp"

// ctrl K ctrl S   shortcuts

apapi::apapi()
{
}

apapi::~apapi()
{
    Stop();
}

bool apapi::Initiliaze()
{
    strApiKey = GetApiKey();
    ht.Initiliaze();
    db.Initiliaze();
    // ht->ClearHeaders();
    ht.AddHeader("Content-Type: application/x-www-form-urlencoded", true);
    StopFlag = false;
    return true;
}

BackObject apapi::GetAccountInfo()
{
    BackObject back;
    // get account info
    std::string strurl = Config::apiRootUrl + Config::apiAccountUrl + "?apikey=" + Config::apiKey;
    std::wstring strback;

    back = ht.DoGet(strurl);
    if (!back.Success)
        return back;
    Logger::WriteLog(back.StrValue);

    // get account plans
    strurl = Config::apiRootUrl + Config::apiAccountUrl + "/plans" + "?apikey=" + Config::apiKey;
    back = ht.DoGet(strurl);
    if (!back.Success)
        return back;
    Logger::WriteLog(back.StrValue);

    // get account quotas
    strurl = Config::apiRootUrl + Config::apiAccountUrl + "/quotas" + "?apikey=" + Config::apiKey;
    back = ht.DoGet(strurl);
    if (!back.Success)
        return back;
    Logger::WriteLog(back.StrValue);

    // get account downloads
    strurl = Config::apiRootUrl + Config::apiAccountUrl + "/downloads" + "?apikey=" + Config::apiKey;
    back = ht.DoGet(strurl);
    if (!back.Success)
        return back;
    Logger::WriteLog(back.StrValue);

    return back;
}

BackObject apapi::Search()
{
    BackObject back;
    std::string strurl;
    if (NextPage.empty())
    {
        UpdateSearchTime();
        std::string strQuery = Globals::replaceStringAll(Config::apiQueryUrl, QueryReplaceString, LastSearch.asTString());
        strQuery = ht.UrlEncode(strQuery);
        strQuery = "content/search?q=" + strQuery;
        strurl = Config::apiRootUrl + strQuery + "&page_size=" + std::to_string(PageSize) + "&in_my_plan=true";
    }
    else
        strurl = NextPage;

    strurl.append(strApiKey);

    Logger::WriteLog("search query: " + strurl);
    back = ht.DoGet(strurl);
    if (back.Success)
    {
        std::string rawjson = back.StrValue;
        ParseSearch(rawjson);
    }
    return back;
}

void apapi::UpdateSearchTime()
{
    // searche aralık vermel lazım
    time_t now_t = 0;
    time(&now_t);
    double dif = 0;
    struct tm *bugun;
    bugun = std::localtime(&now_t);
    bugun->tm_hour = 0;
    bugun->tm_min = 0;
    bugun->tm_sec = 0;
    time_t bugun_t = mktime(bugun);

    if (LastSearch.GetTime() == 0)
    {
        if (LastSearch.fromString(Config::lastSearchTime.c_str()))
        {
            time_t ozaman = LastSearch.GetTime();
            dif = difftime(now_t, bugun_t);
            double dtmp = difftime(now_t, ozaman);
            if (dtmp > dif) // bugunden önce ise bugune resetle
            {
                LastSearch.SetTime(bugun_t);
            }
        }
        else
        {
            LastSearch.SetTime(bugun_t); // konfigten alamadık bugune resetle
        }
        NextSearch = now_t;
    }
    else
    {
        LastSearch = NextSearch;
        NextSearch = now_t;
    }
    Config::lastSearchTime = LastSearch.asString();
    Config::WriteConfig();
    Logger::WriteLog("LastSearch: " + LastSearch.asString());
    // std::cout << "LastSearch: " << LastSearch.asString() << std::endl;
}

BackObject apapi::ParseSearch(std::string &pjson)
{
    BackObject back;

    nlohmann::json j = nlohmann::json::parse(pjson);
    std::string itemtype, itemdate;
    nlohmann::json jtmp;
    cAsset tmp;
    int TotalItems = 0;
    int CurrentPage = 0;

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

    NextPage = GetJsonValue<std::string>(j["data"]["next_page"]);
    TotalItems = GetJsonValue<int>(j["data"]["total_items"]);
    CurrentPage = GetJsonValue<int>(j["data"]["current_page"]);

    Logger::WriteLog("Total Items: " + std::to_string(TotalItems) + " CurrentPage: " + std::to_string(CurrentPage));

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
        if (tmp.HeadLine.empty())
        {
            Logger::WriteLog("Header Empty of Id: " + tmp.Id);
        }
        itemdate = GetJsonValue<std::string>(m["item"]["versioncreated"]);
        if (!itemdate.empty())
            tmp.OnDate.fromTString(itemdate.c_str());

        tmp.itemUrl = GetJsonValue<std::string>(m["item"]["uri"]);
        if (tmp.itemUrl.empty())
            continue;
        back = GetLanguage(tmp.itemUrl, &tmp);
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
            tmp.bodyLink = GetJsonValue<std::string>(m["item"]["renditions"]["script_nitf"]["href"]);
            PickVideoLink(m["item"]["renditions"], &tmp);
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
    // std::cout << pXml << std::endl;
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
    std::string bodydata = ss.str(); // todo burda bazen sapıtıyor
    ReplaceHtml(bodydata);
    pAsset->Body = bodydata;
    if (pAsset->Body.empty())
    {
        Logger::WriteLog("Body Empty of: " + pAsset->Id);
    }
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
    std::cout.flush();
    std::cout << "==================================================================" << std::endl;
    std::cout << "Dumping Asset" << std::endl;
    std::cout << "id: " << pAsset.Id << std::endl;
    std::cout << "type: " << (pAsset.MediaType == MediaTypes::mt_text ? "text" : "video") << std::endl;
    std::cout << "ondate: " << pAsset.OnDate.asString().c_str() << std::endl;
    std::cout << "headline: " << pAsset.HeadLine.c_str() << std::endl;
    std::cout << "body: " << pAsset.Body.c_str() << std::endl;
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

BackObject apapi::GetLanguage(std::string &pStrUrl, cAsset *pAsset)
{
    BackObject back;
    std::string responsejson;
    std::string strurl = pStrUrl + strApiKey;

    back = ht.DoGet(strurl);
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
    std::string strurl;
    if (pAsset == nullptr)
    {
        back.ErrDesc = "asset is null";
        back.Success = false;
        return back;
    }
    if (pAsset->bodyLink.empty()) // ise itemurlden git
    {
        Logger::WriteLog("body link almaya calisacak");
        strurl = pAsset->itemUrl + strApiKey;
        back = ht.DoGet(strurl);
        if (!back.Success)
        {
            pAsset->Success = AssetSuccess::asSuc_Failed;
            back.ErrDesc = "failed to get bodylink by querying itemurl: " + strurl;
            back.Success = false;
            return back;
        }
        nlohmann::json j = nlohmann::json::parse(back.StrValue);
        if (j.is_discarded())
        {
            back.ErrDesc = "failed to parse itemurl result";
            back.Success = false;
            return back;
        }

        if (pAsset->MediaType == MediaTypes::mt_text)
            pAsset->bodyLink = GetJsonValue<std::string>(j["item"]["renditions"]["nitf"]["href"]);
        else if (pAsset->MediaType == MediaTypes::mt_video)
            pAsset->bodyLink = GetJsonValue<std::string>(j["item"]["renditions"]["script_nitf"]["href"]);

        if (pAsset->bodyLink.empty())
        {
            Logger::WriteLog("body link alamadı");
            return back; // doğru ve yanlış değil, tehir edilecek
        }
        else
        {
            Logger::WriteLog("body link: " + pAsset->bodyLink);
        }
    }
    strurl = pAsset->bodyLink + strApiKey;
    back = ht.DoGet(strurl);
    if (!back.Success)
        return back;
    else
    {
        back = ParseTextXml(back.StrValue, pAsset);
        if (back.Success)
        {
            pAsset->State = AssetState::astat_DOCUMENT_GET;
            if (pAsset->MediaType == MediaTypes::mt_text)
            {
                pAsset->Success = AssetSuccess::asSuc_Completed;
            }
            pAsset->LastTime = time(0);
        }
        else
        {
            pAsset->Success = AssetSuccess::asSuc_Failed;
            pAsset->ErrMessage = back.ErrDesc;
        }
    }
    return back;
}

BackObject apapi::GetVideo(cAsset *pAsset)
{
    BackObject back;
    std::string strurl;
    if (pAsset == nullptr)
    {
        back.ErrDesc = "asset is null";
        back.Success = false;
        return back;
    }
    if (pAsset->videoLink.empty()) // ise itemurlden git
    {

        strurl = pAsset->itemUrl + strApiKey;
        back = ht.DoGet(strurl);
        if (!back.Success)
        {
            pAsset->Success = AssetSuccess::asSuc_Failed;
            back.ErrDesc = "failed to get videolink by querying itemurl: " + strurl;
            back.Success = false;
            return back;
        }
        nlohmann::json j = nlohmann::json::parse(back.StrValue);
        if (j.is_discarded())
        {
            back.ErrDesc = "failed to parse itemurl result";
            back.Success = false;
            return back;
        }
        PickVideoLink(j["data"]["item"]["renditions"], pAsset);

        if (pAsset->videoLink.empty())
        {
            int nezamandir = difftime(pAsset->LastTime.GetTime(), pAsset->FirstTime.GetTime());

            Logger::WriteLog("video link henuz yok, itemurl: " + pAsset->itemUrl + "    " + std::to_string(nezamandir / 60) + " dakika");
            return back; // doğru ve yanlış değil, tehir edilecek
        }
        else
            Logger::WriteLog("video link " + pAsset->videoLink);
    }
    strurl = pAsset->videoLink + strApiKey;

    back = ht.DoGetWritefile(strurl, Config::videoDownloadFolder + pAsset->MediaFile);
    if (back.Success)
    {
        pAsset->State = AssetState::astat_VIDEO_DOWNLOADED;
        pAsset->Success = AssetSuccess::asSuc_Completed;
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

void apapi::PickVideoLink(nlohmann::json &pJson, cAsset *pAsset)
{

    pAsset->videoLink = GetJsonValue<std::string>(pJson["main_1080_25"]["href"]);
    if (pAsset->videoLink.empty())
    {
        pAsset->videoLink = GetJsonValue<std::string>(pJson["main"]["href"]);
        pAsset->MediaFile = GetJsonValue<std::string>(pJson["main"]["originalfilename"]);
        if (pAsset->MediaFile.empty())
            pAsset->MediaFile = pAsset->Id + ".mp4";
        pAsset->MediaFileSize = GetJsonValue<int32_t>(pJson["main"]["sizeinbytes"]);
    }
    else
    {
        pAsset->MediaFile = GetJsonValue<std::string>(pJson["main_1080_25"]["originalfilename"]);
        if (pAsset->MediaFile.empty())
            pAsset->MediaFile = pAsset->Id + ".mp4";
        pAsset->MediaFileSize = GetJsonValue<int32_t>(pJson["main_1080_25"]["sizeinbytes"]);
    }
    if (pAsset->videoLink.empty()) // videolink yoksa sil
        pAsset->IsDeleted = true;
}

BackObject apapi::GetNonCompletedAssets(int nitems)
{
    BackObject back;
    // pthread_mutex_init(&lock, NULL);   // initialize the lock
    // pthread_mutex_lock(&lock);  // acquire lock
    std::string today = Globals::GetNowStr();
    back = db.GetAssets(Assets.GetPointer(), today, (int)AssetSuccess::asSuc_NoProblem, (int)Agencies::a_aptn, nitems);
    // pthread_mutex_unlock(&lock);  // release lock
    return back;
}

void *apapi::ProcessFunc(void *parg)
{
    apapi *pObj = reinterpret_cast<apapi *>(parg);
    if (pObj == nullptr)
        return nullptr;
    std::cout << "AP Agency process is starting..." << std::endl;
    BackObject back;
    //  int QueueSize = 0;
    //  int QueueCapacity = 0;

    // QueueCapacity = pObj->Assets.GetBuffSize();
    /*
        if (Config::PersistDb)
        {
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
        }
    */
    //===============================================

    while (true)
    {
        Logger::WriteLog("=============================================================================================================");
       // Logger::WriteLog("Looping");
        if (pObj->StopFlag)
            goto cikis;

        back = pObj->Search();
        if (!back.Success)
        {
            Logger::WriteLog(back.ErrDesc);
            // if (++failureCount > 3)
            //   goto cikis;
        }

        pObj->WorkList();

        // update db
        if (Config::PersistDb)
        {
            back = pObj->db.SaveAssets(pObj->Assets.GetPointer());
            if (!back.Success)
            {
                Logger::WriteLog(back.ErrDesc);
                goto cikis;
            }
        }

        int asize = pObj->Assets.GetSize();
        pObj->Assets.RemoveCompleted();
        int asize2 = pObj->Assets.GetSize();
        Logger::WriteLog(std::to_string(asize - asize2) + " completed assets removed from list, remaining " + std::to_string(asize2));
        Sleep(Config::DownloadWaitSeconds * 1000);
    }

cikis:

    Logger::WriteLog("exiting download loop ...");
    pthread_mutex_lock(&(pObj->mutex));
    pObj->IsComplete = true;
    pthread_mutex_unlock(&(pObj->mutex));
    pthread_cond_signal(&(pObj->is_zero));
    pthread_exit(nullptr);
    return 0;
}

void apapi::WorkList()
{
    cAsset *pAsset = nullptr;
    BackObject back;

    std::list<cAsset> *Items = Assets.GetPointer();

    for (auto i = Items->begin(); i != Items->end(); i++)
    {
        if (i->Success == AssetSuccess::asSuc_NoProblem && i->IsDeleted == false)
            pAsset = &(*i);
        else
            continue;

        pAsset->LastTime = time(0);
        switch (pAsset->State)
        {
        case AssetState::astat_NONE:

            back = GetBody(pAsset);
            if (!back.Success)
                Logger::WriteLog(back.ErrDesc, LogType::error);

            // update db
            if (Config::PersistDb)
            {
                back = db.SaveAsset(pAsset);
                if (!back.Success)
                {
                    pAsset->Success = AssetSuccess::asSuc_Failed; // todo bunu düşünelim
                    pAsset->ErrMessage = back.ErrDesc;
                    Logger::WriteLog(back.ErrDesc, LogType::error);
                }
            }
            break;

        case AssetState::astat_DOCUMENT_GET:

            back = GetVideo(pAsset);
            if (!back.Success)
                Logger::WriteLog(back.ErrDesc, LogType::error);

            // update db
            if (Config::PersistDb)
            {
                back = db.SaveAsset(pAsset);
                if (!back.Success)
                {
                    pAsset->Success = AssetSuccess::asSuc_Failed;
                    pAsset->ErrMessage = back.ErrDesc;
                    Logger::WriteLog(back.ErrDesc, LogType::error);
                }
            }
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
    }
}

/*
void apapi::WorkList()
{
    cAsset *pAsset = nullptr;
    BackObject back;

    pAsset = Assets.GetFirstWithSState(AssetSuccess::asSuc_NoProblem);  // boyle degil, hepsini bir tur don

    while (pAsset != nullptr)
    {
        switch (pAsset->State)
        {
        case AssetState::astat_NONE:

            back = GetBody(pAsset);
            if (!back.Success)
                Logger::WriteLog(back.ErrDesc, LogType::error);

            // update db
            if (Config::PersistDb)
            {
                back = db.SaveAsset(pAsset);
                if (!back.Success)
                {
                    pAsset->Success = AssetSuccess::asSuc_Failed; // todo bunu düşünelim
                    pAsset->ErrMessage = back.ErrDesc;
                    Logger::WriteLog(back.ErrDesc, LogType::error);
                }
            }
            break;

        case AssetState::astat_DOCUMENT_GET:

            back = GetVideo(pAsset);
            if (!back.Success)
                Logger::WriteLog(back.ErrDesc, LogType::error);

            // update db
            if (Config::PersistDb)
            {
                back = db.SaveAsset(pAsset);
                if (!back.Success)
                {
                    pAsset->Success = AssetSuccess::asSuc_Failed;
                    pAsset->ErrMessage = back.ErrDesc;
                    Logger::WriteLog(back.ErrDesc, LogType::error);
                }
            }
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
*/
void apapi::Stop()
{
    int rv = 0;
    void *res;
    // timestruct_t abstime;
    pthread_mutex_lock(&mutex);
    StopFlag = true;
    while (IsComplete == false)
    {
        pthread_cond_wait(&is_zero, &mutex);
        // pthread_cond_timedwait(&cv, &mp, &abstime);
    }
    pthread_mutex_unlock(&mutex);

    rv = pthread_join(thHandle, &res);
    rv = pthread_mutex_destroy(&mutex);
    rv = pthread_cond_destroy(&is_zero);

    if (rv != 0)
    {
        // ESRCH
        // std::cout << "ERR ====> join failed. ErrCode:" << rv << std::endl;
    }
}

void apapi::Start()
{

    int rv = 0;
    StopFlag = false;
    IsComplete = false;
    rv = pthread_cond_init(&is_zero, NULL);
    rv = pthread_mutex_init(&mutex, NULL);

    rv = pthread_create(&thHandle, NULL, &apapi::ProcessFunc, (void *)this);

    rv = pthread_detach(thHandle);
    if (rv != 0)
    {
        std::cout << "ERR ===> Transcode thread create failed! RV:" << std::endl;
    }
    else
    {
        // std::cout << dye::blue("Thread: ")  << thHandle.x << std::endl;
        // std::cout << hue::blue << "Thread: " << thHandle.x << hue::reset << std::endl;
    }
}