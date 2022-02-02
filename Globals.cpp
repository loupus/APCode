
#include <algorithm>
#include <sstream>
#include <regex>
#include <iomanip>
#include "Globals.hpp"

// string manupilation yapacaksan widechar yapacan

std::wstring_convert<std::codecvt_utf8<wchar_t>> Globals::utf8conv;

std::wstring Globals::to_wstring(std::string const &s)
{
    return utf8conv.from_bytes(s);
}

std::string Globals::to_string(std::wstring const &s)
{
    return utf8conv.to_bytes(s);
}

std::string Globals::to_lower(std::string const &s)
{
    std::wstring ss = to_wstring(s);
    for (auto &c : ss)
    {
        if (c == L'I')
            c = L'ı'; // utf8 ama hangi dil
        if (c == L'İ')
            c = L'i';
        c = std::tolower(c, std::locale());
    }
    return to_string(ss);
}

std::size_t Globals::GetCharLen(std::string &pstr)
{
    size_t back = 0;
    std::wstring wstr = to_wstring(pstr);
    back = wstr.length();
    return back;
}

std::size_t  Globals::GetFirstIndexOf(std::string &pstr, std::string &psrch)
{
    size_t back = 0;
    std::wstring wstr = to_wstring(pstr);
    std::wstring wsrch = to_wstring(psrch);
    back = wstr.find_first_of(wsrch);
    return back;
}

std::size_t  Globals::GetLastIndexOf(std::string &pstr, std::string &psrch)
{
    size_t back = 0;
    std::wstring wstr = to_wstring(pstr);
    std::wstring wsrch = to_wstring(psrch);
     back = wstr.find_last_of(wsrch);
    return back;
}

std::string Globals::GetSubString(std::string &pstr, std::size_t  pbegin, std::size_t  plength)
{
    std::string back = "";
    std::wstring wstr = to_wstring(pstr);
    std::wstring temp = wstr.substr(pbegin, plength);
    back = to_string(temp);
    return back;
}

std::string Globals::utf8_to_string(const char *utf8str, const std::locale &loc)
{
    // UTF-8 to wstring
    std::wstring_convert<std::codecvt_utf8<wchar_t>> wconv;
    std::wstring wstr = wconv.from_bytes(utf8str);
    // wstring to string
    std::vector<char> buf(wstr.size());
    std::use_facet<std::ctype<wchar_t>>(loc).narrow(wstr.data(), wstr.data() + wstr.size(), '?', buf.data());
    return std::string(buf.data(), buf.size());
}

std::string Globals::charToHex(char c)
{
    std::string result;
    char first, second;

    first = (c & 0xF0) / 16;
    first += first > 9 ? 'A' - 10 : '0';
    second = c & 0x0F;
    second += second > 9 ? 'A' - 10 : '0';

    result.append(1, first);
    result.append(1, second);

    return result;
}

char Globals::hexToChar(char first, char second)
{
    int digit;

    digit = (first >= 'A' ? ((first & 0xDF) - 'A') + 10 : (first - '0'));
    digit *= 16;
    digit += (second >= 'A' ? ((second & 0xDF) - 'A') + 10 : (second - '0'));
    return static_cast<char>(digit);
}

std::string Globals::UrlEncode(const std::string &src)
{
    std::string result;
    std::string::const_iterator iter;

    for (iter = src.begin(); iter != src.end(); ++iter)
    {
        switch (*iter)
        {
        case ' ':
            result.append(1, '+');
            break;
            // alnum
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            // mark
        case '-':
        case '_':
        case '.':
        case '!':
        case '~':
        case '*':
        case '\'':
        case '(':
        case ')':
            result.append(1, *iter);
            break;
            // escape
        default:
            result.append(1, '%');
            result.append(charToHex(*iter));
            break;
        }
    }

    return result;
}

std::string Globals::UrlDecode(const std::string &src)
{
    std::string result;
    std::string::const_iterator iter;
    char c;

    for (iter = src.begin(); iter != src.end(); ++iter)
    {
        switch (*iter)
        {
        case '+':
            result.append(1, ' ');
            break;
        case '%':
            // Don't assume well-formed input
            if (std::distance(iter, src.end()) >= 2 && std::isxdigit(*(iter + 1)) && std::isxdigit(*(iter + 2)))
            {
                c = *++iter;
                result.append(1, hexToChar(c, *++iter));
            }
            // Just pass the % through untouched
            else
            {
                result.append(1, '%');
            }
            break;

        default:
            result.append(1, *iter);
            break;
        }
    }

    return result;
}

std::string Globals::replaceStringAll(std::string str, const std::string &replace, const std::string &with)
{
    if (!replace.empty())
    {
        std::size_t pos = 0;
        while ((pos = str.find(replace, pos)) != std::string::npos)
        {
            str.replace(pos, replace.length(), with);
            pos += with.length();
        }
    }
    return str;
}

std::wstring BackObject::GetWStrValue()
{
    std::wstring back = L"";
    back = Globals::to_wstring(StrValue);
    return back;
}

bool is_valid_utf8(const char *string)
{
    if (!string)
        return true;

    const unsigned char *bytes = (const unsigned char *)string;
    unsigned int cp;
    int num;

    while (*bytes != 0x00)
    {
        if ((*bytes & 0x80) == 0x00)
        {
            // U+0000 to U+007F
            cp = (*bytes & 0x7F);
            num = 1;
        }
        else if ((*bytes & 0xE0) == 0xC0)
        {
            // U+0080 to U+07FF
            cp = (*bytes & 0x1F);
            num = 2;
        }
        else if ((*bytes & 0xF0) == 0xE0)
        {
            // U+0800 to U+FFFF
            cp = (*bytes & 0x0F);
            num = 3;
        }
        else if ((*bytes & 0xF8) == 0xF0)
        {
            // U+10000 to U+10FFFF
            cp = (*bytes & 0x07);
            num = 4;
        }
        else
            return false;

        bytes += 1;
        for (int i = 1; i < num; ++i)
        {
            if ((*bytes & 0xC0) != 0x80)
                return false;
            cp = (cp << 6) | (*bytes & 0x3F);
            bytes += 1;
        }

        if ((cp > 0x10FFFF) ||
            ((cp >= 0xD800) && (cp <= 0xDFFF)) ||
            ((cp <= 0x007F) && (num != 1)) ||
            ((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
            ((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
            ((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4)))
            return false;
    }

    return true;
}

std::string Globals::tmToStr(tm *ptm)
{
    std::ostringstream dtss;
    dtss << std::put_time(ptm, "%Y-%m-%d %H:%M:%S");
    return dtss.str();
}

void Globals::strToTm(tm *ptm, std::string pstr)
{
    std::istringstream ss(pstr);
    ss >> std::get_time(ptm, "%Y-%m-%d %H:%M:%S");
}

void Globals::splitString(std::string pstr, std::string pdelimiter, std::vector<std::string> *strvec)
{
    size_t pos_start = 0, pos_end, delim_len = pdelimiter.length();
    std::string token;
    while ((pos_end = pstr.find(pdelimiter, pos_start)) != std::string::npos)
    {
        token = pstr.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        strvec->push_back(token);
    }
}

std::string Globals::joinString(std::string pdelimiter, std::vector<std::string> *strvec)
{
    std::string back;
    for (auto it = strvec->begin(); it != strvec->end(); ++it)
    {
        back.append((*it));
        back.append(pdelimiter);
    }
    return back;
}