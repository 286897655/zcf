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
#include "zcf/zcf_buffer.hpp"
#include "zcf/zcf_utility.hpp"
#ifdef __x86_64__
#include <immintrin.h>
#elif __ARM_NEON
#include <arm_neon.h>
#endif

namespace zcf{

void cross_byte_u8_c(const uint8_t* buffer,size_t size)
{
    for(size_t i = 0; i < size; ){
        uint16_t read = Z_RBE16(buffer + i);
        Z_WLE16(buffer + i,read);

        // cross 2 bytes each time
        i+=2;
    }
};

#ifdef __SSSE3__
static void cross_byte_u8_x86_sse(const uint8_t* buffer,size_t size)
{
    constexpr static size_t CROSS_BYTE = 128 / 8;
    constexpr static size_t LOG2_CROSS_BYTE = 4;
    size_t count = size >> LOG2_CROSS_BYTE;
    size_t mod = size & (CROSS_BYTE - 1);
    for(size_t i = 0 ;i < count ; i++){
        uint8_t* pos = (uint8_t*)buffer + i * CROSS_BYTE;
        // x86 load in little endian
        const static __m128i ShuffleRev = _mm_setr_epi8
        (
            1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14
        );
        // Load 16 bytes at once into one 16-byte register
        __m128i load = _mm_loadu_si128(
            reinterpret_cast<__m128i*>(pos)
        );
        __m128i cross = _mm_shuffle_epi8(load,ShuffleRev);
        _mm_storeu_si128(reinterpret_cast<__m128i*>(pos),cross);
    }
    // un 16bytes aligned use c code
    cross_byte_u8_c(buffer + CROSS_BYTE * count,mod);
};
#endif

#ifdef __ARM_NEON
#error "arm not support shuffle,do it in other"
static void cross_byte_u8_arm_neon(const uint8_t* buffer,size_t size){


};
#endif

void cross_byte_u8(const uint8_t* buffer,size_t size)
{
    Z_ASSERT(!(size & 0x01));
    #ifdef __SSSE3__
    cross_byte_u8_x86_sse(buffer,size);
    #elif __ARM_NEON
    cross_byte_u8_arm_neon(buffer,size);
    #else
    cross_byte_u8_c(buffer,size);
    #endif
}

void cross_byte_s16_c(const int16_t* bytes,size_t size)
{
    cross_byte_u8_c((const uint8_t*)bytes,size * sizeof(int16_t));
};
void cross_byte_s16(const int16_t* bytes,size_t size)
{
    #ifdef __SSSE3__
    cross_byte_u8_x86_sse((const uint8_t*)bytes,size * sizeof(int16_t));
    #elif __ARM_NEON
    cross_byte_u8_arm_neon((const uint8_t*)bytes,size * sizeof(int16_t));
    #else
    cross_byte_u8_c((const uint8_t*)bytes,size * sizeof(int16_t));
    #endif
}

};//!namespace zcf