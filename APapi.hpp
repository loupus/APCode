#pragma once
#include <iostream>
#include "AssetQueue.hpp"
#include "HttpManager.hpp"

class apapi
{
private:
    cAssetQueue Assets;
    cHttpManager *ht;
    volatile bool StopFlag;
    AssetTime LastSearch;
    const std::string QueryReplaceString = "0000-00-00T00:00:00Z";
    BackObject ParseSearch(std::string &pjson);
    BackObject ParseTextXml(std::string &pXml, cAsset *pAsset);
    void ReplaceHtml(std::string &pstr);
    void DumpAsset(cAsset &pAsset);
    void IsSuccess(nlohmann::json &pJon, BackObject &pBack);
    std::string strApiKey;
    std::string GetApiKey();
    BackObject GetLanguage(std::string &strurl, cAsset *pAsset);
    BackObject GetBody(cAsset *pAsset);
    BackObject GetVideo(cAsset *pAsset);
    int PageNumber;
    int PageSize = 20;

    template <typename T>
    T GetJsonValue(nlohmann::json &pJson);

    static void *ProcessFunc(void *parg);
    void WorkList();

public:
    apapi();
    ~apapi();
    bool Initiliaze();
    BackObject GetAccountInfo();
    BackObject Search();
    void UpdateSearchTime(AssetTime &pAssetTime);
};