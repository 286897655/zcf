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

#ifdef __x86_64__
#include <immintrin.h>
#elif __ARM_NEON
#include <arm_neon.h>
#endif

namespace zcf{
void cross_byte(const uint8_t* buffer,size_t size){
size_t count = size >> 4;
size_t mod = size & (16 - 1);
for(int i = 0; i < count; i++){
    uint8_t* bytes = (uint8_t*)buffer + 16 * i;
    #ifdef __SSSE3__
    const __m128i ShuffleRev = _mm_set_epi8(
			1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14
		);
    // Load 16 bytes at once into one 16-byte register
    __m128i load = _mm_loadu_si128(
        reinterpret_cast<__m128i*>(&bytes)
    );
    __m128i cross = _mm_shuffle_epi8(load,ShuffleRev);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(bytes),cross);
    #elif __ARM_NEON
    // TODO arm no shuffle
    #error "arm not support shuffle"
    #else
    #error "not support simd"
    #endif
}

uint8_t* bytes = (uint8_t*)buffer + 16 * count;
for(int i = 0;i < mod; i+=2){
    uint16_t read = Z_RBE16(bytes + i);
    Z_WLE16(bytes + i,read);
}
}

};//!namespace zcf