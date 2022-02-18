#pragma once
#include <iostream>
#include <map>

std::map<std::string, std::string> configMap =
    {
        {"//Config/Database/dbServer", ""},
        {"//Config/Database/dbPort", ""},
        {"//Config/Database/dbName", ""},
        {"//Config/Database/dbUser", ""},
        {"//Config/Database/dbPass", ""},

        {"//Config/Agencies/AP/Run", ""},
        {"//Config/Agencies/AP/apiKey", ""},
        {"//Config/Agencies/AP/apiRootUrl", ""},
        {"//Config/Agencies/AP/apiAccountUrl", ""},
        {"//Config/Agencies/AP/apiQueryUrl", ""},
        {"//Config/Agencies/AP/videoDownloadFolder", ""},
        {"//Config/Agencies/AP/SleepSeconds", ""},
        {"//Config/Agencies/AP/lastSearchTime", ""},
        {"//Config/Agencies/AP/PersistDb", ""},
        {"//Config/Agencies/AP/EgsFolder", ""},

        {"//Config/Agencies/AA/Run", ""},
        {"//Config/Agencies/AA/apiUser", ""},
        {"//Config/Agencies/AA/apiPass", ""},
        {"//Config/Agencies/AA/apiRootUrl", ""},
        {"//Config/Agencies/AA/selCategories", ""},
        {"//Config/Agencies/AA/selPriorities", ""},
        {"//Config/Agencies/AA/selPackages", ""},
        {"//Config/Agencies/AA/selTypes", ""},

        {"//Config/LogFolder", ""}

};

const char configTemplate[] = R"(
<?xml version="1.0"?>
<Config>
  <Database>
    <dbServer></dbServer>
    <dbPort></dbPort>
    <dbName></dbName>
    <dbUser></dbUser>
    <dbPass></dbPass>
  </Database>
  <Agencies>
    <AP>
      <Run></Run>
      <apiKey></apiKey>
      <apiAccountUrl></apiAccountUrl>
      <apiQueryUrl></apiQueryUrl>
      <apiRootUrl></apiRootUrl>
      <videoDownloadFolder></videoDownloadFolder>
      <SleepSeconds></SleepSeconds>
      <lastSearchTime></lastSearchTime>
      <PersistDb></PersistDb>
      <EgsFolder></EgsFolder>
    </AP>
    <AA>
      <Run></Run>
      <apiUser></apiUser>
      <apiPass></apiPass>
      <apiRootUrl></apiRootUrl>
      <selCategories></selCategories>
      <selPriorities></selPriorities>
      <selPackages></selPackages>
      <selTypes></selTypes>
      <videoDownloadFolder></videoDownloadFolder>
      <SleepSeconds></SleepSeconds>
      <lastSearchTime></lastSearchTime>
      <PersistDb></PersistDb>
      <EgsFolder></EgsFolder>
    </AA>
  </Agencies>
  <LogFolder></LogFolder>
</Config>
)";

