#pragma once
#include <ctime>
#include <iostream>
#include <fstream>
#include <mutex>
#include <vector>
#include <sstream>    // for wstringstream


typedef void(*LogCallback)(void*, std::string&);

struct LogCallbackArg
{
    LogCallback cb;
    void* obj;
    std::string ID;
};

enum class LogType
{
    info,
    error,
    warning,
    userevent
};

class Logger
{

    static std::mutex mtx;
    static std::vector<LogCallbackArg*> Callbacks;
    static std::stringstream wl;
    static std::ofstream  lw;
    static std::string logfile;
    static std::string logtype;

public:

    static void registerCallback(void* pobj, LogCallback pfunction, const std::string& pcallerID);;

    static void unRegisterCallback(const std::string& pGuid);

    static void WriteLog(std::string pLog, LogType pLogType = LogType::info);

    static void FlushLog();

};

