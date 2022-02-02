#include "AssetQueue.hpp"

cAssetQueue::cAssetQueue()
{
}

cAssetQueue::~cAssetQueue()
{
	q.clear();
}

BackObject cAssetQueue::Add(const cAsset &value)
{
	BackObject back;
	bool CanAdd = true;
	if (q.size() < BufCapacity)
	{
		for (it = q.begin(); it != q.end(); it++)
		{
			if (value == *it)
			{
				CanAdd = false;
				break;
			}
		}
		if (CanAdd)
		{
			std::unique_lock<std::mutex> lock(mtx);
			q.push_back(value);
			lock.unlock();
			back.Success = true;
		}
		else
		{
			back.Success = false;
			back.ErrDesc = "Asset already in list";
		}
	}
	else
	{
		back.Success = false;
		back.ErrDesc = "Asset Queue is full";
	}
	return back;
}

void cAssetQueue::Remove(cAsset &value)
{
	std::unique_lock<std::mutex> lock(mtx);
	q.remove(value);	
	lock.unlock();
}

void cAssetQueue::RemoveWithSuccess(int pState)
{
	std::unique_lock<std::mutex> lock(mtx);
	q.remove_if([=](cAsset a){return a.Success == pState;});	
	lock.unlock();
}

void cAssetQueue::RemoveCompleted()
{
	std::unique_lock<std::mutex> lock(mtx);
	q.remove_if([=](cAsset a){return a.IsDeleted == true;});
	q.remove_if([=](cAsset a){return a.Success ==  AssetSuccess::asSuc_Completed && a.MediaType == MediaTypes::mt_text;});
	q.remove_if([=](cAsset a){return a.Success ==  AssetSuccess::asSuc_Completed && a.MediaType == MediaTypes::mt_video && a.State == AssetState::astat_PROXY_CREATED;});
	lock.unlock();
}

int cAssetQueue::GetSize()
{
	return q.size();
}

std::list<cAsset> *cAssetQueue::GetPointer()
{
	std::list<cAsset> *back = nullptr;
	back = &q;
	return back;
}

cAsset cAssetQueue::Front()
{
	cAsset back = q.front();
	return back;
}

cAsset* cAssetQueue::GetFirstWithStat(int pState)
{
	for (it = q.begin(); it != q.end(); it++)
	{
		if (it->State == pState && it->Success == AssetSuccess::asSuc_NoProblem && it->IsDeleted == false) 
		{
			return &(*it);
		}
	}
	return nullptr;
}

cAsset* cAssetQueue::GetFirstWithSState(int pSsuccess)
{
	for (it = q.begin(); it != q.end(); it++)
	{
		if (it->Success == AssetSuccess::asSuc_NoProblem && it->IsDeleted == false)
		{
			return &(*it);
		}
	}
	return nullptr;
}

int cAssetQueue::GetBuffSize()
{
	return BufCapacity;
}

cAsset* cAssetQueue::GetWithID(std::string pAssetID)
{
	it = std::find_if(q.begin(), q.end(),[&pAssetID](const cAsset& asset){return asset.Id == pAssetID;});
	if(it != q.end())
		return &(*it);
	else
		return nullptr;
}


