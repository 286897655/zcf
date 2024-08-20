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

/**
 * @brief g722.1 codec history describe below
*/

/*
 * g722_1 - a library for the G.722.1 and Annex C codecs
 *
 * g722_1.h
 *
 * Adapted by Steve Underwood <steveu@coppice.org> from the reference
 * code supplied with ITU G.722.1, which is:
 *
 *   (C) 2004 Polycom, Inc.
 *   All rights reserved.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Modification for zav,2024 by zhaoj
 * remove 
 * 
 */

#ifndef ZAV_G722_1_H_
#define ZAV_G722_1_H_

#include <stddef.h>
#include <stdint.h>
#include <zav/av.h>

namespace zav{
struct g722_1_decoder;
struct g722_1_encoder;

// g722.1 support samplerate 16000/32000
enum G722_1_SupportSampleRate{
    // Basic G.722.1 sampling rate
    G722_1_SAMPLE_RATE_16000 = 16000,
    // G.722.1 Annex C sampling rate
    G722_1_SAMPLE_RATE_32000 = 32000
};

// g722.1 support bitrate 24k/32k/48k
enum G722_1_BitRateMode{
    G722_1_BIT_RATE_24000 = 24000,
    G722_1_BIT_RATE_32000 = 32000,
    G722_1_BIT_RATE_48000 = 48000
};

// this encoder and decoder use BigEndian 
// if input data use little endian,should convert
enum G722_1_BitStream_PackMode{
    G722_1_BITSTREAM_PACKED_BE = 0,
    G722_1_BITSTREAM_PACKED_LE = 1
};

/// @brief 16000 samplerate decoded framesize 320 int16_t
///        32000 samplerate decoded framesize 640 int16_t
///        16000 bitrate input g7221 frame 40 uint8_t 16000/50/8
///        24000 bitrate input g7221 frame 60 uint8_t
///        32000 bitrate input g7221 frame 80 uint8_t
///        48000 bitrate input g7221 frame 120 uint8_t
///        g7221 20ms one frame
///        max 5 frame 100ms -> int16_t 5*640 3200 int16_t
///                          -> uint8_t 5*120 600 int16_t
enum {
    MAX_G722_1_FRAME = 5
};
class G722_1_Decoder final{
public:
    G722_1_Decoder(G722_1_SupportSampleRate samplerate,G722_1_BitRateMode bitrate,G722_1_BitStream_PackMode packmode);
    ~G722_1_Decoder();

    void Reset(G722_1_SupportSampleRate samplerate,G722_1_BitRateMode bitrate,G722_1_BitStream_PackMode packmode);
    size_t Decode(const uint8_t* g722_1_data,size_t len,const int16_t* amp);
    int Decode(const uint8_t* g722_1_data,size_t len,pcm_buf* pcmbuf);
private:
    struct g722_1_decoder* decoder_;
    G722_1_BitStream_PackMode pack_mode_;
    int16_t* amp_buf_;
    size_t g7221_frame_len_;
    size_t amp_frame_len_;
};

class G722_1_Encoder final{
public:
    G722_1_Encoder(G722_1_SupportSampleRate samplerate,G722_1_BitRateMode bitrate);
    ~G722_1_Encoder();

    void Reset(G722_1_SupportSampleRate samplerate,G722_1_BitRateMode bitrate);
    size_t Encode(const int16_t* amp,size_t len,const uint8_t* g722_1_data);
private:
    struct g722_1_encoder* encoder_;
};
};//!namespace zav
#endif//!ZAV_G722_1_H_