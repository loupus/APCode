
#include <iostream>
#include "nlohmann/json.hpp"
#include "Config.hpp"
#include "DbManager.hpp"

AADb::AADb()
{
    // conStr = "hostaddr =10.1.101.13 port=5432 dbname=agencydb user=postgres password=fender2 connect_timeout=5 client_encoding=UTF8";
}

AADb::~AADb()
{
}

void AADb::Initiliaze()
{
    conStr = "hostaddr =" + Config::AsStr("//Config/Database/dbServer") + " port=" + Config::AsStr("//Config/Database/dbPort") + " dbname=" + Config::AsStr("//Config/Database/dbName")
         + " user=" + Config::AsStr("//Config/Database/dbUser")+ " password=" + Config::AsStr("//Config/Database/dbPass") + " connect_timeout=5 client_encoding=UTF8";
    pdb.SetConStr(conStr);
}

BackObject AADb::SaveAsset(cAsset *pAsset)
{
    // todo null kontrolleri yap
    BackObject back;
    std::string q;
    //    q.append("Insert into assets(assetid,assetsource,title,headline,body,mediatype,mediafile,mediapath,assetstate,firsttime,lasttime,ondate)");
    //  q.append("values($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12)");

    q.append("CALL saveasset($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16)");

    back = pdb.Connect();
    if (back.Success == false)
        return back;

    int iagencysource, imediatype, istate, isuccess, iurgency;
    int *ipagencysource, *ipmediatype, *ipstate, *ipsuccess, *ipurgency;

    iagencysource = pAsset->AgencySource;
    imediatype = pAsset->MediaType;
    istate = pAsset->State;
    isuccess = pAsset->Success;
    iurgency = pAsset->urgency;

    ipagencysource = &iagencysource;
    ipmediatype = &imediatype;
    ipstate = &istate;
    ipsuccess = &isuccess;
    ipurgency = &iurgency;

    std::string strFirstTime = pAsset->FirstTime.asString();
    std::string strLastTime = pAsset->LastTime.asString();
    std::string strOndate = pAsset->OnDate.asString();

    pdb.AddParam((char *)pAsset->Id.c_str(), pAsset->Id.length(), PgFormats::binary, PgTypeOids::oid_varchar, true);
    pdb.AddParam((char *)ipagencysource, sizeof(int), PgFormats::binary, PgTypeOids::oid_int4);
    pdb.AddParam((char *)pAsset->HeadLine.c_str(), pAsset->HeadLine.length(), PgFormats::binary, PgTypeOids::oid_varchar);
    pdb.AddParam((char *)pAsset->Body.c_str(), pAsset->Body.length(), PgFormats::binary, PgTypeOids::oid_text);
    pdb.AddParam((char *)ipmediatype, sizeof(int), PgFormats::binary, PgTypeOids::oid_int4);
    pdb.AddParam((char *)pAsset->MediaFile.c_str(), pAsset->MediaFile.length(), PgFormats::binary, PgTypeOids::oid_varchar);
    pdb.AddParam((char *)pAsset->MediaPath.c_str(), pAsset->MediaPath.length(), PgFormats::binary, PgTypeOids::oid_varchar);
    pdb.AddParam((char *)ipstate, sizeof(int), PgFormats::binary, PgTypeOids::oid_int4);
    pdb.AddParam((char *)ipsuccess, sizeof(int), PgFormats::binary, PgTypeOids::oid_int4);
    pdb.AddParam((char *)strFirstTime.c_str(), strFirstTime.length(), PgFormats::text, PgTypeOids::oid_timestamp);
    pdb.AddParam((char *)strLastTime.c_str(), strLastTime.length(), PgFormats::text, PgTypeOids::oid_timestamp);
    pdb.AddParam((char *)strOndate.c_str(), strOndate.length(), PgFormats::text, PgTypeOids::oid_timestamp);
    pdb.AddParam((char *)pAsset->ErrMessage.c_str(), pAsset->ErrMessage.length(), PgFormats::binary, PgTypeOids::oid_text);
    pdb.AddParam((char *)pAsset->ProxyFile.c_str(), pAsset->ProxyFile.length(), PgFormats::binary, PgTypeOids::oid_varchar);
    pdb.AddParam((char *)pAsset->itemUrl.c_str(), pAsset->itemUrl.length(), PgFormats::binary, PgTypeOids::oid_varchar);
    pdb.AddParam((char *)ipurgency, sizeof(int), PgFormats::binary, PgTypeOids::oid_int4);

    back = pdb.DoExecuteEx(q);

    pdb.Disconnect();
    return back;
}

