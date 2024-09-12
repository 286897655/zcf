/** 
 * @copyright Copyright © 2020-2024 code by zhaoj
 * 
 * LICENSE
 * 
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

 /**
 * @author zhaoj 286897655@qq.com
 * @brief strings utility
 * 
 * using like strings::xxxxxx
 */

#ifndef ZCF_STRINGS_HPP_
#define ZCF_STRINGS_HPP_

#include <string>
#include <algorithm>
#include <sstream>
#include <memory>

namespace zcf{

namespace strings{
/// @brief left trim string
/// @param s 
/// @return 
inline std::string ltrim(std::string& s) 
{
    s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), [](char ch) { return std::isspace(ch); }));
    return s;
};

/// @brief right trim string
/// @param s 
/// @return 
inline std::string rtrim(std::string& s) 
{
    s.erase(std::find_if_not(s.rbegin(), s.rend(), [](char c) { return std::isspace(c); }).base(), s.end());
    return s;
};

/// @brief trim string head and tail
/// @param s 
/// @return 
inline std::string trim(std::string& s) 
{
    s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), [](char c) { return std::isspace(c); }));
    s.erase(std::find_if_not(s.rbegin(), s.rend(), [](char c) { return std::isspace(c); }).base(), s.end());

    return s;
};

/// @brief std::string replace all / part replace use std::string::replace
/// @param origin 
/// @param old_pattern 
/// @param new_pattern 
/// @return 
std::string replace_all(const std::string& origin,const std::string& old_pattern,const std::string& new_pattern);

/// @brief split string to vector string with delim
/// @param origin 
/// @param delim 
/// @return 
std::vector<std::string> split(const std::string& origin,const char* delim);

/// @brief to upper case abcd->ABCD
/// @param str 
inline void toupper(std::string* const str) 
{
    std::transform(str->begin(), str->end(), str->begin(), [](char c) { return std::toupper(c); });
};

/// @brief to lower case ABCD->abcd
/// @param str 
inline void tolower(std::string* const str) 
{
    std::transform(str->begin(), str->end(), str->begin(), [](char c) { return std::tolower(c); });
};

/// @brief to upper case abcd->ABCD with return
/// @param str 
/// @return 
inline std::string toupper(const std::string& str) 
{
    std::string result("", str.size());
    std::transform(str.begin(), str.end(), result.begin(), [](char c) { return std::toupper(c); });
    return result;
};

/// @brief to lower case ABCD->abcd with return
/// @param str 
/// @return 
inline std::string tolower(const std::string& str) 
{
    std::string result("", str.size());
    std::transform(str.begin(), str.end(), result.begin(), [](char c) { return std::tolower(c); });
    return result;
};

// ----------------------------------------------------------------------
// has_prefix()
//    Check if a string begins with a given prefix.
// strip_prefix()
//    Given a string and a putative prefix, returns the string minus the
//    prefix string if the prefix matches, otherwise the original
//    string.
// ----------------------------------------------------------------------
inline bool has_prefix(const std::string& str, const std::string& prefix)
{
    return str.size() >= prefix.size() &&
        str.compare(0, prefix.size(), prefix) == 0;
}

inline std::string strip_prefix(const std::string& str, const std::string& prefix)
{
    if (has_prefix(str, prefix))
        return str.substr(prefix.size());
    else
        return str;
}

// ----------------------------------------------------------------------
//  has_suffix()
//    Return true if str ends in suffix.
//  strip_suffix()
//    Given a string and a putative suffix, returns the string minus the
//    suffix string if the suffix matches, otherwise the original
//    string.
// ----------------------------------------------------------------------
inline bool has_suffix(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() &&
        str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

inline std::string strip_suffix(const std::string& str, const std::string& suffix)
{
    if (has_suffix(str, suffix))
        return str.substr(0, str.size() - suffix.size());
    else
        return str;
}

/// @brief a simple from,but not effcient
/// @tparam T 
/// @param t 
/// @return 
template<typename T>
std::string conv_from(T t){
    std::ostringstream oss;
    oss << t;
    return oss.str();
};

/// @brief stoi will throw exception,so if str is sure number value 
/// use stoi，then use this to avoid exception
/// @tparam T 
/// @param str 
/// @return 
template<typename T>
T conv_to(const std::string& str){
    T t;
    std::istringstream iss(str);
    iss >> t;

    return t;
};


static constexpr size_t kDefaultStackBuffer = 1024;//1024 in stack,other will be in heap
template<typename ... Args>
std::string format(const std::string& fmt, Args ... args) {
    // snprintf will write \0 at end.the return no counting \0
    int size_buf = ::snprintf(nullptr,0,fmt.c_str(),args ...) + 1;
    if(size_buf <= kDefaultStackBuffer){
        char buffer[kDefaultStackBuffer];
        size_buf = ::snprintf(buffer,sizeof(buffer),fmt.c_str(),args ...);
        return std::string(buffer,size_buf);
    }else{
        std::unique_ptr<char> buffer(new(std::nothrow) char[size_buf]);
        size_buf = ::snprintf(buffer.get(),size_buf,fmt.c_str(),args ...);
        return std::string(buffer.get(),size_buf);
    }
};

}//!namespace strings

}//!namespace zcf
#endif //!ZCF_STRINGS_HPP_
