#include <iostream>
#include "Config.hpp"
#include "APapi.hpp"

void TestAccountInfo()
{
    BackObject back;
    apapi ap;
    if(ap.Initiliaze())
    {
    back = ap.GetAccountInfo();
    if(!back.Success)
       std::cout << back.ErrDesc << std::endl;
    }

}

void TestSearch()
{
    BackObject back;
    apapi ap;
    if(!ap.Initiliaze())
    {
        std::cout << "ap api initiliaze failed!" << std::endl;
        return;
    }
    back = ap.Search();
    if(!back.Success)
       std::cout << back.ErrDesc << std::endl;
   // else
   //    std::cout << back.StrValue << std::endl;
}

int main()
{
    std::cout << "AP Code starting" << std::endl;
    Config::ReadConfig();
    TestSearch();
 
    
    return 0;
}