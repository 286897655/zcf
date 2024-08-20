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

#ifndef ZCF_BUFFER_HPP_
#define ZCF_BUFFER_HPP_

/*
* R/W means read/write, B/L means big/little/native endianness.
*/

#ifndef Z_RBE8
#define Z_RBE8(x) (((const uint8_t*)(x))[0])
#endif
#ifndef Z_RLE8
#define Z_RLE8(x) Z_RBE8(x)
#endif
#ifndef Z_RBE16
#   define Z_RBE16(x)                            \
    (((uint16_t)((const uint8_t*)(x))[0] << 8) |   \
                ((const uint8_t*)(x))[1])
#endif
#ifndef Z_WBE16
#   define Z_WBE16(p, val) do {               \
        uint16_t d = (val);                     \
        ((uint8_t*)(p))[1] = (d);               \
        ((uint8_t*)(p))[0] = (d)>>8;            \
    } while(0)
#endif
#ifndef Z_RLE16
#   define Z_RLE16(x)                           \
    (((uint16_t)((const uint8_t*)(x))[1] << 8) |  \
                ((const uint8_t*)(x))[0])
#endif
#ifndef Z_WLE16
#   define Z_WLE16(p, val) do {               \
        uint16_t d = (val);                     \
        ((uint8_t*)(p))[0] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
    } while(0)
#endif

#ifndef Z_RBE24
#   define Z_RBE24(x)                             \
    (((uint32_t)((const uint8_t*)(x))[0] << 16) |   \
               (((const uint8_t*)(x))[1] <<  8) |   \
                ((const uint8_t*)(x))[2])
#endif

#ifndef Z_WBE24
#   define Z_WBE24(p,val) do{                 \
        uint32_t d = (val);                     \
        ((uint8_t*)(p))[2] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
        ((uint8_t*)(p))[0] = (d)>>16;           \
    }while(0)
#endif

#ifndef Z_RLE24
#   define Z_RLE24(x)                             \
    (((uint32_t)((const uint8_t*)(x))[2] << 16) |   \
               (((const uint8_t*)(x))[1] <<  8) |   \
                ((const uint8_t*)(x))[0])
#endif

#ifndef Z_WLE24
#   define Z_WLE24(p, val) do {               \
        uint32_t d = (val);                     \
        ((uint8_t*)(p))[0] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
        ((uint8_t*)(p))[2] = (d)>>16;           \
    } while(0)
#endif

#ifndef Z_RBE32
#   define Z_RBE32(x)                             \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) |   \
               (((const uint8_t*)(x))[1] << 16) |   \
               (((const uint8_t*)(x))[2] <<  8) |   \
                ((const uint8_t*)(x))[3])
#endif
#ifndef Z_WBE32
#   define Z_WBE32(p, val) do {               \
        uint32_t d = (val);                     \
        ((uint8_t*)(p))[3] = (d);               \
        ((uint8_t*)(p))[2] = (d)>>8;            \
        ((uint8_t*)(p))[1] = (d)>>16;           \
        ((uint8_t*)(p))[0] = (d)>>24;           \
    } while(0)
#endif
#ifndef Z_RLE32
#   define Z_RLE32(x)                              \
    (((uint32_t)((const uint8_t*)(x))[3] << 24) |    \
               (((const uint8_t*)(x))[2] << 16) |    \
               (((const uint8_t*)(x))[1] <<  8) |    \
                ((const uint8_t*)(x))[0])
#endif
#ifndef Z_WLE32
#   define Z_WLE32(p, val) do {               \
        uint32_t d = (val);                     \
        ((uint8_t*)(p))[0] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
        ((uint8_t*)(p))[2] = (d)>>16;           \
        ((uint8_t*)(p))[3] = (d)>>24;           \
    } while(0)
#endif
#ifndef Z_RBE64
#   define Z_RBE64(x)                                 \
    (((uint64_t)((const uint8_t*)(x))[0] << 56) |       \
     ((uint64_t)((const uint8_t*)(x))[1] << 48) |       \
     ((uint64_t)((const uint8_t*)(x))[2] << 40) |       \
     ((uint64_t)((const uint8_t*)(x))[3] << 32) |       \
     ((uint64_t)((const uint8_t*)(x))[4] << 24) |       \
     ((uint64_t)((const uint8_t*)(x))[5] << 16) |       \
     ((uint64_t)((const uint8_t*)(x))[6] <<  8) |       \
      (uint64_t)((const uint8_t*)(x))[7])
#endif
#ifndef Z_WBE64
#   define Z_WBE64(p, val) do {               \
        uint64_t d = (val);                     \
        ((uint8_t*)(p))[7] = (d);               \
        ((uint8_t*)(p))[6] = (d)>>8;            \
        ((uint8_t*)(p))[5] = (d)>>16;           \
        ((uint8_t*)(p))[4] = (d)>>24;           \
        ((uint8_t*)(p))[3] = (d)>>32;           \
        ((uint8_t*)(p))[2] = (d)>>40;           \
        ((uint8_t*)(p))[1] = (d)>>48;           \
        ((uint8_t*)(p))[0] = (d)>>56;           \
    } while(0)
#endif
#ifndef Z_RLE64
#   define Z_RLE64(x)                                 \
    (((uint64_t)((const uint8_t*)(x))[7] << 56) |       \
     ((uint64_t)((const uint8_t*)(x))[6] << 48) |       \
     ((uint64_t)((const uint8_t*)(x))[5] << 40) |       \
     ((uint64_t)((const uint8_t*)(x))[4] << 32) |       \
     ((uint64_t)((const uint8_t*)(x))[3] << 24) |       \
     ((uint64_t)((const uint8_t*)(x))[2] << 16) |       \
     ((uint64_t)((const uint8_t*)(x))[1] <<  8) |       \
      (uint64_t)((const uint8_t*)(x))[0])
#endif
#ifndef Z_WLE64
#   define Z_WLE64(p, val) do {               \
        uint64_t d = (val);                     \
        ((uint8_t*)(p))[0] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
        ((uint8_t*)(p))[2] = (d)>>16;           \
        ((uint8_t*)(p))[3] = (d)>>24;           \
        ((uint8_t*)(p))[4] = (d)>>32;           \
        ((uint8_t*)(p))[5] = (d)>>40;           \
        ((uint8_t*)(p))[6] = (d)>>48;           \
        ((uint8_t*)(p))[7] = (d)>>56;           \
    } while(0)
#endif

namespace zcf{

class bit_buffer{

};

}//!namespace zcf


#endif //!ZCF_BUFFER_HPP_