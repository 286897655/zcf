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

#include "zav/codec/h26x.h"
#include <string.h>

#include "zcf/memory.hpp"
#include <immintrin.h>
namespace zav{

namespace h26x{

static const uint8_t h26x_start_prefix[3] = {0x00,0x00,0x01};

/**
 *  @brief  Helper structure to simplify work with 256-bit registers.
 */
typedef union sz_u256_vec_t {
    __m256i ymm;
    __m128i xmms[2];
    uint64_t u64s[4];
    uint32_t u32s[8];
    uint16_t u16s[16];
    uint8_t u8s[32];
} sz_u256_vec_t;

/**
 *  @brief  Helper structure to simplify work with 64-bit words.
 *  @see    sz_u64_load
 */
typedef union sz_u64_vec_t {
    uint64_t u64;
    uint32_t u32s[2];
    uint16_t u16s[4];
    uint8_t u8s[8];
} sz_u64_vec_t;

/**
 *  @brief  Byte-level equality comparison between two 64-bit integers.
 *  @return 64-bit integer, where every top bit in each byte signifies a match.
 */
sz_u64_vec_t _sz_u64_each_byte_equal(sz_u64_vec_t a, sz_u64_vec_t b) {
    sz_u64_vec_t vec;
    vec.u64 = ~(a.u64 ^ b.u64);
    // The match is valid, if every bit within each byte is set.
    // For that take the bottom 7 bits of each byte, add one to them,
    // and if this sets the top bit to one, then all the 7 bits are ones as well.
    vec.u64 = ((vec.u64 & 0x7F7F7F7F7F7F7F7Full) + 0x0101010101010101ull) & ((vec.u64 & 0x8080808080808080ull));
    return vec;
}

/**
 *  @brief  3Byte-level equality comparison between two 64-bit integers.
 *  @return 64-bit integer, where every top bit in each 3byte signifies a match.
 */
sz_u64_vec_t _sz_u64_each_3byte_equal(sz_u64_vec_t a, sz_u64_vec_t b) {
    sz_u64_vec_t vec;
    vec.u64 = ~(a.u64 ^ b.u64);
    // The match is valid, if every bit within each 4byte is set.
    // For that take the bottom 31 bits of each 4byte, add one to them,
    // and if this sets the top bit to one, then all the 31 bits are ones as well.
    vec.u64 = ((vec.u64 & 0xFFFF7FFFFF7FFFFFull) + 0x0000000001000001ull) & ((vec.u64 & 0x0000800000800000ull));
    return vec;
}

uint8_t* sz_find_byte_serial(uint8_t* h, size_t h_length, uint8_t* n) {

    if (!h_length) return nullptr;
    uint8_t* const h_end = h + h_length;

    // Broadcast the n into every byte of a 64-bit integer to use SWAR
    // techniques and process eight characters at a time.
    sz_u64_vec_t h_vec, n_vec, match_vec;
    match_vec.u64 = 0;
    n_vec.u64 = (uint64_t)n[0] * 0x0101010101010101ull;
    for (; h + 8 <= h_end; h += 8) {
        h_vec.u64 = *(uint64_t const *)h;
        match_vec = _sz_u64_each_byte_equal(h_vec, n_vec);
        if (match_vec.u64) return h + __builtin_ctzll(match_vec.u64) / 8;
    }

    // Handle the misaligned tail.
    for (; h < h_end; ++h)
        if (*h == *n) return h;
    return nullptr;
}

uint8_t* sz_find_byte_avx2(uint8_t* bytes, size_t sizeBytes) {
    int mask;
    sz_u256_vec_t h_vec, n_vec;
    n_vec.ymm = _mm256_set1_epi8(h26x_start_prefix[0]);
    
    uint8_t* h = bytes;
    size_t h_length = sizeBytes;
    while (h_length >= 32) {
        h_vec.ymm = _mm256_lddqu_si256((__m256i const *)h);
        mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(h_vec.ymm, n_vec.ymm));
        if (mask) return bytes + __builtin_ctz(mask);
        h += 32, h_length -= 32;
    }

