
#include "winsock2.h"
#include "windows.h"
#include <iostream>
#include "Config.hpp"
#include "APapi.hpp"
#include <libpq-fe.h>
#include "pugixml.hpp"

void TestAccountInfo()
{
    BackObject back;
    apapi ap;
    if (ap.Initiliaze())
    {
        back = ap.GetAccountInfo();
        if (!back.Success)
            std::cout << back.ErrDesc << std::endl;
    }
}

void TestSearch()
{
    BackObject back;
    apapi ap;
    if (!ap.Initiliaze())
    {
        std::cout << "ap api initiliaze failed!" << std::endl;
        return;
    }
    back = ap.Search();
    if (!back.Success)
        std::cout << back.ErrDesc << std::endl;
    // else
    //    std::cout << back.StrValue << std::endl;
}

apapi *ap;
void TestProcess()
{
    BackObject back;

    ap = new apapi();
    if (!ap->Initiliaze())
    {
        std::cout << "ap api initiliaze failed!" << std::endl;
        return;
    }
    ap->Start();

    // if(!back.Success)
    //   std::cout << back.ErrDesc << std::endl;
}

void TestXml()
{
    apapi ap;
    if (!ap.Initiliaze())
    {
        std::cout << "ap api initiliaze failed!" << std::endl;
        return;
    }

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("D:\\nitf.xml");
    if (!result)
    {
        std::string strErrdesc(result.description());
    }
    pugi::xpath_node node = doc.select_node("//nitf/body/body.content/block");
    if (!node)
    {
    }

    std::stringstream ss;
    node.node().print(ss);
    std::string bodydata = ss.str(); // todo burda bazen sapıtıyor
    ap.ReplaceHtml(bodydata);
    std::cout << bodydata << std::endl;
}

int LocalSettingsC(std::string pLocal)
{
    // C.UTF-8   olmuyor
    char *ss;

    std::cout << "Current LCALL: " << setlocale(LC_ALL, NULL) << std::endl;
    std::cout << "Current LC_CTYPE: " << setlocale(LC_CTYPE, NULL) << std::endl;
    std::cout << "Setting C local Information to " << pLocal << std::endl;
    ss = nullptr;
    ss = setlocale(LC_ALL, pLocal.c_str());
    if (ss == nullptr)
    {
        std::cout << "Failed to set local" << std::endl;
        return -1;
    }
    std::cout << "LC_ALL SET: " << ss << std::endl;
    std::cout << "====================================================================" << std::endl;
    std::cout << std::endl;

    return 0;
}

int main()
{
    int back = 0;
    std::cout << "AP Code starting" << std::endl;
    // back = LocalSettingsC("Turkish");

    // TestXml();
    // return 0;

    Config::ReadConfig();
    TestProcess();
    std::cin.get();
    system("pause");
    return back;
}