BackObject AADb::SaveAssets(std::list<cAsset> *pList)
{
    BackObject back;
    if (pList == nullptr)
    {
        back.Success = false;
        back.ErrDesc = "null parameter";
        return back;
    }
    std::string StrJson = GetJsonList(pList);

    // std::cout << "json str" << std::endl;
    // std::cout << StrJson << std::endl;
    // std::cout << "===========" << std::endl;

    if (StrJson.empty())
    {
        back.Success = false;
        back.ErrDesc = "empty json array";
        return back;
    }

    std::string q;
    q.append("CALL saveassets($1)");
    back = pdb.Connect();
    if (back.Success == false)
        return back;
    pdb.AddParam((char *)StrJson.c_str(), StrJson.length(), PgFormats::binary, PgTypeOids::oid_json, true);
    back = pdb.DoExecuteEx(q); // burda cörtliyo çünkü postgre notice gönderiyor
    pdb.Disconnect();
    return back;
}

BackObject AADb::GetAssets(std::list<cAsset> *pList, std::string pDate, int isuccess, int SourceAgency, int nitems)
{
    BackObject back;
    if (pList == nullptr)
    {
        back.ErrDesc = "GetAssets failed, null argument";
        back.Success = false;
        return back;
    }
    std::string q;
    q.append("select assetid,assetsource,headline,body,mediatype,mediafile,mediapath,assetstate,assetsuccess,");
    q.append("firsttime::character varying,lasttime::character varying,ondate::character varying, errmessage, proxyfile, itemurl,urgency from assets ");
    q.append("where ondate::date = $1 and assetsuccess = $2 and assetsource =$3");
    if (nitems > 0)
        q.append(" limit $4");
    //std::cout << q << std::endl;
    back = pdb.Connect();
    if (back.Success == false)
        return back;
    pdb.AddParam((char *)pDate.c_str(), pDate.length(), PgFormats::text, PgTypeOids::oid_timestamp, true);
    pdb.AddParam((char *)&isuccess, sizeof(int), PgFormats::binary, PgTypeOids::oid_int4);
    pdb.AddParam((char *)&SourceAgency, sizeof(int), PgFormats::binary, PgTypeOids::oid_int4);    
    if (nitems > 0)
        pdb.AddParam((char *)&nitems, sizeof(int), PgFormats::binary, PgTypeOids::oid_int4);
    back = pdb.DoExecuteEx(q);
    pdb.Disconnect();
    if (back.Success == true)
    {
        std::vector<RowData *> *vd = pdb.GetLoaded();
        if (vd)
        {
            back = PopulateMulti(vd, pList);
            // DumpAssets(pList);
        }
    }
    return back;
}

BackObject AADb::GetAsset(cAsset *pAsset)
{
    BackObject back;
    if (pAsset == nullptr)
    {
        back.ErrDesc = "GetAsset failed, null argument";
        back.Success = false;
        return back;
    }
    std::string q;
    q.append("select assetid,assetsource,headline,body,mediatype,mediafile,mediapath,assetstate,assetsuccess,");
    q.append("firsttime::character varying,lasttime::character varying,ondate::character varying,errmessage,proxyfile,itemurl,urgency from assets where assetid = $1 limit 1");
    back = pdb.Connect();
    if (back.Success == false)
        return back;
    pdb.AddParam((char *)pAsset->Id.c_str(), pAsset->Id.length(), PgFormats::binary, PgTypeOids::oid_varchar, true);
    back = pdb.DoExecuteEx(q);
    pdb.Disconnect();

    if (back.Success == true)
    {
        std::vector<RowData *> *vd = pdb.GetLoaded();
        if (vd)
        {
            RowData *rd = vd->at(0);
            back = PopulateSingle(rd, pAsset);
            // DumpAsset(pAsset);
        }
    }

    return back;
}

BackObject AADb::UpdateSearchTime(tm *lastSearchTime)
{
    BackObject back;
    std::string q;
    std::string strlastSearchTime = Globals::tmToStr(lastSearchTime);
    q.append("update definitions set aalastsearchtime = $1");
    pdb.AddParam((char *)strlastSearchTime.c_str(), strlastSearchTime.length(), PgFormats::text, PgTypeOids::oid_timestamp, true);
    back = pdb.Connect();
    if (back.Success == false)
        return back;
    back = pdb.DoExecuteEx(q);
    pdb.Disconnect();
    return back;
}

BackObject AADb::GetSearchTime(tm *lastSearchTime)
{
    BackObject back;

    std::string q;
    q.append("select aalastsearchtime from definitions");
    back = pdb.Connect();
    if (back.Success == false)
        return back;
    back = pdb.DoExecuteEx(q);
    pdb.Disconnect();

    if (back.Success == true)
    {
        std::vector<RowData *> *vd = pdb.GetLoaded();
        if (vd)
        {
            RowData *rd = vd->at(0);
            std::string strlstime = ((FieldData<std::string> *)rd->Fields[0])->GetValue();
            Globals::strToTm(lastSearchTime, strlstime);

            // std::istringstream ss(strlstime);
            // ss >> std::get_time(lastSearchTime, "%Y-%m-%d %H:%M:%S");
            //  back = PopulateSingle(rd, pAsset);
        }
    }

    return back;
}

