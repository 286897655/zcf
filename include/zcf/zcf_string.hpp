#ifndef ZCF_STRING_HPP
#define ZCF_STRING_HPP

#include <string>
#include <memory>
#include <zcf/zcf_utility.hpp>

namespace zcf{
    static constexpr size_t kDefaultPage = 4096;//4K

    template<typename ... Args>
    std::string str_printf_format(const std::string& fmt, Args ... args) {
        // snprintf will write \0 at end.the return no counting \0
        int size_buf = ::snprintf(nullptr,0,fmt.c_str(),args ...) + 1;
        if(ZLikely(size_buf <= kDefaultPage)){
            char buffer[kDefaultPage];
            size_buf = ::snprintf(buffer,sizeof(buffer),fmt.c_str(),args ...);
            return std::string(buffer,size_buf);
        }else{
            std::unique_ptr<char> buffer(new(std::nothrow) char[size_buf]);
            size_buf = ::snprintf(buffer.get(),size_buf,fmt.c_str(),args ...);
            return std::string(buffer.get(),size_buf);
        }
    };

    /**
    * std::string replace all
    * part replace use std::string::replace
    */
    inline std::string str_replace(const std::string& origin,const std::string& old_pattern,const std::string& new_pattern){
        if(!old_pattern.compare(new_pattern))
            return origin;
        
        size_t pos = 0;
        std::string ret_val = origin;
        while((pos = ret_val.find(old_pattern,pos)) != std::string::npos){
            ret_val.replace(pos,old_pattern.length(),new_pattern);
        }

        return ret_val;
    };

    inline std::vector<std::string> str_split(const std::string& origin,const char* delim){
        std::vector<std::string> ret;
        size_t last = 0;
        auto index = origin.find(delim, last);
        while (index != std::string::npos) {
            if (index - last > 0) {
                ret.push_back(origin.substr(last, index - last));
            }
            last = index + ::strlen(delim);
            index = origin.find(delim, last);
        }
        if (!origin.size() || origin.size() - last > 0) {
            ret.push_back(origin.substr(last));
        }
        return ret;
    };

    inline std::string str_ltrim(std::string& s) {
        s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), [](char ch) {
            return std::isspace(ch);
            }));
        return s;
    };

    inline std::string str_rtrim(std::string& s) {
        s.erase(std::find_if_not(s.rbegin(), s.rend(), [](char c) { return std::isspace(c); }).base(), s.end());
        return s;
    };

    inline std::string str_trim(std::string& s) {
        s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), [](char c) { return std::isspace(c); }));
        s.erase(std::find_if_not(s.rbegin(), s.rend(), [](char c) { return std::isspace(c); }).base(), s.end());

        return s;
    };

    inline void str_toUpper(std::string* const str) {
        std::transform(str->begin(), str->end(), str->begin(), [](char c) { return std::toupper(c); });
    };

    inline void str_toLower(std::string* const str) {
        std::transform(str->begin(), str->end(), str->begin(), [](char c) { return std::tolower(c); });
    };

    inline std::string str_toUpper(const std::string& str) {
        std::string result("", str.size());
        std::transform(str.begin(), str.end(), result.begin(), [](char c) { return std::toupper(c); });
        return result;
    };

    inline std::string str_toLower(const std::string& str) {
        std::string result("", str.size());
        std::transform(str.begin(), str.end(), result.begin(), [](char c) { return std::tolower(c); });
        return result;
    };

    // ----------------------------------------------------------------------
    // str_hasprefix()
    //    Check if a string begins with a given prefix.
    // str_stripprefix()
    //    Given a string and a putative prefix, returns the string minus the
    //    prefix string if the prefix matches, otherwise the original
    //    string.
    // ----------------------------------------------------------------------
    inline bool str_hasprefix(const std::string& str, const std::string& prefix)
    {
        return str.size() >= prefix.size() &&
            str.compare(0, prefix.size(), prefix) == 0;
    }

    inline std::string str_stripprefix(const std::string& str, const std::string& prefix)
    {
        if (str_hasprefix(str, prefix))
            return str.substr(prefix.size());
        else
            return str;
    }

    // ----------------------------------------------------------------------
    // str_hassuffix()
    //    Return true if str ends in suffix.
    // str_stripsuffix()
    //    Given a string and a putative suffix, returns the string minus the
    //    suffix string if the suffix matches, otherwise the original
    //    string.
    // ----------------------------------------------------------------------
    inline bool str_hassuffix(const std::string& str, const std::string& suffix)
    {
        return str.size() >= suffix.size() &&
            str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    inline std::string str_stripsuffix(const std::string& str, const std::string& suffix)
    {
        if (str_hassuffix(str, suffix))
            return str.substr(0, str.size() - suffix.size());
        else
            return str;
    }
}//!namesapce zcf

#endif //!ZCF_STRING_HPP