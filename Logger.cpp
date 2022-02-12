#include <iomanip>
#include <locale>
#include <codecvt>
#include "Globals.hpp"
#include "Config.hpp"
#include "Logger.hpp"

std::ofstream Logger::lw;
std::stringstream Logger::wl;
std::string Logger::logfile;
std::string Logger::logtype;
std::mutex Logger::mtx;
std::vector<LogCallbackArg *> Logger::Callbacks;

void Logger::registerCallback(void *pobj, LogCallback pfunction, const std::string &pcallerID)
{
    LogCallbackArg *cba = new LogCallbackArg();
    cba->obj = pobj;
    cba->cb = pfunction;
    cba->ID = pcallerID;
    Callbacks.push_back(cba);
}
void Logger::unRegisterCallback(const std::string &pGuid)
{
    for (std::vector<LogCallbackArg *>::iterator it = Callbacks.begin(); it != Callbacks.end(); ++it)
    {
        if ((*it)->ID == pGuid)
        {
            Callbacks.erase(it);
            it--;
        }
    }
}

void Logger::WriteLog(std::string pLog, LogType pLogType)
{

    std::unique_lock<std::mutex> lck(mtx);

    time_t now = time(0);
    tm *ltm = localtime(&now);

    logfile.clear();
    logtype.clear();
    wl.clear();
    wl.str("");

    // logfile = "d:\\";

    if (Config::logFolder.empty())
        logfile = ".\\";
    else
        logfile = Config::logFolder;

    // wl << std::put_time(ltm, "%F");
    wl << std::put_time(ltm, "%Y-%m-%d");

    std::string asd = wl.str();
    logfile.append(wl.str());
    logfile.append(".txt");

    wl << " " << std::put_time(ltm, "%X");
    asd.clear();
    asd = wl.str();

    switch (pLogType)
    {
    case LogType::error:
    {
        logtype = "ERROR";
        break;
    }
    case LogType::warning:
    {
        logtype = "WARNING";
        break;
    }
    case LogType::info:
    {
        logtype = "INFO";
        break;
    }
    case LogType::userevent:
    {
        logtype = "USEREVENT";
        break;
    }
    }

    wl << " : " << logtype.c_str() << " : " << pLog.c_str();


    //std::codecvt_utf8<wchar_t> *ct = new std::codecvt_utf8<wchar_t>();
    //std::locale loc(std::locale(), ct);
    //lw.imbue(loc);

    try
    {
        lw.open(logfile.c_str(), std::ios::out | std::ios::app);
    }
    catch (std::ofstream::failure &e)
    {
        std::cerr << "Exception opening/reading/closing file\n";
    }

    try
    {   
        lw << wl.str() << std::endl;
        std::cout << wl.str() << std::endl;
    }
    catch (const std::exception &)
    {
        std::wcout << L"cannot output string" << std::endl;
    }
    FlushLog();
    lw.close();
    wl.clear();
    std::cout.flush();
}

void Logger::FlushLog()
{
    wl.flush();
    lw.flush();
}