void AADb::DumpAsset(cAsset *pAsset)
{
    std::cout << "Dumping Asset" << std::endl;
    std::cout << "-------------" << std::endl;
    std::cout << "AssetID : " << pAsset->Id << std::endl;
    std::cout << "Asset Source : " << pAsset->AgencySource << std::endl;
    std::cout << "Headline : " << pAsset->HeadLine << std::endl;
    std::cout << "Body : " << pAsset->Body << std::endl;
    std::cout << "MediaType : " << pAsset->MediaType << std::endl;
    std::cout << "MediaFile : " << pAsset->MediaFile << std::endl;
    std::cout << "MediaPath : " << pAsset->MediaPath << std::endl;
    std::cout << "AssetState : " << pAsset->State << std::endl;
    std::cout << "AssetSuccess : " << pAsset->Success << std::endl;
    std::cout << "FirstTime : " << pAsset->FirstTime.asString() << std::endl;
    std::cout << "LastTime : " << pAsset->LastTime.asString() << std::endl;
    std::cout << "OnDate : " << pAsset->OnDate.asString() << std::endl;
    std::cout << "ErrMessage : " << pAsset->ErrMessage << std::endl;
    std::cout << "ProxyFile : " << pAsset->ProxyFile << std::endl;
    std::cout << "ItemUrl : " << pAsset->itemUrl << std::endl;
    std::cout << "Urgency : " << pAsset->urgency << std::endl;
}

void AADb::DumpAssets(std::list<cAsset> *pAssets)
{
    for (auto i = pAssets->begin(); i != pAssets->end(); i++)
    {
        DumpAsset(&(*i));
        std::cout << "==============================================" << std::endl;
    }
}

BackObject AADb::PopulateSingle(RowData *rdp, cAsset *pAsset)
{
    BackObject back;
    if (pAsset == nullptr || rdp == nullptr)
    {
        back.ErrDesc = "Populate failed, null argument";
        back.Success = false;
        return back;
    }
    for (auto j : rdp->Fields)
    {

        if (j->fieldName == "assetid")
        {
            pAsset->Id = ((FieldData<std::string> *)j)->GetValue();
        }
        else if (j->fieldName == "assetsource")
        {
            pAsset->AgencySource = ((FieldData<int> *)j)->GetValue();
        }
        else if (j->fieldName == "headline")
        {
            pAsset->HeadLine = ((FieldData<std::string> *)j)->GetValue();
        }
        else if (j->fieldName == "body")
        {
            pAsset->Body = ((FieldData<std::string> *)j)->GetValue();
        }
        else if (j->fieldName == "mediatype")
        {
            pAsset->MediaType = ((FieldData<int> *)j)->GetValue();
        }
        else if (j->fieldName == "mediafile")
        {
            pAsset->MediaFile = ((FieldData<std::string> *)j)->GetValue();
        }
        else if (j->fieldName == "mediapath")
        {
            pAsset->MediaPath = ((FieldData<std::string> *)j)->GetValue();
        }
        else if (j->fieldName == "assetstate")
        {
            pAsset->State = ((FieldData<int> *)j)->GetValue();
        }
        else if (j->fieldName == "assetsuccess")
        {
            pAsset->Success = ((FieldData<int> *)j)->GetValue();
        }
        else if (j->fieldName == "firsttime")
        {
            pAsset->FirstTime.fromString(((FieldData<std::string> *)j)->GetValue().c_str());
        }
        else if (j->fieldName == "lasttime")
        {
            pAsset->LastTime.fromString(((FieldData<std::string> *)j)->GetValue().c_str());
        }
        else if (j->fieldName == "ondate")
        {
            pAsset->OnDate.fromString(((FieldData<std::string> *)j)->GetValue().c_str());
        }
        else if (j->fieldName == "errmessage")
        {
            pAsset->ErrMessage = ((FieldData<std::string> *)j)->GetValue();
        }
        else if (j->fieldName == "proxyfile")
        {
            pAsset->ProxyFile = ((FieldData<std::string> *)j)->GetValue();
        }
        else if (j->fieldName == "itemurl")
        {
            pAsset->itemUrl = ((FieldData<std::string> *)j)->GetValue();
        }
        else if (j->fieldName == "urgency")
        {
            pAsset->urgency = ((FieldData<int> *)j)->GetValue();
        }
    }

    return back;
}

BackObject AADb::PopulateMulti(std::vector<RowData *> *pRows, std::list<cAsset> *pAssets)
{
    BackObject back;
    if (pRows == nullptr || pAssets == nullptr)
    {
        back.ErrDesc = "Populate failed, null argument";
        back.Success = false;
        return back;
    }
    cAsset pAsset;
    RowData *prd = nullptr;
    for (auto i = pRows->begin(); i != pRows->end(); i++)
    {
        prd = *i;
        back = PopulateSingle(prd, &pAsset);
        pAssets->push_back(pAsset);
    }
    return back;
}

std::string AADb::GetJsonList(std::list<cAsset> *pAssets)
{
    nlohmann::json jlist = nlohmann::json::array();
    for (auto it = pAssets->begin(); it != pAssets->end(); it++)
    {
        jlist.push_back((it)->toJson());
    }
    return jlist.dump();
}
