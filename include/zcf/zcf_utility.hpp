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

#define ZCF_DISABLE_COPY(Class) \
    Class(const Class &) = delete;\
    Class &operator=(const Class &) = delete;

#define ZCF_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;

#define ZCF_DISABLE_COPY_MOVE(Class) \
    ZCF_DISABLE_COPY(Class) \
    ZCF_DISABLE_MOVE(Class)


#define Z_UNUSED(x) (void)x;

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

#ifndef Z_INT_SUCCESS
#define Z_INT_SUCCESS 0
#endif

#ifndef Z_INT_FAIL
#define Z_INT_FAIL -1
#endif

#ifndef ZLikely
    #ifdef _MSC_VER
    #define ZLikely(cond) (cond)
    #elif defined(__GNUC__)
    #define ZLikely(cond) __builtin_expect(!!(cond), 1) 
    #endif
#endif

#ifndef ZUnlikely
    #ifdef _MSC_VER
    #define ZUnlikely(cond) (cond)
    #elif defined(__GNUC__)
    #define ZUnlikely(cond) __builtin_expect(!!(cond), 0) 
    #endif
#endif

#ifndef ZDELETE_P
#define ZDELETE_P(p) \
    if (p) { \
        delete p; \
        p = NULL; \
    } \
    (void)0
#endif

#ifndef ZDELETE_PA
#define ZDELETE_PA(pa) \
    if (pa) { \
        delete[] pa; \
        pa = NULL; \
    } \
    (void)0
#endif

namespace zcf{
    template<typename T>
    struct using_shared_ptr{
        using shared = std::shared_ptr<T>;
        using weak = std::weak_ptr<T>;
    };

    template<typename T>
    struct using_unique_ptr{
        using unique = std::unique_ptr<T>;
    };

    // make_unique support for pre c++14
#if __cplusplus >= 201402L  // C++14 and beyond
using std::enable_if_t;
using std::make_unique;
#else
template <bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&...args) {
    static_assert(!std::is_array<T>::value, "arrays not supported");
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif
};//!namespace zcf

// some constant declaration
namespace zcf{
    extern const char kZCFVersion[];
}//!namespace zcf

#endif //!ZCF_UTILITY_HPP_