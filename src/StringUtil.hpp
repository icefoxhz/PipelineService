//
// Created by saber on 2022/5/5.
//

#ifndef HZUTILS_STRINGUTIL_HPP
#define HZUTILS_STRINGUTIL_HPP

#include <objbase.h>
#include "iostream"
#include <codecvt>
#include <string>
#include <regex>

using namespace std;

namespace hzStringUtil {
    static std::wstring To_Wide_String(const std::string& input)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(input);
    }

    static std::string To_Byte_String(const std::wstring& input)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.to_bytes(input);
    }

    static std::vector<std::wstring> WSplit(const std::wstring& in, const std::wstring& delim)
    {
         wregex re{ delim };
            return vector<wstring>{
                    wsregex_token_iterator(in.begin(), in.end(), re, -1),
                    wsregex_token_iterator()
            };
    }

    static std::vector<std::string> Split(const std::string& in, const std::string& delim)
    {
        regex re{ delim };
            return vector<string>{
                    sregex_token_iterator(in.begin(), in.end(), re, -1),
                    sregex_token_iterator()
            };
    }

    static std::string GuidToString(const GUID& guid)
    {
        char buf[64] = { 0 };
        sprintf_s(buf, sizeof(buf),
                  "%08lX%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
                  guid.Data1, guid.Data2, guid.Data3,
                  guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
                  guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
        return buf;
    }

    static bool endWith(const string& s, const string& endStr){
        size_t sLen = s.size();
        size_t endStrLen = endStr.size();
        if (sLen < endStrLen || sLen == 0 || endStrLen == 0)
        {
            return false;
        }

        return s.compare(s.size() - endStr.size(), endStr.size(), endStr) == 0;
    }

    static bool startWith(const string& s, const string& startStr){
        size_t sLen = s.size();
        size_t startStrLen = startStr.size();
        if (sLen < startStrLen || sLen == 0 || startStrLen == 0)
        {
            return false;
        }

        return s.compare(0, startStr.size(), startStr) == 0;
    }

    static std::string replace(const std::string& str, const std::string& from, const std::string& to) {
        std::string result = str;
        size_t start_pos = 0;
        while((start_pos = result.find(from, start_pos)) != std::string::npos) {
            result.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
        return result;
    }

    template<typename T>
    static string join(vector<T> src, char c){
        string res;
        if(src.empty()) return res;

        auto it = src.begin();
        T v = *it;
        res += to_string(v);

        for(it++;it!=src.end();it++){
            res += c;
            v = *it;
            res += to_string(v);
        }
        return res;
    }

    static string join(vector<string> src, char c){
        string res;
        if(src.empty()) return res;

        auto it = src.begin();
        res += *it;

        for(it++;it!=src.end();it++){
            res += c;
            res += *it;
        }
        return res;
    }

    template<typename T>
    static string join(set<T> src, char c){
        string res;
        if(src.empty()) return res;

        auto it = src.begin();
        T v = *it;
        res += to_string(v);

        for(it++;it!=src.end();it++){
            res += c;
            v = *it;
            res += to_string(v);
        }
        return res;
    }

    static string join(set<string> src, char c){
        string res;
        if(src.empty()) return res;

        auto it = src.begin();
        res += *it;

        for(it++;it!=src.end();it++){
            res += c;
            res += *it;
        }
        return res;
    }

} // hz

#endif //HZUTILS_STRINGUTIL_HPP
