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

#include "zcf/strings.hpp"
#include <string.h>

namespace zcf{
namespace strings{

std::vector<std::string> split(const std::string& origin,const char* delim)
{
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
}

std::string replace_all(const std::string& origin,const std::string& old_pattern,const std::string& new_pattern)
{
    if(!old_pattern.compare(new_pattern))
        return origin;
    
    size_t pos = 0;
    std::string ret_val = origin;
    while((pos = ret_val.find(old_pattern,pos)) != std::string::npos){
        ret_val.replace(pos,old_pattern.length(),new_pattern);
    }

    return ret_val;
}

}//!namespace strings
}//!namespace zcf