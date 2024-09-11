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

#ifndef ZCF_CONFIG_HPP_
#define ZCF_CONFIG_HPP_

#if defined(NDEBUG) && defined(DEBUG)
#error "NDEBUG and DEBUG can't defined at same time"
#endif


#if (defined(linux) || defined(__linux) || defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)) && !defined(_CRAYC)
#define ZCF_SYS_LINUX 1
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define ZCF_SYS_WINDOWS 1
#elif defined (__MACH__) && defined (__APPLE__)
#define ZCF_SYS_MACH 1
#endif

#if defined(ZCF_SYS_LINUX) && defined(ZCF_SYS_WINDOWS)
#error "ZCF_SYS_LINUX and ZCF_SYS_WINDOWS can't defined at same time"
#endif

//ASSERT
#ifdef NDEBUG // release
    #define Z_ASSERT(x) ((void)0)
#else
    #include <assert.h>
    #define Z_ASSERT(x) assert(x)
#endif

#endif //!ZCF_CONFIG_HPP_