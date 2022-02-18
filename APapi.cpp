#include "winsock2.h"
#include <iostream>
#include <filesystem>
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
    // std::string strurl = Config::apiRootUrl + Config::apiAccountUrl + "?apikey=" + Config::apiKey;

    std::string strRootUrl = Config::AsStr("//Config/Agencies/AP/apiRootUrl");
    std::string strAccountUrl = Config::AsStr("//Config/Agencies/AP/apiAccountUrl");
    std::string strurltmp = strRootUrl + strAccountUrl + strApiKey;
    std::string strurl;

    back = ht.DoGet(strurltmp);
    if (!back.Success)
        return back;
    Logger::WriteLog(back.StrValue);

    // get account plans
    strurl = strurltmp + "/plans" + strApiKey;
    back = ht.DoGet(strurl);
    if (!back.Success)
        return back;
    Logger::WriteLog(back.StrValue);

    // get account quotas
    strurl = strurltmp + "/quotas" + strApiKey;
    back = ht.DoGet(strurl);
    if (!back.Success)
        return back;
    Logger::WriteLog(back.StrValue);

    // get account downloads
    strurl = strurltmp + "/downloads" + strApiKey;
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
        std::string apiQuery = Config::AsStr("//Config/Agencies/AP/apiQueryUrl");
        std::string apiRoot = Config::AsStr("//Config/Agencies/AP/apiRootUrl");
        std::string strQuery = Globals::replaceStringAll(apiQuery, QueryReplaceString, LastSearch.asTString());
        strQuery = ht.UrlEncode(strQuery);
        strQuery = "content/search?q=" + strQuery;
        strurl = apiRoot + strQuery + "&page_size=" + std::to_string(PageSize) + "&in_my_plan=true";
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
        // if (LastSearch.fromString(Config::lastSearchTime.c_str()))
        if (LastSearch.fromString(Config::AsStr("//Config/Agencies/AP/lastSearchTime").c_str()))
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

    Config::SetValue("//Config/Agencies/AP/lastSearchTime", LastSearch.asString().c_str());
    Config::WriteConfigEx();
    Logger::WriteLog("LastSearch: " + LastSearch.asString());
}

BackObject apapi::ParseSearch(std::string &pjson)
{
    BackObject back;

    nlohmann::json j = nlohmann::json::parse(pjson);
    std::string itemtype, itemdate, tempstr;
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
        {
            tempstr = "item with id " + tmp.Id + " language: " + tmp.Language + " will be discarded";
            Logger::WriteLog(tempstr);
            tempstr.clear();
            continue;
        }

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

    //  std::string aa = pugi::

    std::stringstream ss;
    node.node().print(ss);
    std::string bodydata = ss.str();

    bodydata = Globals::ReplaceString2(bodydata, "<block>", "");
    bodydata = Globals::ReplaceString2(bodydata, "</block>", "");

    ReplaceHtml(bodydata);
    pAsset->Body = bodydata;
    if (pAsset->Body.empty())
    {
        back.Success = false;
        back.ErrDesc = "Body is empty";
        return back;
    }
    back.Success = true;
    return back;
}

void apapi::RemoveHrefTags(std::string &pStr)
{

    std::string beginstr = "<a href";
    std::string endstr = ">";
    std::string removestr = "";
    std::string emptystr = "";
    size_t beginpos = 0, endpos = 0;
    do
    {
        beginpos = Globals::GetFirstIndexOf(pStr, beginstr, beginpos);
        if (beginpos > 0 && beginpos != std::string::npos)
        {
            endpos = Globals::GetFirstIndexOf(pStr, endstr, beginpos);
            if (endpos == std::string::npos)
                break;
            removestr = Globals::GetSubString(pStr, beginpos, (endpos - beginpos + 1));
            pStr = Globals::ReplaceString(pStr, removestr, emptystr);
            beginpos = endpos;
        }

        beginpos = Globals::GetFirstIndexOf(pStr, beginstr, beginpos);

    } while (beginpos > 0 && beginpos != std::string::npos);

    beginstr = "</a>";
    pStr = Globals::ReplaceString(pStr, beginstr, emptystr);

    beginstr = "<p />";
    pStr = Globals::ReplaceString(pStr, beginstr, emptystr);

    // bodytext = Globals::replaceStringAll(bodytext,"<a href=\"","link:");
    // bodytext = Globals::replaceStringAll(bodytext,"</a>","");
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
    std::string back = "&apikey=" + Config::AsStr("//Config/Agencies/AP/apiKey");
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

    jtmp = uriJson["data"]["item"]["urgency"];
    pAsset->urgency = GetJsonValue<int>(jtmp);

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
        strurl = pAsset->itemUrl + strApiKey;
        back = ht.DoGet(strurl);
        if (!back.Success)
        {
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
            pAsset->bodyLink = GetJsonValue<std::string>(j["data"]["item"]["renditions"]["nitf"]["href"]);
        else if (pAsset->MediaType == MediaTypes::mt_video)
            pAsset->bodyLink = GetJsonValue<std::string>(j["data"]["item"]["renditions"]["script_nitf"]["href"]);

        if (pAsset->bodyLink.empty())
        {
            back.ErrDesc = "failed to get body link";
            back.Success = false;
            return back;
        }
    }
    strurl = pAsset->bodyLink + strApiKey;
    back = ht.DoGet(strurl);
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

        if (pAsset->videoLink.empty()) // videolink yoksa sil
        {
            back.ErrDesc = "failed to get videolink";
            back.Success = false;
            return back;
        }
    }
    // configMap["//Config/Agencies/AP/apiKey"]
    strurl = pAsset->videoLink + strApiKey;
    std::string fpath = Config::AsStr("//Config/Agencies/AP/videoDownloadFolder") + pAsset->MediaFile;
    if (!std::filesystem::exists(fpath))
        back = ht.DoGetWritefile(strurl, fpath);
    else
        Logger::WriteLog("file already exists : " + fpath);

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

