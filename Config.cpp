#include <filesystem>
#include "pugixml.hpp"
#include "Globals.hpp"
#include "Config.hpp"
#include "configDic.hpp"

/*
std::string Config::dbServer = "10.1.101.13";
std::string Config::dbPort = "5432";
std::string Config::dbName = "agencydb";
std::string Config::dbUser = "postgres";
std::string Config::dbPass = "fender2";

std::string Config::apiUser = "hakansoyalp";
std::string Config::apiPass = "uzaKden1z";
std::string Config::apiAutorizationStr = "";
std::string Config::apiRootUrl = "https://api.aa.com.tr";
std::string Config::apiSeachUrl = "/abone/search";
std::string Config::apiDiscoverUrl = "/abone/discover/tr_TR";
std::string Config::apiSubscriptionUrl = "/abone/subscription";
std::string Config::videoDownloadFolder = "\\\\hansel.sistem.turkmedya.local\\AGENCIES\\";
std::string Config::logFolder = "";
int Config::HoursBefore = -1;
int Config::DownloadWaitSeconds = 1;
*/

/*
std::string Config::dbServer;
std::string Config::dbPort;
std::string Config::dbName;
std::string Config::dbUser;
std::string Config::dbPass;

std::string Config::apiKey;
std::string Config::apiRootUrl;
std::string Config::apiAccountUrl;
std::string Config::apiQueryUrl;
std::string Config::logFolder;
std::string Config::videoDownloadFolder;
std::string Config::EgsFolder;
int Config::DownloadWaitSeconds = 30;
std::string Config::lastSearchTime;
bool Config::PersistDb = false;
*/

std::string Config::ConfigFile = ".\\AP_AjansConfig.xml";

/*
bool Config::ReadConfig()
{
	bool back = false;
	pugi::xml_document dconfig;
	pugi::xml_parse_result result = dconfig.load_file(ConfigFile.c_str());

	if (!result)
	{
		std::cout << "Loading config file failed !" << std::endl;
		std::cout << result.description() << std::endl;
		return back;
	}

	pugi::xpath_node node = dconfig.select_node("//Config/apiKey");
	if (!node)
	{
		std::cout << "apiKey not found on config !" << std::endl;
		return back;
	}
	apiKey = node.node().text().as_string();

	//===============================================================
	node = dconfig.select_node("//Config/apiRootUrl");
	if (!node)
	{
		std::cout << "apiRootUrl not found on config !" << std::endl;
		return back;
	}
	apiRootUrl = node.node().text().as_string();

	//===============================================================
	node = dconfig.select_node("//Config/apiAccountUrl");
	if (!node)
	{
		std::cout << "apiAccountUrl not found on config !" << std::endl;
		return back;
	}
	apiAccountUrl = node.node().text().as_string();

	//===============================================================
	node = dconfig.select_node("//Config/apiQueryUrl");
	if (!node)
	{
		std::cout << "apiQueryUrl not found on config !" << std::endl;
		return back;
	}
	apiQueryUrl = node.node().text().as_string();

	//===============================================================
	node = dconfig.select_node("//Config/videoDownloadFolder");
	if (!node)
	{
		std::cout << "videoDownloadFolder not found on config !" << std::endl;
		return back;
	}
	videoDownloadFolder = node.node().text().as_string();

	//===============================================================
	node = dconfig.select_node("//Config/logFolder");
	if (!node)
	{
		std::cout << "logFolder not found on config ! Default in use: empty" << std::endl;
		logFolder = "";
	}
	else
		logFolder = node.node().text().as_string();

	//===============================================================
	node = dconfig.select_node("//Config/dbServer");
	if (!node)
	{
		std::cout << "dbServer not found on config !" << std::endl;
		return back;
	}
	dbServer = node.node().text().as_string();

	//===============================================================
	node = dconfig.select_node("//Config/dbPort");
	if (!node)
	{
		std::cout << "dbPort not found on config !" << std::endl;
		return back;
	}
	dbPort = node.node().text().as_string();

	//===============================================================
	node = dconfig.select_node("//Config/dbName");
	if (!node)
	{
		std::cout << "dbName not found on config !" << std::endl;
		return back;
	}
	dbName = node.node().text().as_string();

	//===============================================================
	node = dconfig.select_node("//Config/dbUser");
	if (!node)
	{
		std::cout << "dbUser not found on config !" << std::endl;
		return back;
	}
	dbUser = node.node().text().as_string();

	//===============================================================
	node = dconfig.select_node("//Config/dbPass");
	if (!node)
	{
		std::cout << "dbPass not found on config !" << std::endl;
		return back;
	}
	dbPass = node.node().text().as_string();

	//===============================================================
	node = dconfig.select_node("//Config/DownloadWaitSeconds");
	if (!node)
	{
		std::cout << "DownloadWaitSeconds not found on config ! default in use:" << std::to_string(DownloadWaitSeconds) << std::endl;
	}
	else
		DownloadWaitSeconds = node.node().text().as_int();

	//===============================================================
	node = dconfig.select_node("//Config/lastSearchTime");
	if (!node)
	{
		time_t now = time(0);
		tm *ltm = localtime(&now);
		ltm->tm_hour -= 1;
		mktime(ltm);
		lastSearchTime = Globals::tmToStr(ltm);
		std::cout << "lastSearchTime not found on config ! default in use: " << lastSearchTime << std::endl;
	}
	else
		lastSearchTime = node.node().text().as_string();

	//===============================================================
	//	node = dconfig.select_node("//Config/SearchInterval");
	// if (!node)
	//{
	//		std::cout << "SearchInterval not found on config ! default in use: " << SearchInterval << std::endl;
	//}
	// else
	//		SearchInterval = node.node().text().as_int();

	//===============================================================
	node = dconfig.select_node("//Config/PersistDb");
	if (!node)
	{
		std::cout << "PersistDb not found on config ! default in use: " << PersistDb << std::endl;
	}
	else
		PersistDb = node.node().text().as_bool();

	//===============================================================
	node = dconfig.select_node("//Config/EgsFolder");
	if (!node)
	{
		std::cout << "EgsFolder not found on config! Egs xml will not be generated" << std::endl;
	}
	else
	{
		EgsFolder = node.node().text().as_string();
		if (!std::filesystem::exists(EgsFolder))
		{
			EgsFolder.clear();
			std::cout << "EgsFolder not exists! Egs xml will not be generated" << std::endl;
		}
	}

	back = true;
	return back;
}

bool Config::WriteConfig()
{
	bool back = false;
	pugi::xml_document dconfig;
	auto root = dconfig.append_child("Config");

	pugi::xml_node tmp;
	tmp = root.append_child("apiKey");
	back = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(apiKey.c_str());

	tmp = root.append_child("apiRootUrl");
	back = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(apiRootUrl.c_str());

	tmp = root.append_child("apiAccountUrl");
	back = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(apiAccountUrl.c_str());

	tmp = root.append_child("apiQueryUrl");
	back = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(apiQueryUrl.c_str());

	tmp = root.append_child("videoDownloadFolder");
	back = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(videoDownloadFolder.c_str());

	tmp = root.append_child("logFolder");
	back = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(logFolder.c_str());

	tmp = root.append_child("dbServer");
	back = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(dbServer.c_str());

	tmp = root.append_child("dbPort");
	back = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(dbPort.c_str());

	tmp = root.append_child("dbName");
	back = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(dbName.c_str());

	tmp = root.append_child("dbUser");
	back = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(dbUser.c_str());

	tmp = root.append_child("dbPass");
	back = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(dbPass.c_str());

	tmp = root.append_child("DownloadWaitSeconds");
	back = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(std::to_string(DownloadWaitSeconds).c_str());

	tmp = root.append_child("lastSearchTime");
	back = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(lastSearchTime.c_str());

	// tmp = root.append_child("SearchInterval");
	// back = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(std::to_string(SearchInterval).c_str());

	tmp = root.append_child("PersistDb");
	int iPersisDb = PersistDb == true ? 1 : 0;
	back = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(std::to_string(iPersisDb).c_str());

	tmp = root.append_child("EgsFolder");
	back = tmp.append_child(pugi::xml_node_type::node_pcdata).set_value(EgsFolder.c_str());

	back = dconfig.save_file(ConfigFile.c_str(), PUGIXML_TEXT("  "));

	if (!back)
	{
		std::cout << "failed to write config !" << std::endl;
	}

	return back;
}
*/

