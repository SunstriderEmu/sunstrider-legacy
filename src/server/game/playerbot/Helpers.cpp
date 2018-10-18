
#include "playerbot.h"
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

vector<std::string>& split(const std::string &s, char delim, vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while(getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}


vector<std::string> split(const std::string &s, char delim)
{
    vector<std::string> elems;
    return split(s, delim, elems);
}

char *strstri(const char *haystack, const char *needle)
{
    if ( !*needle )
    {
        return (char*)haystack;
    }
    for ( ; *haystack; ++haystack )
    {
        if ( tolower(*haystack) == tolower(*needle) )
        {
            const char *h = haystack, *n = needle;
            for ( ; *h && *n; ++h, ++n )
            {
                if ( tolower(*h) != tolower(*n) )
                {
                    break;
                }
            }
            if ( !*n )
            {
                return (char*)haystack;
            }
        }
    }
    return 0;
}

std::string &ltrim(std::string &s) 
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
    return s;
}

std::string &rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}

std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}
