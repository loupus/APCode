#include <iostream>
#include "postDb.hpp"
#include "Logger.hpp"

postDb::postDb(std::string pConnStr)
{
    ConnStr = pConnStr;
    conn = nullptr;
}

postDb::~postDb()
{
    ClearParams();
    Disconnect();
    ResetLoad();
}

BackObject postDb::Connect()
{
    BackObject back;
    if (!IsConnected())
     {
        conn = PQconnectdb(ConnStr.c_str());
        if (PQstatus(conn) != CONNECTION_OK)
        {
            std::string strErr = PQerrorMessage(conn);
            back.Success = false;
            back.ErrDesc = "Connection Failed: " + (strErr);
        }
    }
    return back;
}

void postDb::Disconnect()
{
    if (conn)
    {
        PQfinish(conn);
        conn = nullptr;
    }
       
}

bool postDb::IsConnected()
{
    if (conn == nullptr)
        return false;
    if (PQstatus(conn) == ConnStatusType::CONNECTION_OK)
        return true;
    else
        return false;
}

BackObject postDb::DoExecute(std::string pStatement)
{
    BackObject back;
    back = Connect();
    if (back.Success)
    {
        res = PQexec(conn, pStatement.c_str());
        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            std::string strErr = PQresultErrorMessage(res);
            back.Success = false;
            back.ErrDesc = strErr;
        }
    }
    PQclear(res);
    return back;
}

BackObject postDb::DoExecuteEx(std::string pStatement)
{
    BackObject back;

    //char *strEscaped = PQescapeLiteral(conn, pStatement.c_str(), pStatement.length());

    res = PQexecParams(
        conn,
        pStatement.c_str(),
        params.paramValues.size(),
        params.paramOids.data(), // parametrelerin oid leri, nullsa pg karar verir
        params.paramValues.data(), 
        params.paramLengths.data(),
        params.paramFormats.data(),
        PgFormats::binary // donus verisi binary olsun
    );

    bool IsRow = false;
    bool IsScaler = false;

    switch (PQresultStatus(res))
    {
    case PGRES_COMMAND_OK:
    {

        break;
    }
    case PGRES_TUPLES_OK:
    {
        IsRow = true;
        break;
    }

    case PGRES_EMPTY_QUERY:
    {
        std::string strErr = PQresultErrorMessage(res);
        back.Success = false;
        back.ErrDesc = "The string sent to the server was empty, " + (strErr);
        break;
    }
    case PGRES_COPY_OUT:
    {

        break;
    }
    case PGRES_COPY_IN:
    {

        break;
    }
    case PGRES_BAD_RESPONSE:
    {
        std::string strErr = PQresultErrorMessage(res);
        back.Success = false;
        back.ErrDesc = "The server's response was not understood, " + (strErr);
        break;
    }
    case PGRES_NONFATAL_ERROR:
    {
        std::string strErr = PQresultErrorMessage(res);
        back.Success = false;
        back.ErrDesc = "A nonfatal error (a notice or warning) occurred, " + (strErr);
        break;
    }
    case PGRES_FATAL_ERROR:
    {
        std::string strErr = PQresultErrorMessage(res);
        back.Success = false;
        back.ErrDesc = "A fatal error occurred, " + (strErr);
        break;
    }
    case PGRES_COPY_BOTH:
    {

        break;
    }
    case PGRES_SINGLE_TUPLE:
    {
        IsScaler = true;
        break;
    }
    }

    if (back.Success == false)
    {
        PQclear(res);
        return back;
    }

    //PrintValues();
     // std::vector<IDbData*> data;
    if (IsRow)
    {
        Load();
       // DumpLoaded();
    }
    if(IsScaler)
    {
        //todo do something
    }

    PQclear(res);
    return back;
}

void postDb::AddParam(char *pValue, int pSize, int pFormat, Oid otype, bool DoReset)
{
    if (DoReset)
        ClearParams();

    if (otype == PgTypeOids::oid_int2 || otype == PgTypeOids::oid_int4 || otype == PgTypeOids::oid_int8)
        SwapInteger(pValue, otype);

    // params.paramValues.push_back((char *)pValue);
    params.paramValues.push_back(reinterpret_cast<char *>(pValue));
    params.paramLengths.push_back(pSize);
    params.paramFormats.push_back(pFormat);
    params.paramOids.push_back(otype);
}

void postDb::ClearParams()
{
    
    params.paramValues.clear();
    params.paramLengths.clear();
    params.paramFormats.clear();
    params.paramOids.clear();
}