    return sz_find_byte_serial(h, h_length, (uint8_t*)h26x_start_prefix);
}

/**
 *  @brief  Chooses the offsets of the most interesting characters in a search needle.
 *
 *  Search throughput can significantly deteriorate if we are matching the wrong characters.
 *  Say the needle is "aXaYa", and we are comparing the first, second, and last character.
 *  If we use SIMD and compare many offsets at a time, comparing against "a" in every register is a waste.
 *
 *  Similarly, dealing with UTF8 inputs, we know that the lower bits of each character code carry more information.
 *  Cyrillic alphabet, for example, falls into [0x0410, 0x042F] code range for uppercase [А, Я], and
 *  into [0x0430, 0x044F] for lowercase [а, я]. Scanning through a text written in Russian, half of the
 *  bytes will carry absolutely no value and will be equal to 0x04.
 */
void _sz_locate_needle_anomalies(uint8_t* start, size_t length, //
                                             size_t *first, size_t *second, size_t *third) {
    *first = 0;
    *second = length / 2;
    *third = length - 1;

    //
    int has_duplicates =                   //
        start[*first] == start[*second] || //
        start[*first] == start[*third] ||  //
        start[*second] == start[*third];

    // Loop through letters to find non-colliding variants.
    if (length > 3 && has_duplicates) {
        // Pivot the middle point right, until we find a character different from the first one.
        for (; start[*second] == start[*first] && *second + 1 < *third; ++(*second)) {}
        // Pivot the third (last) point left, until we find a different character.
        for (; (start[*third] == start[*second] || start[*third] == start[*first]) && *third > (*second + 1);
             --(*third)) {}
    }

    // TODO: Investigate alternative strategies for long needles.
    // On very long needles we have the luxury to choose!
    // Often dealing with UTF8, we will likely benfit from shifting the first and second characters
    // further to the right, to achieve not only uniqness within the needle, but also avoid common
    // rune prefixes of 2-, 3-, and 4-byte codes.
    if (length > 8) {
        // Pivot the first and second points right, until we find a character, that:
        // > is different from others.
        // > doesn't start with 0b'110x'xxxx - only 5 bits of relevant info.
        // > doesn't start with 0b'1110'xxxx - only 4 bits of relevant info.
        // > doesn't start with 0b'1111'0xxx - only 3 bits of relevant info.
        //
        // So we are practically searching for byte values that start with 0b0xxx'xxxx or 0b'10xx'xxxx.
        // Meaning they fall in the range [0, 127] and [128, 191], in other words any unsigned int up to 191.
        uint8_t const *start_u8 = (uint8_t const *)start;
        size_t vibrant_first = *first, vibrant_second = *second, vibrant_third = *third;

        // Let's begin with the seccond character, as the termination criterea there is more obvious
        // and we may end up with more variants to check for the first candidate.
        for (; (start_u8[vibrant_second] > 191 || start_u8[vibrant_second] == start_u8[vibrant_third]) &&
               (vibrant_second + 1 < vibrant_third);
             ++vibrant_second) {}

        // Now check if we've indeed found a good candidate or should revert the `vibrant_second` to `second`.
        if (start_u8[vibrant_second] < 191) { *second = vibrant_second; }
        else { vibrant_second = *second; }

        // Now check the first character.
        for (; (start_u8[vibrant_first] > 191 || start_u8[vibrant_first] == start_u8[vibrant_second] ||
                start_u8[vibrant_first] == start_u8[vibrant_third]) &&
               (vibrant_first + 1 < vibrant_second);
             ++vibrant_first) {}

        // Now check if we've indeed found a good candidate or should revert the `vibrant_first` to `first`.
        // We don't need to shift the third one when dealing with texts as the last byte of the text is
        // also the last byte of a rune and contains the most information.
        if (start_u8[vibrant_first] < 191) { *first = vibrant_first; }
    }
}



uint8_t* _sz_find_3byte_serial(uint8_t* h, size_t h_length, uint8_t* n) {

    // This is an internal method, and the haystack is guaranteed to be at least 4 bytes long.
    //sz_assert(h_length >= 3 && "The haystack is too short.");
    uint8_t* const h_end = h + h_length;

    // We fetch 12
    sz_u64_vec_t h0_vec, h1_vec, h2_vec, h3_vec, h4_vec;
    sz_u64_vec_t matches0_vec, matches1_vec, matches2_vec, matches3_vec, matches4_vec;
    sz_u64_vec_t n_vec;
    n_vec.u64 = 0;
    n_vec.u8s[0] = n[0], n_vec.u8s[1] = n[1], n_vec.u8s[2] = n[2], n_vec.u8s[3] = n[3];
    n_vec.u64 *= 0x0000000001000001ull; // broadcast

    // This code simulates hyper-scalar execution, analyzing 8 offsets at a time using three 64-bit words.
    // We load the subsequent two-byte word as well.
    uint64_t h_page_current, h_page_next;
    for (; h + sizeof(uint64_t) + sizeof(uint16_t) <= h_end; h += sizeof(uint64_t)) {
        h_page_current = *(uint64_t *)h;
        h_page_next = *(uint16_t *)(h + 8);
        h0_vec.u64 = (h_page_current);
        h1_vec.u64 = (h_page_current >> 8) | (h_page_next << 56);
        h2_vec.u64 = (h_page_current >> 16) | (h_page_next << 48);
        h3_vec.u64 = (h_page_current >> 24) | (h_page_next << 40);
        h4_vec.u64 = (h_page_current >> 32) | (h_page_next << 32);
        matches0_vec = _sz_u64_each_3byte_equal(h0_vec, n_vec);
        matches1_vec = _sz_u64_each_3byte_equal(h1_vec, n_vec);
        matches2_vec = _sz_u64_each_3byte_equal(h2_vec, n_vec);
        matches3_vec = _sz_u64_each_3byte_equal(h3_vec, n_vec);
        matches4_vec = _sz_u64_each_3byte_equal(h4_vec, n_vec);

        if (matches0_vec.u64 | matches1_vec.u64 | matches2_vec.u64 | matches3_vec.u64 | matches4_vec.u64) {
            matches0_vec.u64 >>= 16;
            matches1_vec.u64 >>= 8;
            matches3_vec.u64 <<= 8;
            matches4_vec.u64 <<= 16;
            uint64_t match_indicators =
                matches0_vec.u64 | matches1_vec.u64 | matches2_vec.u64 | matches3_vec.u64 | matches4_vec.u64;
            return h + __builtin_ctzll(match_indicators) / 8;
        }
    }

    for (; h + 3 <= h_end; ++h)
        if ((h[0] == n[0]) + (h[1] == n[1]) + (h[2] == n[2]) == 3) return h;
    return nullptr;
}

uint8_t* sz_find_avx2(uint8_t* h, size_t h_length, uint8_t* n, size_t n_length) {

    // This almost never fires, but it's better to be safe than sorry.
    // if (h_length < n_length || !n_length) return SZ_NULL_CHAR;
    // if (n_length == 1) return sz_find_byte_avx2(h, h_length, n);

    // Pick the parts of the needle that are worth comparing.
    size_t offset_first, offset_mid, offset_last;
    _sz_locate_needle_anomalies(n, n_length, &offset_first, &offset_mid, &offset_last);

    // Broadcast those characters into YMM registers.
    int matches;
    sz_u256_vec_t h_first_vec, h_mid_vec, h_last_vec, n_first_vec, n_mid_vec, n_last_vec;
    n_first_vec.ymm = _mm256_set1_epi8(n[offset_first]);
    n_mid_vec.ymm = _mm256_set1_epi8(n[offset_mid]);
    n_last_vec.ymm = _mm256_set1_epi8(n[offset_last]);

    // Scan through the string.
    for (; h_length >= n_length + 32; h += 32, h_length -= 32) {
        h_first_vec.ymm = _mm256_lddqu_si256((__m256i const *)(h + offset_first));
        h_mid_vec.ymm = _mm256_lddqu_si256((__m256i const *)(h + offset_mid));
        h_last_vec.ymm = _mm256_lddqu_si256((__m256i const *)(h + offset_last));
        matches = _mm256_movemask_epi8(_mm256_cmpeq_epi8(h_first_vec.ymm, n_first_vec.ymm)) &
                  _mm256_movemask_epi8(_mm256_cmpeq_epi8(h_mid_vec.ymm, n_mid_vec.ymm)) &
                  _mm256_movemask_epi8(_mm256_cmpeq_epi8(h_last_vec.ymm, n_last_vec.ymm));
        while (matches) {
            int potential_offset = __builtin_ctz(matches);
            if (memcmp(h + potential_offset, n, n_length) == 0) return h + potential_offset;
            matches &= matches - 1;
        }
    }

    return _sz_find_3byte_serial(h, h_length, n);
}

// sz_cptr_t sz_find_neon(sz_cptr_t h, sz_size_t h_length, sz_cptr_t n, sz_size_t n_length) {

//     // This almost never fires, but it's better to be safe than sorry.
//     if (h_length < n_length || !n_length) return SZ_NULL_CHAR;
//     if (n_length == 1) return sz_find_byte_neon(h, h_length, n);

//     // Scan through the string.
//     // Assuming how tiny the Arm NEON registers are, we should avoid internal branches at all costs.
//     // That's why, for smaller needles, we use different loops.
//     if (n_length == 2) {
//         // Broadcast needle characters into SIMD registers.
//         sz_u64_t matches;
//         sz_u128_vec_t h_first_vec, h_last_vec, n_first_vec, n_last_vec, matches_vec;
//         // Dealing with 16-bit values, we can load 2 registers at a time and compare 31 possible offsets
//         // in a single loop iteration.
//         n_first_vec.u8x16 = vld1q_dup_u8((sz_u8_t const *)&n[0]);
//         n_last_vec.u8x16 = vld1q_dup_u8((sz_u8_t const *)&n[1]);
//         for (; h_length >= 17; h += 16, h_length -= 16) {
//             h_first_vec.u8x16 = vld1q_u8((sz_u8_t const *)(h + 0));
//             h_last_vec.u8x16 = vld1q_u8((sz_u8_t const *)(h + 1));
//             matches_vec.u8x16 =
//                 vandq_u8(vceqq_u8(h_first_vec.u8x16, n_first_vec.u8x16), vceqq_u8(h_last_vec.u8x16, n_last_vec.u8x16));
//             matches = vreinterpretq_u8_u4(matches_vec.u8x16);
//             if (matches) return h + sz_u64_ctz(matches) / 4;
//         }
//     }
//     else if (n_length == 3) {
//         // Broadcast needle characters into SIMD registers.
//         sz_u64_t matches;
//         sz_u128_vec_t h_first_vec, h_mid_vec, h_last_vec, n_first_vec, n_mid_vec, n_last_vec, matches_vec;
//         // Comparing 24-bit values is a bumer. Being lazy, I went with the same approach
//         // as when searching for string over 4 characters long. I only avoid the last comparison.
//         n_first_vec.u8x16 = vld1q_dup_u8((sz_u8_t const *)&n[0]);
//         n_mid_vec.u8x16 = vld1q_dup_u8((sz_u8_t const *)&n[1]);
//         n_last_vec.u8x16 = vld1q_dup_u8((sz_u8_t const *)&n[2]);
//         for (; h_length >= 18; h += 16, h_length -= 16) {
//             h_first_vec.u8x16 = vld1q_u8((sz_u8_t const *)(h + 0));
//             h_mid_vec.u8x16 = vld1q_u8((sz_u8_t const *)(h + 1));
//             h_last_vec.u8x16 = vld1q_u8((sz_u8_t const *)(h + 2));
//             matches_vec.u8x16 = vandq_u8(                           //
//                 vandq_u8(                                           //
//                     vceqq_u8(h_first_vec.u8x16, n_first_vec.u8x16), //
//                     vceqq_u8(h_mid_vec.u8x16, n_mid_vec.u8x16)),
//                 vceqq_u8(h_last_vec.u8x16, n_last_vec.u8x16));
//             matches = vreinterpretq_u8_u4(matches_vec.u8x16);
//             if (matches) return h + sz_u64_ctz(matches) / 4;
//         }
//     }
//     else {
//         // Pick the parts of the needle that are worth comparing.
//         sz_size_t offset_first, offset_mid, offset_last;
//         _sz_locate_needle_anomalies(n, n_length, &offset_first, &offset_mid, &offset_last);
//         // Broadcast those characters into SIMD registers.
//         sz_u64_t matches;
//         sz_u128_vec_t h_first_vec, h_mid_vec, h_last_vec, n_first_vec, n_mid_vec, n_last_vec, matches_vec;
//         n_first_vec.u8x16 = vld1q_dup_u8((sz_u8_t const *)&n[offset_first]);
//         n_mid_vec.u8x16 = vld1q_dup_u8((sz_u8_t const *)&n[offset_mid]);
//         n_last_vec.u8x16 = vld1q_dup_u8((sz_u8_t const *)&n[offset_last]);
//         // Walk through the string.
//         for (; h_length >= n_length + 16; h += 16, h_length -= 16) {
//             h_first_vec.u8x16 = vld1q_u8((sz_u8_t const *)(h + offset_first));
//             h_mid_vec.u8x16 = vld1q_u8((sz_u8_t const *)(h + offset_mid));
//             h_last_vec.u8x16 = vld1q_u8((sz_u8_t const *)(h + offset_last));
//             matches_vec.u8x16 = vandq_u8(                           //
//                 vandq_u8(                                           //
//                     vceqq_u8(h_first_vec.u8x16, n_first_vec.u8x16), //
//                     vceqq_u8(h_mid_vec.u8x16, n_mid_vec.u8x16)),
//                 vceqq_u8(h_last_vec.u8x16, n_last_vec.u8x16));
//             matches = vreinterpretq_u8_u4(matches_vec.u8x16);
//             while (matches) {
//                 int potential_offset = sz_u64_ctz(matches) / 4;
//                 if (sz_equal(h + potential_offset, n, n_length)) return h + potential_offset;
//                 matches &= matches - 1;
//             }
//         }
//     }

//     return sz_find_serial(h, h_length, n, n_length);
// }

// two-way
uint8_t* annexb_find_start_memmem(const uint8_t* bytes,size_t sizeBytes)
{
    return (uint8_t*)::memmem(bytes,sizeBytes,h26x_start_prefix,sizeof(h26x_start_prefix));
}

uint8_t* annexb_find_start_memcmp(const uint8_t* bytes,size_t sizeBytes){
    const uint8_t* p = bytes;
    const uint8_t* pend = bytes + sizeBytes;
    uint8_t* found = nullptr;
    while(p + 2 < pend){
        if(::memcmp(p,h26x_start_prefix,sizeof(h26x_start_prefix)) == 0){
            found = (uint8_t*)p;
            break;
        }
        ++p;
    }
    return found;
}

uint8_t* annexb_find_start_seq(const uint8_t* bytes,size_t sizeBytes)
{
    const uint8_t* p = bytes;
    const uint8_t* pend = bytes + sizeBytes;
    uint8_t* found = NULL;
    while(p + 2 < pend){
        // 0x00
        if(p[0] != 0x00){
            ++p;
            continue;
        }
        // 0x00 0x00
        if(p[1] != 0x00){
            p+=2;
            continue;
        }

        // 0x00 0x00 0x02
        if(p[2] > 0x01){
            p+=3;
            continue;
        }
        // 0x00 0x00 0x00
        if(p[2] == 0x00){
            ++p;
            continue;
        }else{// 0x00 0x00 0x01
            found = (uint8_t*)p;
            break;
        }
    }

    return found;
}

// This is sorta like Boyer-Moore, but with only the first optimization step:
// given a 3-byte sequence we're looking at, if the 3rd byte isn't 1 or 0,
// skip ahead to the next 3-byte sequence. 0s and 1s are relatively rare, so
// this will skip the majority of reads/checks.
const uint8_t* annexb_find_start_sbm(const uint8_t* bytes,size_t sizeBytes)
{
    const uint8_t* p = bytes;
    const uint8_t* pend = bytes + sizeBytes;
    const uint8_t* found = nullptr;

    while(p + 2 < pend){
        if(p[2] > 1){
            // x x 02...should not be start skip three byte
            p += 3;
        }else if(p[2] == 1){
            // x x 01 ... may be start check index 0 and 1
            if(p[0] == 0 && p[1] == 0){ //0 0 1 start
                // founded
                found = p;
                break;
            }

            // x y 1 not start skip three bytes
            p += 3;
        }else{// zero p[2]
            ++p;
        }
    }

    return found;
}

uint8_t* annexb_find_start_3byte(const uint8_t* bytes,size_t sizeBytes)
{
    return _sz_find_3byte_serial((uint8_t*)bytes,sizeBytes,(uint8_t*)h26x_start_prefix);
}
uint8_t* annexb_find_start_avx2(const uint8_t* bytes,size_t sizeBytes)
{
    return sz_find_avx2((uint8_t*)bytes,sizeBytes,(uint8_t*)h26x_start_prefix,3);
}

const uint8_t* annexb_find_next_nalu_start(const uint8_t* bytes,size_t sizeBytes,NALU_PREFIX_SIZE* prefix){
    // 至少需要3字节，如果最后的字节是00 00 01也应该抛弃掉 
    if(sizeBytes < 3) return nullptr;

    // default use sbm search (memmem(two-way) only on linux glibc)
    const uint8_t* found = annexb_find_start_sbm(bytes,sizeBytes);
    if(found){
        *prefix = NALU_PREFIX_SIZE::NALU_SHORT_PREFIX;
        if(found > bytes && *(found - 1) == 0x00){
            *prefix = NALU_PREFIX_SIZE::NALU_LONG_PREFIX;
            --found;
        }
    }
    return found;
}

int annexb_find_next_nalu(const uint8_t* bytes,size_t sizeBytes,h26x_nalu* nalu){
    // 能判断NALU类型的至少需要4字节，少于4字节不可识别
    // (00) 00 00 01 xx(type) xx(data)
    if(sizeBytes < 4) return 0;

    // 第一次查找头
    const uint8_t* p = bytes;
    size_t remain_size = sizeBytes;
    zav::NALU_PREFIX_SIZE prefix;
    const uint8_t* found = annexb_find_next_nalu_start(p,remain_size,&prefix);
    if(!found) return 0;
    
    // 第二次查找头
    nalu->prefix = prefix;
    nalu->start = found;

    p = found + prefix;
    remain_size -= prefix;
    found = annexb_find_next_nalu_start(p,remain_size,&prefix);
    if(!found){
        // 说明只有一个nalu
        nalu->end = bytes + sizeBytes - 1;
        return 1;
    }
    // 说明后面还有
    nalu->end = found - 1;
    return 1;
}

};//!namepsace h26x

