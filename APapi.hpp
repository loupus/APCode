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

public:
    apapi();
    ~apapi();
    bool Initiliaze();
    BackObject GetAccountInfo();
    BackObject Search();
    void UpdateSearchTime(AssetTime &pAssetTime);
};