void postDb::PrintValues()
{
    char *rowsAffected = PQcmdTuples(res);
    int nrows = 0;
    int nfields = 0;
    char *fname = nullptr;
    Oid fttype;
    Oid ftype;
    int format;
    int vsize;
    char *value = nullptr;
    bool isnull = false;
    int vlen = 0;

    std::string strra(rowsAffected);

    Logger::WriteLog("RowsAffected: " + (strra));

    // std::cout << "Rows Affected: " << strra.c_str() << std::endl;
    // printf("RowsAffected: %s\n", rowsAffected);
    //  fprintf(stdout, "RowsAffected: %s\n", rowsAffected);

    nrows = PQntuples(res);
    nfields = PQnfields(res);
    for (int i = 0; i < nrows; i++)
    {
        for (int j = 0; j < nfields; j++)
        {
            fname = PQfname(res, j);
            fttype = PQftable(res, j);
            format = PQfformat(res, j);
            ftype = PQftype(res, j);
            vsize = PQfsize(res, j);
            PQgetisnull(res, i, j) == 1 ? isnull = true : isnull = false;
            value = PQgetvalue(res, i, j);
            vlen = PQgetlength(res, i, j);

            //printf("%s\n", str);
            std::string strfname(fname);

            Logger::WriteLog("FieldName: " + strfname);
            Logger::WriteLog("FieldTableType: " + (std::to_string(fttype)));
            Logger::WriteLog("FieldFormat: " + (std::to_string(format)));
            Logger::WriteLog("FieldType: " + (std::to_string(ftype)));
            Logger::WriteLog("ValueSize: " + (std::to_string(vsize)));
            Logger::WriteLog("isnull: " + (std::to_string(isnull)));
            Logger::WriteLog("ValueLength: " + (std::to_string(vlen)));
            if (ftype == 25) // text ise
            {
                std::string strValue(value);
                Logger::WriteLog("Value: " + strValue);
            }
            else if (ftype == 23) // int4 ise
            {
                uint32_t *pIntValue = reinterpret_cast<uint32_t *>(value);
                uint32_t intValue = _byteswap_ulong(*pIntValue);
                ;
                Logger::WriteLog("Value: " + (std::to_string(intValue)));
            }

         
        }
    }

    PQprintOpt pqp;
    pqp.header = 1;
    pqp.align = 1;
    pqp.html3 = 0;
    pqp.expanded = 0;
    pqp.pager = 0;
    pqp.fieldName = NULL;
    PQprint(stdout, res, &pqp);
}

void postDb::LoadAsset(cAsset *pAsset)
{
    int nrows = 0;
    int nfields = 0;
    nrows = PQntuples(res);
    nfields = PQnfields(res);
    char *fname, *value;
    Oid ftype;
    for (int i = 0; i < nrows; i++)
    {
        for (int j = 0; j < nfields; j++)
        {
            fname = PQfname(res, j);
            value = PQgetvalue(res, i, j);
            ftype =PQftype(res, j);
            if (strcmp(fname, "assetid") == 0)
            {
                pAsset->Id = value;
            }
            else if (strcmp(fname, "assetsource") == 0)
            {
                SwapInteger(value, ftype);
                pAsset->AgencySource = *(reinterpret_cast<unsigned long *>(value));
            }
            else if (strcmp(fname, "headline") == 0)
            {
                pAsset->HeadLine = value;
            }
            else if (strcmp(fname, "mediatype") == 0)
            {
                SwapInteger(value, ftype);
                pAsset->AgencySource = *(reinterpret_cast<unsigned long *>(value));
            }
            else if (strcmp(fname, "mediafile") == 0)
            {
                pAsset->MediaFile = value;
            }
            else if (strcmp(fname, "mediapath") == 0)
            {
                pAsset->MediaPath = value;
            }
            else if (strcmp(fname, "assetstate") == 0)
            {
                SwapInteger(value, ftype);
                pAsset->State = *(reinterpret_cast<unsigned long *>(value));
            }
            else if (strcmp(fname, "firsttime") == 0)
            {
                pAsset->FirstTime.fromString(value);
            }
            else if (strcmp(fname, "lasttime") == 0)
            {
                pAsset->LastTime.fromString(value);
            }
            else if (strcmp(fname, "ondate") == 0)
            {
                pAsset->OnDate.fromString(value);
            }
            else if (strcmp(fname, "errmessage") == 0)
            {
                pAsset->ErrMessage = value;
            }
        }
    }
}

