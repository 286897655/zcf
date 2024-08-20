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
 * @brief g722 codec history describe below
*/

/*
 * SpanDSP - a series of DSP components for telephony
 *
 * g722.h - The ITU G.722 codec.
 *
 * Written by Steve Underwood <steveu@coppice.org>
 *
 * Copyright (C) 2005 Steve Underwood
 *
 *  Despite my general liking of the GPL, I place my own contributions
 *  to this code in the public domain for the benefit of all mankind -
 *  even the slimy ones who might try to proprietize my work and use it
 *  to my detriment.
 *
 * Based on a single channel G.722 codec which is:
 *
 *****    Copyright (c) CMU    1993      *****
 * Computer Science, Speech Group
 * Chengxiang Lu and Alex Hauptmann
 *
 * $Id: g722.h,v 1.10 2006/06/16 12:45:53 steveu Exp $
 *
 * Modifications for WebRtc, 2011/04/28, by tlegrand:
 * -Changed to use WebRtc types
 * -Added new defines for minimum and maximum values of short int
 * 
 * Modification for zav,2024 by zhaoj
 * -changed structure and option rate mode
 */

#ifndef ZAV_CODEC_G722_H_
#define ZAV_CODEC_G722_H_

#include <stddef.h>
#include <stdint.h>

namespace zav{

/*! \page g722_page G.722 encoding and decoding
\section g722_page_sec_1 What does it do?
The G.722 module is a bit exact implementation of the ITU G.722 specification
for all three specified bit rates - 64000bps, 56000bps and 48000bps. It passes
the ITU tests.

To allow fast and flexible interworking with narrow band telephony, the encoder
and decoder support an option for the linear audio to be an 8k samples/second
stream. In this mode the codec is considerably faster, and still fully
compatible with wideband terminals using G.722.

\section g722_page_sec_2 How does it work?
???.
*/

/*! bits_per_sample 6 for 48000kbps, 7 for 56000kbps, or 8 for 64000kbps. */
enum G722BitRateMode{
    g722_48000_bps,
    g722_56000_bps,
    g722_64000_bps
};

enum G722SampleOption{
    G722_SAMPLE_RATE_8000 = 0x0001,//8000khz
    G722_PACKED = 0x0002 //16khz
};

typedef struct G722State{
  /*! TRUE if the operating in the special ITU test mode, with the band split
     filters disabled. */
  int itu_test_mode;
  /*! TRUE if the G.722 data is packed */
  int packed;
  /*! TRUE if decode to 8k samples/second */
  int eight_k;
  /*! 6 for 48000kbps, 7 for 56000kbps, or 8 for 64000kbps. */
  int bits_per_sample;

  /*! Signal history for the QMF */
  int x[24];

  struct {
    int s;
    int sp;
    int sz;
    int r[3];
    int a[3];
    int ap[3];
    int p[3];
    int d[7];
    int b[7];
    int bp[7];
    int sg[7];
    int nb;
    int det;
  } band[2];

  unsigned int in_buffer;
  int in_bits;
  unsigned int out_buffer;
  int out_bits;
} G722DecoderState,G722EncoderState;

class G722Decoder{
public:
    explicit G722Decoder(G722BitRateMode bitrate,G722SampleOption option);
    ~G722Decoder();


    size_t Decode(const uint8_t* g722_data,size_t len,int16_t* amp);

    void Reset(G722BitRateMode bitrate,G722SampleOption option);
private:
    G722DecoderState* decoder_state_;
};

class G722Encoder{
public:
    explicit G722Encoder(G722BitRateMode bitrate,G722SampleOption option);
    ~G722Encoder();

    size_t Encode(const int16_t* amp,size_t len,uint8_t* g722_data);
    void Reset(G722BitRateMode bitrate,G722SampleOption option);
private:
    G722EncoderState* encoder_state_;
};

};//!namespace zav

#endif //!ZAV_CODEC_G722_H_