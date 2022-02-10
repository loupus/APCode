#pragma once
#include <iostream>
#include <pthread.h>
#include "AssetQueue.hpp"
#include "HttpManager.hpp"
#include "DbManager.hpp"

class apapi
{
private:
    cAssetQueue Assets;
    cHttpManager ht;
    pthread_t thHandle;
    pthread_attr_t attr;
    AADb db;
    volatile bool StopFlag;
    pthread_mutex_t lock; // declare a lock
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
    BackObject GetNonCompletedAssets(int nitems);

    void WorkList();

public:
    apapi();
    ~apapi();
    bool Initiliaze();
    BackObject GetAccountInfo();
    BackObject Search();
    void UpdateSearchTime(AssetTime &pAssetTime);
    static void *ProcessFunc(void *parg);

    void Start();
    void Stop();
};