const uint8_t* h264::annexb_skip_unsupported_nalu(const uint8_t* bytes,size_t sizeBytes){
    const uint8_t* p = bytes;
    const uint8_t* pend = bytes + sizeBytes;
    const uint8_t* support = NULL;
    h26x_nalu nalu;
    while(p < pend){
        int found = h26x::annexb_find_next_nalu(p, pend - p, &nalu);
        if(found){
            if(nalu.end > nalu.start + nalu.prefix){
                // 有效的nalu
                H264_NAL_UNIT_TYPE nalu_type = (H264_NAL_UNIT_TYPE)H264_NALU_TYPE(nalu.start[nalu.prefix]);
                if(nalu_type == H264_NALU_SEI
                    || nalu_type == H264_NALU_AUD
                    || nalu_type == H264_NALU_UNSPECIFIED
                    || nalu_type > H264_NALU_CODEC_SLICE_EXTENSION_3D_AVC
                    ){
                    // 0 6 9 22-31 skip
                    p = nalu.end + 1;
                    continue;
                }
                // 是IBP SPS PPS等等
                support = nalu.start;
                break;
            }
            // nalu.end == nalu.start + prefix 00 00 00 01 67没有后面的数据没有意义
            p = nalu.end + 1;
            continue;
        }
        // not found has no nalu,skip
        break;
    }
    return support;
}

const uint8_t* h265::annexb_skip_unsupported_nalu(const uint8_t* bytes,size_t sizeBytes){
    const uint8_t* p = bytes;
    const uint8_t* pend = bytes + sizeBytes;
    const uint8_t* support = NULL;
    h26x_nalu nalu;
    while(p < pend){
        int found = h26x::annexb_find_next_nalu(p, pend - p, &nalu);
        if(found){
            if(nalu.end > nalu.start + nalu.prefix){
                // 有效的nalu
                H265_NAL_UNIT_TYPE nalu_type = (H265_NAL_UNIT_TYPE)H265_NALU_TYPE(nalu.start[nalu.prefix]);
                if(nalu_type == H265_NALU_AUD || nalu_type > H265_NALU_PREFIX_SEI){
                    // 35 39-xxxx skip
                    p = nalu.end + 1;
                    continue;
                }
                // 是IBP SPS PPS等等
                support = nalu.start;
                break;
            }
            // nalu.end == nalu.start + prefix 00 00 00 01 67没有后面的数据没有意义
            p = nalu.end + 1;
            continue;
        }
        // not found has no nalu,skip
        break;
    }
    return support;
}

};//!namespace zav