void postDb::SwapInteger(char *pValue, Oid otype)
{
    switch (otype)
    {
    case PgTypeOids::oid_int2:
    {
        unsigned short tmp = 0;
        tmp = _byteswap_ushort(*(reinterpret_cast<unsigned short *>(pValue)));
        *(unsigned short *)pValue = tmp;
        break;
    }
    case PgTypeOids::oid_int4:
    {
        unsigned long tmp = 0;
        tmp = _byteswap_ulong(*(reinterpret_cast<unsigned long *>(pValue)));
        *(unsigned long *)pValue = tmp;
        break;
    }
    case PgTypeOids::oid_int8:
    {
        unsigned __int64 tmp = 0;
        tmp = _byteswap_uint64(*(reinterpret_cast<unsigned __int64 *>(pValue)));
        *(unsigned __int64 *)pValue = tmp;
        break;
    }
    }
}

void postDb::Load()
{
  

    ResetLoad();

    int nrows = 0;
    int nfields = 0;   
    nrows = PQntuples(res);
    nfields = PQnfields(res);
    char *fname, *value;
    Oid ftype;
    IFieldData *dd;
   
    for (int i = 0; i < nrows; i++)
    {
        RowData *rd = new RowData();
        rd->RowNumber = i;
        for (int j = 0; j < nfields; j++)
        {

            fname = PQfname(res, j);
            value = PQgetvalue(res, i, j);
            ftype = PQftype(res, j);
           
            switch (ftype)
            {
                case PgTypeOids::oid_int2:
                {
                    dd = new FieldData<short>();
                    dd->fieldName = fname;
                    dd->dbType = dbRetType::Short;
                    dd->fnumber = j;
                    SwapInteger(value, PgTypeOids::oid_int2);
                    short sint = *(reinterpret_cast<unsigned short *>(value));                   
                    ((FieldData<short>*)(dd))->SetValue(sint);
                    break;
                }
                case PgTypeOids::oid_int4:
                {
                    dd = new FieldData<int>();
                    dd->fieldName = fname;
                    dd->dbType = dbRetType::Int;
                    dd->fnumber = j;
                    SwapInteger(value, PgTypeOids::oid_int4);
                    int sint = *(reinterpret_cast<unsigned long *>(value));                   
                    ((FieldData<int>*)dd)->SetValue(sint);
                    break;
                }
                case PgTypeOids::oid_int8:
                {
                    dd = new FieldData<int64_t>();
                    dd->fieldName = fname;
                    dd->dbType = dbRetType::Int64;
                    dd->fnumber = j;
                    SwapInteger(value, PgTypeOids::oid_int8);
                    int64_t sint = *(reinterpret_cast<unsigned __int64 *>(value));                  
                     ((FieldData<int64_t>*)dd)->SetValue(sint);
                    break;
                }
                case PgTypeOids::oid_varchar:
                case PgTypeOids::oid_text:
                case PgTypeOids::oid_timestamp:
                {               
                    dd = new FieldData<std::string>();
                    dd->fieldName = fname;
                    dd->dbType = dbRetType::String;
                    dd->fnumber = j;
                    ((FieldData<std::string>*)dd)->SetValue(value);
                    break;
                }
            }
            rd->Fields.push_back(dd);        
        }

        vd.push_back(rd);
    }
}

void postDb::DumpLoaded()
{
  
    for(auto i : vd)
    {
         std::cout << "RowNumber: " << i->RowNumber << std::endl;
        for(auto j : i->Fields)
        {
           std::cout << j->fieldName << " : " << j->GetStrValue();
        }
        std::cout << std::endl << std::endl;
        //std::cout << "FieldName: " << (*i)->fieldName << " \t Type: " << (*i)->dbType  << " \t Value: " <<  ((DbData<std::string>*)(*i))->GetStrValue() << std::endl;
         //std::cout << "FieldName: " << (*i)->fieldName << " \t Type: " << (*i)->dbType  << " \t Value: " <<  (*i)->GetStrValue() << std::endl;
    }
}

void postDb::ResetLoad()
{
    
      for(auto i:vd)
      {
          for(auto j: i->Fields)
          {
              delete j;
          }
          i->Fields.clear();
          i->Fields.shrink_to_fit();
          delete i;
      }
        vd.clear();
        vd.shrink_to_fit();
    
}

std::vector<RowData*>* postDb::GetLoaded()
{
    return &vd;
}