bool Config::ReadConfigEx()
{
	bool back = true;
	if (!std::filesystem::exists(ConfigFile.c_str()))
	{
		std::cout << "Config file does not exist, creating an empty one";
		back = WriteConfigEx();
		return back;
	}
	pugi::xml_document dconfig;
	pugi::xpath_node xnode;
	pugi::xml_parse_result result;
	result = dconfig.load_file(ConfigFile.c_str());

	if (!result)
	{
		std::cout << "Loading config file failed !" << std::endl;
		std::cout << result.description() << std::endl;
		back = false;
		return back;
	}

	for (auto &item : configMap)
	{
		xnode = dconfig.select_node(item.first.c_str());
		if (!xnode)
		{
			std::cout << item.first.c_str() << " not found on config !" << std::endl;
			back = false;
		}
		else
		{
			item.second = xnode.node().text().as_string();
		}
	}
	return back;
}

bool Config::WriteConfigEx()
{
	bool back = true;
	size_t configTemplateSize = sizeof(configTemplate);
	pugi::xml_document dconfig;
	pugi::xpath_node xnode;
	pugi::xml_parse_result result;
	result = dconfig.load_buffer(configTemplate, configTemplateSize);

	if (!result)
	{
		std::cout << "Loading config file failed !" << std::endl;
		std::cout << result.description() << std::endl;
		back = false;
		return back;
	}

	for (auto &item : configMap)
	{
		xnode = dconfig.select_node(item.first.c_str());
		if (!xnode)
		{
			std::cout << item.first.c_str() << " not found on temp config !" << std::endl;
			back = false;
		}
		else
		{

			back = xnode.node().text() = (item.second.c_str());
		}
	}

	back = dconfig.save_file(ConfigFile.c_str(), PUGIXML_TEXT("  "));

	if (!back)
	{
		std::cout << "failed to write config !" << std::endl;
	}

	return back;
}

int Config::AsInt(const char *pValueKey)
{
	int back = 0;
	if (pValueKey == nullptr)
		return back;
	try
	{
		back = stoi(configMap[pValueKey]);
	}
	catch (...)
	{
	}

	return back;
}

std::string Config::AsStr(const char *pValueKey)
{
	std::string back;
	back = configMap[pValueKey];
	return back;
}

void Config::SetValue(const char *pKey, const char *pValue)
{
	if(pKey == nullptr || pValue == nullptr) return;
	configMap[pKey] = pValue;
}