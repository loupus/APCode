#include <iostream>
#include <vector>


class Config
{
public:
	static std::string ConfigFile;
	/*
	static std::string apiKey ;
	static std::string apiRootUrl ;
	static std::string apiAccountUrl ;
	static std::string apiQueryUrl ;
	static std::string logFolder ;
	static std::string videoDownloadFolder ;


	static std::string dbServer;
	static std::string dbPort;
	static std::string dbName;
	static std::string dbUser;
	static std::string dbPass;


	static int HoursBefore ;
	static int DownloadWaitSeconds ;
//	static int SearchInterval;    		// in seconds
	static std::string lastSearchTime;
	static bool PersistDb;
	static std::string EgsFolder;
	*/

	static int AsInt(const char* pValueKey);
	static std::string AsStr(const char* pValueKey);
	static void SetValue(const char* pKey, const char* pValue);

	static bool ReadConfig();
	static bool WriteConfig();

	static bool ReadConfigEx();
	static bool WriteConfigEx();

};