BackObject apapi::GenerateEgsXml(cAsset *pAsset)
{
    BackObject back;
    back.Success = false;
    back.ErrDesc = "failed to create egs xml";
    bool rv = false;

    pugi::xml_document doc;
    pugi::xml_node tmp;
    auto root = doc.append_child("Dummy");

    tmp = root.append_child("NewsItemId");
    rv = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(pAsset->Id.c_str());
    if (!rv)
        return back;
    tmp = root.append_child("DateAndTime");
    rv = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(pAsset->OnDate.asString().c_str());
    if (!rv)
        return back;
    tmp = root.append_child("HeadLine");
    rv = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(pAsset->HeadLine.c_str());
    if (!rv)
        return back;

    std::string bodytext = pAsset->Body.c_str();
    RemoveHrefTags(bodytext);
    bodytext.append("=================================================\n");
    bodytext.append("news id: ");
    bodytext.append(pAsset->Id);
    bodytext.append(" news date: ");
    bodytext.append(pAsset->OnDate.asString() + "\n");

    if (pAsset->urgency == AssetUrgencies::ur_flash)
        bodytext.append("acelehaber");

    tmp = root.append_child("Body");
    rv = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(bodytext.c_str());
    if (!rv)
        return back;

    std::string fname = Config::AsStr("//Config/Agencies/AP/EgsFolder") + pAsset->Id + ".xml";
    rv = doc.save_file(fname.c_str(), PUGIXML_TEXT("  "), pugi::format_no_escapes);
    if (!rv)
    {
        back.Success = false;
        back.ErrDesc = "failed to save egs xml";
    }
    else
    {
        back.Success = true;
        back.ErrDesc.clear();
    }

    return back;
}

void *apapi::ProcessFunc(void *parg)
{
    apapi *pObj = reinterpret_cast<apapi *>(parg);
    if (pObj == nullptr)
        return nullptr;
    std::cout << "AP Agency process is starting..." << std::endl;
    BackObject back;
    time_t pbegin, pend;
    int tdiff;
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
        pbegin = time(0);
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

        if (Config::AsInt("//Config/Agencies/AP/PersistDb") == 1)
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
        pend = time(0);
        tdiff = difftime(pend, pbegin);
        Logger::WriteLog("period time elapsed: " + std::to_string(tdiff) + " seconds");
        if (tdiff >= Config::AsInt("//Config/Agencies/AP/SleepSeconds"))
            continue; // beklemeye gerek yok
        Sleep(Config::AsInt("//Config/Agencies/AP/SleepSeconds") * 1000);
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
    Logger::WriteLog("Items Count : " + std::to_string(Items->size()));

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
            {
                Logger::WriteLog(back.ErrDesc, LogType::error);
                pAsset->Success = AssetSuccess::asSuc_Failed;
            }
            else
            {
                pAsset->State = AssetState::astat_DOCUMENT_GET;
                if (!Config::AsStr("//Config/Agencies/AP/EgsFolder").empty())
                {
                    back = GenerateEgsXml(pAsset);
                    if (!back.Success)
                    {
                        Logger::WriteLog(back.ErrDesc, LogType::error);
                    }
                }
            }

            // update db
            if (Config::AsInt("//Config/Agencies/AP/PersistDb") == 1)
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
            {
                Logger::WriteLog(back.ErrDesc, LogType::error);
                pAsset->Success = AssetSuccess::asSuc_Failed;
            }
            else
            {
                pAsset->State = AssetState::astat_VIDEO_DOWNLOADED;
                pAsset->MediaPath = Config::AsStr("//Config/Agencies/AP/videoDownloadFolder");
                pAsset->LastTime = time(0);
            }

            // update db
            if (Config::AsInt("//Config/Agencies/AP/PersistDb") == 1)
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

        if (pAsset->MediaType == MediaTypes::mt_video && pAsset->State == AssetState::astat_VIDEO_DOWNLOADED)
            pAsset->Success = AssetSuccess::asSuc_Completed;

        if (StopFlag)
            break;
        Sleep(1000);
        if (StopFlag)
            break;
    }
    Logger::WriteLog("Worklist completed");
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
        // std::cout << "ERR ===> Transcode thread create failed! RV:" << std::endl;
    }
    else
    {
        // std::cout << dye::blue("Thread: ")  << thHandle.x << std::endl;
        // std::cout << hue::blue << "Thread: " << thHandle.x << hue::reset << std::endl;
    }
}