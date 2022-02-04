#pragma once
#include <iostream>
#include <map>

std::map<std::string, std::string> map1 =
    {
        // https://ascii.cl/htmlcodes.htm

        {"&#32;", " "}, // space
        {"&#33;", "!"}, // exclamation point
        {"&#34;", """"},  // double quotes
        {"&#35;", "#"}, // number sign
        {"&#36;", "$"}, // dollar sign
        {"&#37;", "%"}, // percent sign
        {"&#38;", "&"},
        {"&#39;", "'"},  
        {"&#42;", "*"},
        {"&#43;", "+"},     
        {"&#45;", "-"},      
        {"&#47;", "/"},
        {"&#60;", "<"},
        {"&#61;", "="},
        {"&#62;", ">"},  
        {"&#64;", "@"},
        {"&#94;", "^"},
        {"&#95;", "_"},
        {"&#96;", "`"},
        {"&#126;", "~"},

        {"&#163;", "£"},            //pound sign
        {"&#169;", "©"},            //copyright sign
        {"&#174;", "®"},            //registered trade mark sign


        {"&#199;", "Ç"},
        {"&#214;", "Ö"},
        {"&#220;", "Ü"},
        {"&#231;", "ç"},
        {"&#246;", "ö"},
        {"&#252;", "ü"},

        {"&#8364;", "€"}

};

std::map<std::string, std::string> map2 =
    {
        // https://ascii.cl/htmlcodes.htm

        {"&nbsp;", " "}, 
        {"&amp;", "&"}, 
        {"&quot;", """"},  
        {"&lt;", "<"}, 
        {"&gt;", ">"}, 
        {"&copy;", "©"}, 
        {"&euro;", "€"},
        {"&pound;", "£"}        

};