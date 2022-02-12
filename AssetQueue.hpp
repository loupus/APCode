#pragma once
#include <list>
#include <mutex>
#include "Globals.hpp"
#include "Asset.hpp"

class cAssetQueue
{
private:
	std::list<cAsset> q;
	std::list<cAsset>::iterator it;
	std::mutex mtx;
	size_t BufCapacity = 200;

public:
	cAssetQueue();
	~cAssetQueue();

	BackObject Add(const cAsset &value);
	void Remove(cAsset &value);
	void RemoveWithSuccess(int pState);
	void RemoveCompleted();
	int GetSize();
	std::list<cAsset> *GetPointer();
	cAsset Front();
	cAsset *GetFirstWithStat(int pState);
	cAsset *GetFirstWithSState(int pSsuccess);
	int GetBuffSize();
	cAsset *GetWithID(std::string pAssetID);

};
