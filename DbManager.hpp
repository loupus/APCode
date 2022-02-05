#pragma once
#include <list>
#include "Asset.hpp"
#include "postDb.hpp"
#include "DBClasses.hpp"

class AADb
{
private:
	postDb* pdb;
	std::string conStr = "";
public:

	AADb();
	~AADb();
	BackObject SaveAsset(cAsset *pAsset);
	BackObject SaveAssets(std::list<cAsset> *pList);
	BackObject GetAsset(cAsset *pAsset);
	BackObject GetAssets(std::list<cAsset> *pList, std::string pDate, int isuccess, int nitems=0);
	void DumpAsset(cAsset *pAsset);
	void DumpAssets(std::list<cAsset> *pAssets);
	BackObject PopulateSingle(RowData *rdp, cAsset* pAsset);
	BackObject PopulateMulti(std::vector<RowData*> *pRows, std::list<cAsset> *pAssets);
	std::string GetJsonList(std::list<cAsset> *pAssets);
	BackObject UpdateSearchTime(tm* lastSearchTime);
	BackObject GetSearchTime(tm* lastSearchTime);
	
};

