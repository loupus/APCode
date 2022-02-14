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
  AADb db;

  pthread_t thHandle;
  pthread_cond_t is_zero;
  pthread_mutex_t mutex;
  bool StopFlag;
  bool IsComplete;
  AssetTime LastSearch;
  AssetTime NextSearch;
  std::string NextPage;
  int PageSize = 20;

  const std::string QueryReplaceString = "0000-00-00T00:00:00Z";
  BackObject ParseSearch(std::string &pJson);
  BackObject ParseTextXml(std::string &pXml, cAsset *pAsset);

  void DumpAsset(cAsset &pAsset);
  void IsSuccess(nlohmann::json &pJson, BackObject &pBack);
  std::string strApiKey;
  std::string GetApiKey();
  BackObject GetLanguage(std::string &pStrUrl, cAsset *pAsset);
  BackObject GetBody(cAsset *pAsset);
  BackObject GetVideo(cAsset *pAsset);
  void PickVideoLink(nlohmann::json &pJson, cAsset *pAsset);

  template <typename T>
  T GetJsonValue(nlohmann::json &pJson);
  BackObject GetNonCompletedAssets(int nitems);
  // void UpdateSearchTime(AssetTime &pAssetTime);

  void UpdateSearchTime();
  void WorkList();

public:
  apapi();
  ~apapi();
  bool Initiliaze();
  BackObject GetAccountInfo();
  BackObject Search();
  BackObject GenerateNewsML(cAsset *pAsset);
  static void *ProcessFunc(void *parg);
  void ReplaceHtml(std::string &pstr);

  void Start();
  void Stop();
};