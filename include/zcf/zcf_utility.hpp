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

#ifndef ZCF_UTILITY_HPP_
#define ZCF_UTILITY_HPP_

// 
#include <memory>
#include <zcf/zcf_config.hpp>

// forward declare macro for obvirousely forward declaration
#define Z_FWD_DECLARE_CLASS(name) class name;
#define Z_FWD_DECLARE_STRUCT(name) struct name;

#ifndef Z_STR
// to string
#define STR_1(x) #x
// contact string
#define JOIN_1(x,y) x##y


#define Z_STR(x) STR_1(x)
#define Z_BUILD_STR(x,y) JOIN_1(x,y)
#endif

// some constant declaration
namespace zcf{
    extern const char kZCFVersion[];
}//!namespace zcf

#endif //!ZCF_UTILITY_HPP_