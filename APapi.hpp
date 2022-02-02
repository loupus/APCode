#pragma once
#include <iostream>
#include "AssetQueue.hpp"
#include "HttpManager.hpp"

class apapi
{
    private:
	cAssetQueue Assets;
	cHttpManager* ht;
    volatile bool StopFlag;
    AssetTime LastSearch;

    public:
    apapi();
    ~apapi();
    bool Initiliaze();
    BackObject GetAccountInfo();
    void GetNextSearchTime(AssetTime& pAssetTime);
    
};