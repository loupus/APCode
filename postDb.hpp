#pragma once
#include <vector>
#include "Globals.hpp"
#include <libpq-fe.h>
#include "Asset.hpp"
#include "DBClasses.hpp"

enum PgFormats
{
    text = 0, //ascii text
    binary = 1 // binary
};

enum PgTypeOids
{
    oid_bool = 16,
    oid_char = 18,
    oid_int8 = 20,
    oid_int2 = 21,
    oid_int4 = 23,
    oid_text = 25,
    oid_json = 114,
    oid_xml = 142,
    oid_varchar = 1043,
    oid_date = 1082,
    oid_time = 1083,
    oid_timestamp = 1114,
    oid_timestamptz = 1184,
    oid_bit = 1560,
    oid_uuid = 2950
};

struct sParameters
{
    std::vector<char*> paramValues;
    std::vector<int> paramLengths;   
    std::vector<int> paramFormats;
    std::vector<Oid> paramOids;
};


class postDb
{
private:
    const char* conninfo;
    PGconn* conn;
    PGresult* res;
    std::string ConnStr;
    sParameters params;
    std::vector<RowData*> vd;
    void ResetLoad();
    
public:
    postDb(std::string pConnStr);
    ~postDb();
    BackObject Connect();
    void Disconnect();
    bool IsConnected();
    BackObject DoExecute(std::string pStatement);
    BackObject DoExecuteEx(std::string pStatement);
    void AddParam(char* pValue,int pSize, int pFormat, Oid otype, bool DoReset = false);

    void ClearParams();
    void PrintValues();
    void LoadAsset(cAsset* pAsset);   
    void Load();  
    void DumpLoaded();
    void SwapInteger(char* pValue, Oid otype);
    std::vector<RowData*>* GetLoaded();
    
};
