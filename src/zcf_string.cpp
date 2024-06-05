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
 * @brief 
 */

#include "zcf/zcf_string.hpp"
#include <iomanip>

namespace zcf{
std::string hexDump(const uint8_t* data,size_t size,size_t max_size){
    Z_ASSERT(data);
    size = std::min(size,max_size);

    constexpr static size_t PRINT_NUM = 16;

    std::ostringstream oss;
    {
        // title
        oss << "OFFSET_H" << " ";
        for(size_t i = 0;i < PRINT_NUM;i++){
            oss << std::hex << std::setw(2) << std::setiosflags(std::ios::uppercase) << i << " ";
        }
        oss << "|" << " " << "ASCII" << std::endl;
    }

    {
        // data
        for(size_t i = 0;i < size;i += PRINT_NUM){
            // head of pos
            oss << std::hex << std::setfill('0') << std::setw(8) << std::setiosflags(std::ios::uppercase) << i << " ";
            // print data
            {
                for(size_t j = 0;j< PRINT_NUM;j++){
                    if(i + j < size){
                        oss << std::hex << std::setfill('0') << std::setw(2) << std::setiosflags(std::ios::uppercase) << static_cast<int>(data[i + j]);
                    }else{
                        oss << "  ";
                    }
                    oss << " ";
                }
            }                
            // print ascii
            oss << "|" << " ";
            for(size_t j = 0;j < PRINT_NUM;j++){
                if(i + j < size){
                    if(std::isprint(data[i+j])){
                        oss << static_cast<char>(data[i+j]);
                    }else{
                        oss << ".";
                    }
                }
            }
            oss << std::endl;
        }
    }

    return oss.str();
}
};//!namespace zcf