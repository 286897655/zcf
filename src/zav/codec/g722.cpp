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
#include "zav/codec/g722.h"
#include <string.h>
#include <zcf/zcf.h>

#if !defined(FALSE)
#define FALSE 0
#endif
#if !defined(TRUE)
#define TRUE (!FALSE)
#endif

#define WEBRTC_INT16_MAX 32767
#define WEBRTC_INT16_MIN -32768

namespace zav{

static constexpr int q6[32] =
    {
           0,   35,   72,  110,  150,  190,  233,  276,
         323,  370,  422,  473,  530,  587,  650,  714,
         786,  858,  940, 1023, 1121, 1219, 1339, 1458,
        1612, 1765, 1980, 2195, 2557, 2919,    0,    0
    };

static constexpr int iln[32] =
    {
         0, 63, 62, 31, 30, 29, 28, 27,
        26, 25, 24, 23, 22, 21, 20, 19,
        18, 17, 16, 15, 14, 13, 12, 11,
        10,  9,  8,  7,  6,  5,  4,  0
    };

static constexpr int ilp[32] =
    {
         0, 61, 60, 59, 58, 57, 56, 55,
        54, 53, 52, 51, 50, 49, 48, 47,
        46, 45, 44, 43, 42, 41, 40, 39,
        38, 37, 36, 35, 34, 33, 32,  0
    };
static constexpr int wl[8] =
    {
        -60, -30, 58, 172, 334, 538, 1198, 3042
    };
static constexpr int rl42[16] =
    {
        0, 7, 6, 5, 4, 3, 2, 1, 7, 6, 5, 4, 3, 2, 1, 0
    };
static constexpr int ilb[32] =
    {
        2048, 2093, 2139, 2186, 2233, 2282, 2332,
        2383, 2435, 2489, 2543, 2599, 2656, 2714,
        2774, 2834, 2896, 2960, 3025, 3091, 3158,
        3228, 3298, 3371, 3444, 3520, 3597, 3676,
        3756, 3838, 3922, 4008
    };
static constexpr int qm2[4] =
    {
        -7408,  -1616,   7408,   1616
    };
static constexpr int qm4[16] =
    {
             0, -20456, -12896, -8968,
         -6288,  -4240,  -2584, -1200,
         20456,  12896,   8968,  6288,
          4240,   2584,   1200,     0
    };
static constexpr int qm5[32] =
    {
           -280,   -280, -23352, -17560,
         -14120, -11664,  -9752,  -8184,
          -6864,  -5712,  -4696,  -3784,
          -2960,  -2208,  -1520,   -880,
          23352,  17560,  14120,  11664,
           9752,   8184,   6864,   5712,
           4696,   3784,   2960,   2208,
           1520,    880,    280,   -280
    };
static constexpr int qm6[64] =
    {
           -136,   -136,   -136,   -136,
         -24808, -21904, -19008, -16704,
         -14984, -13512, -12280, -11192,
         -10232,  -9360,  -8576,  -7856,
          -7192,  -6576,  -6000,  -5456,
          -4944,  -4464,  -4008,  -3576,
          -3168,  -2776,  -2400,  -2032,
          -1688,  -1360,  -1040,   -728,
          24808,  21904,  19008,  16704,
          14984,  13512,  12280,  11192,
          10232,   9360,   8576,   7856,
           7192,   6576,   6000,   5456,
           4944,   4464,   4008,   3576,
           3168,   2776,   2400,   2032,
           1688,   1360,   1040,    728,
            432,    136,   -432,   -136
    };
static constexpr int qmf_coeffs[12] =
    {
           3,  -11,   12,   32, -210,  951, 3876, -805,  362, -156,   53,  -11,
    };
static constexpr int ihn[3] = {0, 1, 0};
static constexpr int ihp[3] = {0, 3, 2};
static constexpr int wh[3] = {0, -214, 798};
static constexpr int rh2[4] = {2, 1, 2, 1};

static int16_t saturate(int32_t amp)
{
    int16_t amp16;

    /* Hopefully this is optimised for the common case - not clipping */
    amp16 = (int16_t) amp;
    if (amp == amp16)
        return amp16;
    if (amp > WEBRTC_INT16_MAX)
        return  WEBRTC_INT16_MAX;
    return  WEBRTC_INT16_MIN;
}

static void block4(G722DecoderState *s, int band, int d)
{
    int wd1;
    int wd2;
    int wd3;
    int i;

    /* Block 4, RECONS */
    s->band[band].d[0] = d;
    s->band[band].r[0] = saturate(s->band[band].s + d);

    /* Block 4, PARREC */
    s->band[band].p[0] = saturate(s->band[band].sz + d);

    /* Block 4, UPPOL2 */
    for (i = 0;  i < 3;  i++)
        s->band[band].sg[i] = s->band[band].p[i] >> 15;
    wd1 = saturate(s->band[band].a[1] * 4);

    wd2 = (s->band[band].sg[0] == s->band[band].sg[1])  ?  -wd1  :  wd1;
    if (wd2 > 32767)
        wd2 = 32767;
    wd3 = (s->band[band].sg[0] == s->band[band].sg[2])  ?  128  :  -128;
    wd3 += (wd2 >> 7);
    wd3 += (s->band[band].a[2]*32512) >> 15;
    if (wd3 > 12288)
        wd3 = 12288;
    else if (wd3 < -12288)
        wd3 = -12288;
    s->band[band].ap[2] = wd3;

    /* Block 4, UPPOL1 */
    s->band[band].sg[0] = s->band[band].p[0] >> 15;
    s->band[band].sg[1] = s->band[band].p[1] >> 15;
    wd1 = (s->band[band].sg[0] == s->band[band].sg[1])  ?  192  :  -192;
    wd2 = (s->band[band].a[1]*32640) >> 15;

    s->band[band].ap[1] = saturate(wd1 + wd2);
    wd3 = saturate(15360 - s->band[band].ap[2]);
    if (s->band[band].ap[1] > wd3)
        s->band[band].ap[1] = wd3;
    else if (s->band[band].ap[1] < -wd3)
        s->band[band].ap[1] = -wd3;

    /* Block 4, UPZERO */
    wd1 = (d == 0)  ?  0  :  128;
    s->band[band].sg[0] = d >> 15;
    for (i = 1;  i < 7;  i++)
    {
        s->band[band].sg[i] = s->band[band].d[i] >> 15;
        wd2 = (s->band[band].sg[i] == s->band[band].sg[0])  ?  wd1  :  -wd1;
        wd3 = (s->band[band].b[i]*32640) >> 15;
        s->band[band].bp[i] = saturate(wd2 + wd3);
    }

    /* Block 4, DELAYA */
    for (i = 6;  i > 0;  i--)
    {
        s->band[band].d[i] = s->band[band].d[i - 1];
        s->band[band].b[i] = s->band[band].bp[i];
    }

    for (i = 2;  i > 0;  i--)
    {
        s->band[band].r[i] = s->band[band].r[i - 1];
        s->band[band].p[i] = s->band[band].p[i - 1];
        s->band[band].a[i] = s->band[band].ap[i];
    }

    /* Block 4, FILTEP */
    wd1 = saturate(s->band[band].r[1] + s->band[band].r[1]);
    wd1 = (s->band[band].a[1]*wd1) >> 15;
    wd2 = saturate(s->band[band].r[2] + s->band[band].r[2]);
    wd2 = (s->band[band].a[2]*wd2) >> 15;
    s->band[band].sp = saturate(wd1 + wd2);

    /* Block 4, FILTEZ */
    s->band[band].sz = 0;
    for (i = 6;  i > 0;  i--)
    {
        wd1 = saturate(s->band[band].d[i] + s->band[band].d[i]);
        s->band[band].sz += (s->band[band].b[i]*wd1) >> 15;
    }
    s->band[band].sz = saturate(s->band[band].sz);

    /* Block 4, PREDIC */
    s->band[band].s = saturate(s->band[band].sp + s->band[band].sz);
}

static void reset_state(G722State* s,G722BitRateMode bitrate,G722SampleOption option){
    ::memset(s,0,sizeof(G722State));
    if (bitrate == g722_48000_bps){
        s->bits_per_sample = 6;
    }else if(bitrate == g722_56000_bps){
        s->bits_per_sample = 7;
    }else{// 64k
        s->bits_per_sample = 8;
    }

    if ((option & G722_SAMPLE_RATE_8000)){
        s->eight_k = TRUE;
    }
        
    if ((option & G722_PACKED)  &&  s->bits_per_sample != 8){
        s->packed = TRUE;
    }else{
        s->packed = FALSE;
    }
        
    s->band[0].det = 32;
    s->band[1].det = 8;
}

static size_t g722_decode(G722DecoderState* s,
    const uint8_t* g722_data,size_t len,
    int16_t* amp){
    int dlowt;
    int rlow;
    int ihigh;
    int dhigh;
    int rhigh;
    int xout1;
    int xout2;
    int wd1;
    int wd2;
    int wd3;
    int code;
    size_t outlen;
    int i;
    size_t j;

    outlen = 0;
    rhigh = 0;
    for (j = 0;  j < len;  )
    {
        if (s->packed)
        {
            /* Unpack the code bits */
            if (s->in_bits < s->bits_per_sample)
            {
                s->in_buffer |= (g722_data[j++] << s->in_bits);
                s->in_bits += 8;
            }
            code = s->in_buffer & ((1 << s->bits_per_sample) - 1);
            s->in_buffer >>= s->bits_per_sample;
            s->in_bits -= s->bits_per_sample;
        }
        else
        {
            code = g722_data[j++];
        }

        switch (s->bits_per_sample)
        {
        default:
        case 8:
            wd1 = code & 0x3F;
            ihigh = (code >> 6) & 0x03;
            wd2 = qm6[wd1];
            wd1 >>= 2;
            break;
        case 7:
            wd1 = code & 0x1F;
            ihigh = (code >> 5) & 0x03;
            wd2 = qm5[wd1];
            wd1 >>= 1;
            break;
        case 6:
            wd1 = code & 0x0F;
            ihigh = (code >> 4) & 0x03;
            wd2 = qm4[wd1];
            break;
        }
        /* Block 5L, LOW BAND INVQBL */
        wd2 = (s->band[0].det*wd2) >> 15;
        /* Block 5L, RECONS */
        rlow = s->band[0].s + wd2;
        /* Block 6L, LIMIT */
        if (rlow > 16383)
            rlow = 16383;
        else if (rlow < -16384)
            rlow = -16384;

        /* Block 2L, INVQAL */
        wd2 = qm4[wd1];
        dlowt = (s->band[0].det*wd2) >> 15;

        /* Block 3L, LOGSCL */
        wd2 = rl42[wd1];
        wd1 = (s->band[0].nb*127) >> 7;
        wd1 += wl[wd2];
        if (wd1 < 0)
            wd1 = 0;
        else if (wd1 > 18432)
            wd1 = 18432;
        s->band[0].nb = wd1;

        /* Block 3L, SCALEL */
        wd1 = (s->band[0].nb >> 6) & 31;
        wd2 = 8 - (s->band[0].nb >> 11);
        wd3 = (wd2 < 0)  ?  (ilb[wd1] << -wd2)  :  (ilb[wd1] >> wd2);
        s->band[0].det = wd3 << 2;

        block4(s, 0, dlowt);

        if (!s->eight_k)
        {
            /* Block 2H, INVQAH */
            wd2 = qm2[ihigh];
            dhigh = (s->band[1].det*wd2) >> 15;
            /* Block 5H, RECONS */
            rhigh = dhigh + s->band[1].s;
            /* Block 6H, LIMIT */
            if (rhigh > 16383)
                rhigh = 16383;
            else if (rhigh < -16384)
                rhigh = -16384;

            /* Block 2H, INVQAH */
            wd2 = rh2[ihigh];
            wd1 = (s->band[1].nb*127) >> 7;
            wd1 += wh[wd2];
            if (wd1 < 0)
                wd1 = 0;
            else if (wd1 > 22528)
                wd1 = 22528;
            s->band[1].nb = wd1;

            /* Block 3H, SCALEH */
            wd1 = (s->band[1].nb >> 6) & 31;
            wd2 = 10 - (s->band[1].nb >> 11);
            wd3 = (wd2 < 0)  ?  (ilb[wd1] << -wd2)  :  (ilb[wd1] >> wd2);
            s->band[1].det = wd3 << 2;

            block4(s, 1, dhigh);
        }

        if (s->itu_test_mode)
        {
            amp[outlen++] = (int16_t) (rlow << 1);
            amp[outlen++] = (int16_t) (rhigh << 1);
        }
        else
        {
            if (s->eight_k)
            {
                amp[outlen++] = (int16_t) (rlow << 1);
            }
            else
            {
                /* Apply the receive QMF */
                for (i = 0;  i < 22;  i++)
                    s->x[i] = s->x[i + 2];
                s->x[22] = rlow + rhigh;
                s->x[23] = rlow - rhigh;

                xout1 = 0;
                xout2 = 0;
                for (i = 0;  i < 12;  i++)
                {
                    xout2 += s->x[2*i]*qmf_coeffs[i];
                    xout1 += s->x[2*i + 1]*qmf_coeffs[11 - i];
                }
                /* We shift by 12 to allow for the QMF filters (DC gain = 4096), less 1
                   to allow for the 15 bit input to the G.722 algorithm. */
                /* WebRtc, tlegrand: added saturation */
                amp[outlen++] = saturate(xout1 >> 11);
                amp[outlen++] = saturate(xout2 >> 11);
            }
        }
    }
    return outlen;
}

static size_t g722_encode(G722EncoderState* s,const int16_t* amp,size_t len,uint8_t* g722_data){
    int dlow;
    int dhigh;
    int el;
    int wd;
    int wd1;
    int ril;
    int wd2;
    int il4;
    int ih2;
    int wd3;
    int eh;
    int mih;
    int i;
    size_t j;
    /* Low and high band PCM from the QMF */
    int xlow;
    int xhigh;
    size_t g722_bytes;
    /* Even and odd tap accumulators */
    int sumeven;
    int sumodd;
    int ihigh;
    int ilow;
    int code;

    g722_bytes = 0;
    xhigh = 0;
    for (j = 0;  j < len;  )
    {
        if (s->itu_test_mode)
        {
            xlow =
            xhigh = amp[j++] >> 1;
        }
        else
        {
            if (s->eight_k)
            {
                /* We shift by 1 to allow for the 15 bit input to the G.722 algorithm. */
                xlow = amp[j++] >> 1;
            }
            else
            {
                /* Apply the transmit QMF */
                /* Shuffle the buffer down */
                for (i = 0;  i < 22;  i++)
                    s->x[i] = s->x[i + 2];
                s->x[22] = amp[j++];
                s->x[23] = amp[j++];

                /* Discard every other QMF output */
                sumeven = 0;
                sumodd = 0;
                for (i = 0;  i < 12;  i++)
                {
                    sumodd += s->x[2*i]*qmf_coeffs[i];
                    sumeven += s->x[2*i + 1]*qmf_coeffs[11 - i];
                }
                /* We shift by 12 to allow for the QMF filters (DC gain = 4096), plus 1
                   to allow for us summing two filters, plus 1 to allow for the 15 bit
                   input to the G.722 algorithm. */
                xlow = (sumeven + sumodd) >> 14;
                xhigh = (sumeven - sumodd) >> 14;

#ifdef RUN_LIKE_REFERENCE_G722
                /* The following lines are only used to verify bit-exactness
                 * with reference implementation of G.722. Higher precision
                 * is achieved without limiting the values.
                 */
                xlow = limitValues(xlow);
                xhigh = limitValues(xhigh);
#endif
            }
        }
        /* Block 1L, SUBTRA */
        el = saturate(xlow - s->band[0].s);

        /* Block 1L, QUANTL */
        wd = (el >= 0)  ?  el  :  -(el + 1);

        for (i = 1;  i < 30;  i++)
        {
            wd1 = (q6[i]*s->band[0].det) >> 12;
            if (wd < wd1)
                break;
        }
        ilow = (el < 0)  ?  iln[i]  :  ilp[i];

        /* Block 2L, INVQAL */
        ril = ilow >> 2;
        wd2 = qm4[ril];
        dlow = (s->band[0].det*wd2) >> 15;

        /* Block 3L, LOGSCL */
        il4 = rl42[ril];
        wd = (s->band[0].nb*127) >> 7;
        s->band[0].nb = wd + wl[il4];
        if (s->band[0].nb < 0)
            s->band[0].nb = 0;
        else if (s->band[0].nb > 18432)
            s->band[0].nb = 18432;

        /* Block 3L, SCALEL */
        wd1 = (s->band[0].nb >> 6) & 31;
        wd2 = 8 - (s->band[0].nb >> 11);
        wd3 = (wd2 < 0)  ?  (ilb[wd1] << -wd2)  :  (ilb[wd1] >> wd2);
        s->band[0].det = wd3 << 2;

        block4(s, 0, dlow);

        if (s->eight_k)
        {
            /* Just leave the high bits as zero */
            code = (0xC0 | ilow) >> (8 - s->bits_per_sample);
        }
        else
        {
            /* Block 1H, SUBTRA */
            eh = saturate(xhigh - s->band[1].s);

            /* Block 1H, QUANTH */
            wd = (eh >= 0)  ?  eh  :  -(eh + 1);
            wd1 = (564*s->band[1].det) >> 12;
            mih = (wd >= wd1)  ?  2  :  1;
            ihigh = (eh < 0)  ?  ihn[mih]  :  ihp[mih];

            /* Block 2H, INVQAH */
            wd2 = qm2[ihigh];
            dhigh = (s->band[1].det*wd2) >> 15;

            /* Block 3H, LOGSCH */
            ih2 = rh2[ihigh];
            wd = (s->band[1].nb*127) >> 7;
            s->band[1].nb = wd + wh[ih2];
            if (s->band[1].nb < 0)
                s->band[1].nb = 0;
            else if (s->band[1].nb > 22528)
                s->band[1].nb = 22528;

            /* Block 3H, SCALEH */
            wd1 = (s->band[1].nb >> 6) & 31;
            wd2 = 10 - (s->band[1].nb >> 11);
            wd3 = (wd2 < 0)  ?  (ilb[wd1] << -wd2)  :  (ilb[wd1] >> wd2);
            s->band[1].det = wd3 << 2;

            block4(s, 1, dhigh);
            code = ((ihigh << 6) | ilow) >> (8 - s->bits_per_sample);
        }

        if (s->packed)
        {
            /* Pack the code bits */
            s->out_buffer |= (code << s->out_bits);
            s->out_bits += s->bits_per_sample;
            if (s->out_bits >= 8)
            {
                g722_data[g722_bytes++] = (uint8_t) (s->out_buffer & 0xFF);
                s->out_bits -= 8;
                s->out_buffer >>= 8;
            }
        }
        else
        {
            g722_data[g722_bytes++] = (uint8_t) code;
        }
    }
    return g722_bytes;
}

G722Decoder::G722Decoder(G722BitRateMode bitrate,G722SampleOption option){
    decoder_state_ = new G722DecoderState();
    Reset(bitrate,option);
}

G722Decoder::~G722Decoder(){
    delete decoder_state_;
    decoder_state_ = nullptr;
}

void G722Decoder::Reset(G722BitRateMode bitrate,G722SampleOption option){
    Z_ASSERT(decoder_state_);
    reset_state(decoder_state_,bitrate,option);
}

size_t G722Decoder::Decode(const uint8_t* g722_data,size_t len,int16_t* amp){
    return g722_decode(decoder_state_,g722_data,len,amp);
}

G722Encoder::G722Encoder(G722BitRateMode bitrate,G722SampleOption option){
    encoder_state_ = new G722EncoderState();
    Reset(bitrate,option);
}

G722Encoder::~G722Encoder(){
    delete encoder_state_;
    encoder_state_ = nullptr;
}

size_t G722Encoder::Encode(const int16_t* amp,size_t len,uint8_t* g722_data){
    return g722_encode(encoder_state_,amp,len,g722_data);
}

void G722Encoder::Reset(G722BitRateMode bitrate,G722SampleOption option){
    Z_ASSERT(encoder_state_);
    reset_state(encoder_state_,bitrate,option);
}

};//!namesapce zav