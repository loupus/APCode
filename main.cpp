#include <iostream>
#include "Config.hpp"
#include "APapi.hpp"

void TestAccountInfo()
{
    BackObject back;
    apapi ap;
    back = ap.GetAccountInfo();
    if(!back.Success)
       std::cout << back.ErrDesc << std::endl;
}

int main()
{
    std::cout << "AP Code starting" << std::endl;
    Config::ReadConfig();
    
        time_t now_t = 0;
        time_t gnow_t = 0;
        time(&now_t);
        struct tm* gmtm;
        gmtm = gmtime(&now_t);
        gnow_t = mktime(gmtm);

        std::cout << "now_t: " << now_t << std::endl;
        std::cout << "gnow_t: " << gnow_t << std::endl;
 
    
    return 0;
}