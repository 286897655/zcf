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

#ifndef ZCF_MATH_HPP_
#define ZCF_MATH_HPP_

#include <zcf/zcf_utility.hpp>
#include <type_traits>

namespace zcf{
#if __cplusplus >= 201402L // C++14 and beyond
    using std::enable_if_t;
#else
    template<bool B, class T = void>
    using enable_if_t = typename std::enable_if<B, T>::type;
#endif

// check add overflow
template<typename T,typename = enable_if_t<std::is_unsigned<T>::value>>
bool check_add(T* result, T t1, T t2){
    Z_ASSERT(result != nullptr);
    if (ZLikely(t1 < std::numeric_limits<T>::max() - t2)) {
        *result = t1 + t2;
        return true;
    } else {
        *result = {};
        return false;
    }
};

// roundup a value aligned by align
template<typename T,typename = enable_if_t<std::is_unsigned<T>::value>>
T roundUp_align(T round,T align){
    auto maxAl = align - 1;
    T realSize;
    if (!check_add<T>(&realSize, round, maxAl)) {
        assert(0);//abort
    }
    return realSize & ~maxAl;
};

}//!namespace zcf


#endif //!ZCF_MATH_HPP_