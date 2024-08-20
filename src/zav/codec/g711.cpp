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

#include "zav/codec/g711.h"

namespace zav
{
#ifdef G711_LOOKUP_TABLE
/*
 * g711.c
 *
 * u-law, A-law and linear PCM conversions.
 * Source: http://www.speech.kth.se/cost250/refsys/latest/src/g711.c
 */
#define	SIGN_BIT	(0x80)		/* Sign bit for a A-law byte. */
#define	QUANT_MASK	(0xf)		/* Quantization field mask. */
#define	NSEGS		(8)		/* Number of A-law segments. */
#define	SEG_SHIFT	(4)		/* Left shift for segment number. */
#define	SEG_MASK	(0x70)		/* Segment field mask. */

static constexpr int16_t seg_aend[8] = 
    {
        0x1F, 0x3F, 0x7F, 0xFF,
        0x1FF, 0x3FF, 0x7FF, 0xFFF
    };
static constexpr int16_t seg_uend[8] = 
    {
        0x3F, 0x7F, 0xFF, 0x1FF,
        0x3FF, 0x7FF, 0xFFF, 0x1FFF
    };

/* copy from CCITT G.711 specifications */
static constexpr uint8_t _u2a[128] = 
    {/* u- to A-law conversions */
        1,  1,  2,  2,  3,  3,  4,  4,
        5,  5,  6,  6,  7,  7,  8,  8,
        9,  10, 11, 12, 13, 14, 15, 16,
        17,	18,	19,	20,	21,	22,	23,	24,
        25,	27,	29,	31,	33,	34,	35,	36,
        37,	38,	39,	40,	41,	42,	43,	44,
        46,	48,	49,	50,	51,	52,	53,	54,
        55,	56,	57,	58,	59,	60,	61,	62,
        64,	65,	66,	67,	68,	69,	70,	71,
        72,	73,	74,	75,	76,	77,	78,	79,
        /* corrected:
            81,	82,	83,	84,	85,	86,	87,	88, 
        should be: */
        80,	82,	83,	84,	85,	86,	87,	88,
        89,	90,	91,	92,	93,	94,	95,	96,
        97, 98, 99, 100,101,102,103,104,
        105,106,107,108,109,110,111,112,
        113,114,115,116,117,118,119,120,
        121,122,123,124,125,126,127,128
    };

static constexpr uint8_t _a2u[128] = 
    {			/* A- to u-law conversions */
        1,	3,	5,	7,	9,	11,	13,	15,
        16,	17,	18,	19,	20,	21,	22,	23,
        24,	25,	26,	27,	28,	29,	30,	31,
        32,	32,	33,	33,	34,	34,	35,	35,
        36,	37,	38,	39,	40,	41,	42,	43,
        44,	45,	46,	47,	48,	48,	49,	49,
        50,	51,	52,	53,	54,	55,	56,	57,
        58,	59,	60,	61,	62,	63,	64,	64,
        65,	66,	67,	68,	69,	70,	71,	72,
        /* corrected:
            73,	74,	75,	76,	77,	78,	79,	79,
        should be: */
        73,	74,	75,	76,	77,	78,	79,	80,
        80,	81,	82,	83,	84,	85,	86,	87,
        88,	89,	90,	91,	92,	93,	94,	95,
        96,	97,	98,	99,	100,101,102,103,
        104,105,106,107,108,109,110,111,
        112,113,114,115,116,117,118,119,
        120,121,122,123,124,125,126,127
    };

static int16_t search(int16_t val,const int16_t *table,int16_t size)
{
   int16_t i;
   
   for (i = 0; i < size; i++) {
      if (val <= *table++)
        return (i);
   }
   return (size);
}

/*
 * linear2alaw() - Convert a 16-bit linear PCM value to 8-bit A-law
 *
 * linear2alaw() accepts an 16-bit integer and encodes it as A-law data.
 *
 *		Linear Input Code	Compressed Code
 *	------------------------	---------------
 *	0000000wxyza			000wxyz
 *	0000001wxyza			001wxyz
 *	000001wxyzab			010wxyz
 *	00001wxyzabc			011wxyz
 *	0001wxyzabcd			100wxyz
 *	001wxyzabcde			101wxyz
 *	01wxyzabcdef			110wxyz
 *	1wxyzabcdefg			111wxyz
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 */
uint8_t linear2alaw(int16_t linear)/* 2's complement (16-bit range) */
{
   int16_t	 mask;
   int16_t	 seg;
   uint8_t aval;
   
   linear >>= 3;

   if (linear >= 0) {
      mask = 0xD5;		/* sign (7th) bit = 1 */
   } else {
      mask = 0x55;		/* sign bit = 0 */
      linear = -linear - 1;
   }
   
   /* Convert the scaled magnitude to segment number. */
   seg = search(linear, seg_aend, 8);
   
   /* Combine the sign, segment, and quantization bits. */
   
   if (seg >= 8)		/* out of range, return maximum value. */
      return (unsigned char) (0x7F ^ mask);
   else {
      aval = (unsigned char) seg << SEG_SHIFT;
      if (seg < 2)
	 aval |= (linear >> 1) & QUANT_MASK;
      else
	 aval |= (linear >> seg) & QUANT_MASK;
      return (aval ^ mask);
   }
}

/*
 * alaw2linear() - Convert an A-law value to 16-bit linear PCM
 *
 */
int16_t alaw2linear(uint8_t	alaw)
{
   int16_t t;
   int16_t seg;
   
   alaw ^= 0x55;
   
   t = (alaw & QUANT_MASK) << 4;
   seg = ((unsigned)alaw & SEG_MASK) >> SEG_SHIFT;
   switch (seg) {
   case 0:
      t += 8;
      break;
   case 1:
      t += 0x108;
      break;
   default:
      t += 0x108;
      t <<= seg - 1;
   }
   return ((alaw & SIGN_BIT) ? t : -t);
}

#define	BIAS		(0x84)		/* Bias for linear code. */
#define CLIP            8159

/*
* linear2ulaw() - Convert a linear PCM value to u-law
*
* In order to simplify the encoding process, the original linear magnitude
* is biased by adding 33 which shifts the encoding range from (0 - 8158) to
* (33 - 8191). The result can be seen in the following encoding table:
*
*	Biased Linear Input Code	Compressed Code
*	------------------------	---------------
*	00000001wxyza			000wxyz
*	0000001wxyzab			001wxyz
*	000001wxyzabc			010wxyz
*	00001wxyzabcd			011wxyz
*	0001wxyzabcde			100wxyz
*	001wxyzabcdef			101wxyz
*	01wxyzabcdefg			110wxyz
*	1wxyzabcdefgh			111wxyz
*
* Each biased linear code has a leading 1 which identifies the segment
* number. The value of the segment number is equal to 7 minus the number
* of leading 0's. The quantization interval is directly available as the
* four bits wxyz.  * The trailing bits (a - h) are ignored.
*
* Ordinarily the complement of the resulting code word is used for
* transmission, and so the code word is complemented before it is returned.
*
* For further information see John C. Bellamy's Digital Telephony, 1982,
* John Wiley & Sons, pps 98-111 and 472-476.
*/
uint8_t linear2ulaw(int16_t linear)	/* 2's complement (16-bit range) */
{
   int16_t         mask;
   int16_t	 seg;
   uint8_t ulaw;
   
   /* Get the sign and the magnitude of the value. */
   linear >>= 2;
   if (linear < 0) {
      linear = -linear;
      mask = 0x7F;
   } else {
      mask = 0xFF;
   }
   if ( linear > CLIP ) linear = CLIP;		/* clip the magnitude */
   linear += (BIAS >> 2);
   
   /* Convert the scaled magnitude to segment number. */
   seg = search(linear, seg_uend, 8);
   
   /*
   * Combine the sign, segment, quantization bits;
   * and complement the code word.
   */
   if (seg >= 8)		/* out of range, return maximum value. */
      return (unsigned char) (0x7F ^ mask);
   else {
      ulaw = (unsigned char) (seg << 4) | ((linear >> (seg + 1)) & 0xF);
      return (ulaw ^ mask);
   }
}

/*
 * ulaw2linear() - Convert a u-law value to 16-bit linear PCM
 *
 * First, a biased linear code is derived from the code word. An unbiased
 * output can then be obtained by subtracting 33 from the biased code.
 *
 * Note that this function expects to be passed the complement of the
 * original code word. This is in keeping with ISDN conventions.
 */
int16_t ulaw2linear(uint8_t ulaw)
{
   int16_t t;
   
   /* Complement to obtain normal u-law value. */
   ulaw = ~ulaw;
   
   /*
    * Extract and bias the quantization bits. Then
    * shift up by the segment number and subtract out the bias.
    */
   t = ((ulaw & QUANT_MASK) << 3) + BIAS;
   t <<= ((unsigned)ulaw & SEG_MASK) >> SEG_SHIFT;
   
   return ((ulaw & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
}

/* A-law to u-law conversion */
uint8_t alaw2ulaw(uint8_t alaw)
{
   alaw &= 0xff;
   return (unsigned char) ((alaw & 0x80) ? (0xFF ^ _a2u[alaw ^ 0xD5]) :
	   (0x7F ^ _a2u[alaw ^ 0x55]));
}

/* u-law to A-law conversion */
uint8_t ulaw2alaw(uint8_t ulaw)
{
   ulaw &= 0xff;
   return (unsigned char) ((ulaw & 0x80) ? (0xD5 ^ (_u2a[0xFF ^ ulaw] - 1)) :
			   (0x55 ^ (_u2a[0x7F ^ ulaw] - 1)));
}

#else // not G711_LOOKUP_TABLE

#ifdef __x86_64__
static int top_bit(unsigned int bits) {
  int res;

  __asm__ __volatile__(
      " movq $-1,%%rdx;\n"
      " bsrq %%rax,%%rdx;\n"
      : "=d"(res)
      : "a"(bits));
  return res;
}

static int bottom_bit(unsigned int bits) {
  int res;

  __asm__ __volatile__(
      " movq $-1,%%rdx;\n"
      " bsfq %%rax,%%rdx;\n"
      : "=d"(res)
      : "a"(bits));
  return res;
}
#else // not __x86_64__ back to c
static int top_bit(unsigned int bits) {
  int i;

  if (bits == 0) {
    return -1;
  }
  i = 0;
  if (bits & 0xFFFF0000) {
    bits &= 0xFFFF0000;
    i += 16;
  }
  if (bits & 0xFF00FF00) {
    bits &= 0xFF00FF00;
    i += 8;
  }
  if (bits & 0xF0F0F0F0) {
    bits &= 0xF0F0F0F0;
    i += 4;
  }
  if (bits & 0xCCCCCCCC) {
    bits &= 0xCCCCCCCC;
    i += 2;
  }
  if (bits & 0xAAAAAAAA) {
    bits &= 0xAAAAAAAA;
    i += 1;
  }
  return i;
}

static  int bottom_bit(unsigned int bits) {
  int i;

  if (bits == 0) {
    return -1;
  }
  i = 32;
  if (bits & 0x0000FFFF) {
    bits &= 0x0000FFFF;
    i -= 16;
  }
  if (bits & 0x00FF00FF) {
    bits &= 0x00FF00FF;
    i -= 8;
  }
  if (bits & 0x0F0F0F0F) {
    bits &= 0x0F0F0F0F;
    i -= 4;
  }
  if (bits & 0x33333333) {
    bits &= 0x33333333;
    i -= 2;
  }
  if (bits & 0x55555555) {
    bits &= 0x55555555;
    i -= 1;
  }
  return i;
}
#endif // 

#define ALAW_AMI_MASK 0x55
uint8_t	linear2alaw(int16_t linear)
{
    int16_t mask;
    int16_t seg;

    if (linear >= 0) {
        /* Sign (bit 7) bit = 1 */
        mask = ALAW_AMI_MASK | 0x80;
        } else {
        /* Sign (bit 7) bit = 0 */
        mask = ALAW_AMI_MASK;
        /* WebRtc, tlegrand: Changed from -8 to -1 to get bitexact to reference
            * implementation */
        linear = -linear - 1;
    }

    /* Convert the scaled magnitude to segment number. */
    seg = top_bit(linear | 0xFF) - 7;
    if (seg >= 8) {
        if (linear >= 0) {
            /* Out of range. Return maximum value. */
            return (uint8_t)(0x7F ^ mask);
        }
        /* We must be just a tiny step below zero */
        return (uint8_t)(0x00 ^ mask);
    }
    /* Combine the sign, segment, and quantization bits. */
    return (uint8_t)(((seg << 4) | ((linear >> ((seg) ? (seg + 3) : 4)) & 0x0F)) ^
                    mask);
}

int16_t	alaw2linear(uint8_t alaw)
{
    int i;
    int seg;

    alaw ^= ALAW_AMI_MASK;
    i = ((alaw & 0x0F) << 4);
    seg = (((int)alaw & 0x70) >> 4);
    if (seg)
        i = (i + 0x108) << (seg - 1);
    else
        i += 8;
    return (int16_t)((alaw & 0x80) ? i : -i);
}

#define ULAW_BIAS 0x84 /* Bias for linear code. */
uint8_t	linear2ulaw(int16_t linear)
{
    uint8_t u_val;
    int mask;
    int seg;

    /* Get the sign and the magnitude of the value. */
    if (linear < 0) {
        /* WebRtc, tlegrand: -1 added to get bitexact to reference implementation */
        linear = ULAW_BIAS - linear - 1;
        mask = 0x7F;
    } else {
        linear = ULAW_BIAS + linear;
        mask = 0xFF;
    }

    seg = top_bit(linear | 0xFF) - 7;

    /*
    * Combine the sign, segment, quantization bits,
    * and complement the code word.
    */
    if (seg >= 8)
        u_val = (uint8_t)(0x7F ^ mask);
    else
        u_val = (uint8_t)(((seg << 4) | ((linear >> (seg + 3)) & 0xF)) ^ mask);
    #ifdef ULAW_ZEROTRAP
    /* Optional ITU trap */
    if (u_val == 0)
        u_val = 0x02;
    #endif
    return u_val;
}

int16_t	ulaw2linear(uint8_t ulaw)
{
    int t;

    /* Complement to obtain normal u-law value. */
    ulaw = ~ulaw;
    /*
    * Extract and bias the quantization bits. Then
    * shift up by the segment number and subtract out the bias.
    */
    t = (((ulaw & 0x0F) << 3) + ULAW_BIAS) << (((int)ulaw & 0x70) >> 4);
    return (int16_t)((ulaw & 0x80) ? (ULAW_BIAS - t) : (t - ULAW_BIAS));
}

/* Copied from the CCITT G.711 specification */
static constexpr uint8_t ulaw_to_alaw_table[256] = {
   42,  43,  40,  41,  46,  47,  44,  45,  34,  35,  32,  33,  38,  39,  36,
   37,  58,  59,  56,  57,  62,  63,  60,  61,  50,  51,  48,  49,  54,  55,
   52,  53,  10,  11,   8,   9,  14,  15,  12,  13,   2,   3,   0,   1,   6,
    7,   4,  26,  27,  24,  25,  30,  31,  28,  29,  18,  19,  16,  17,  22,
   23,  20,  21, 106, 104, 105, 110, 111, 108, 109,  98,  99,  96,  97, 102,
  103, 100, 101, 122, 120, 126, 127, 124, 125, 114, 115, 112, 113, 118, 119,
  116, 117,  75,  73,  79,  77,  66,  67,  64,  65,  70,  71,  68,  69,  90,
   91,  88,  89,  94,  95,  92,  93,  82,  82,  83,  83,  80,  80,  81,  81,
   86,  86,  87,  87,  84,  84,  85,  85, 170, 171, 168, 169, 174, 175, 172,
  173, 162, 163, 160, 161, 166, 167, 164, 165, 186, 187, 184, 185, 190, 191,
  188, 189, 178, 179, 176, 177, 182, 183, 180, 181, 138, 139, 136, 137, 142,
  143, 140, 141, 130, 131, 128, 129, 134, 135, 132, 154, 155, 152, 153, 158,
  159, 156, 157, 146, 147, 144, 145, 150, 151, 148, 149, 234, 232, 233, 238,
  239, 236, 237, 226, 227, 224, 225, 230, 231, 228, 229, 250, 248, 254, 255,
  252, 253, 242, 243, 240, 241, 246, 247, 244, 245, 203, 201, 207, 205, 194,
  195, 192, 193, 198, 199, 196, 197, 218, 219, 216, 217, 222, 223, 220, 221,
  210, 210, 211, 211, 208, 208, 209, 209, 214, 214, 215, 215, 212, 212, 213,
  213
};

/* These transcoding tables are copied from the CCITT G.711 specification. To
   achieve optimal results, do not change them. */
static constexpr uint8_t alaw_to_ulaw_table[256] = {
   42,  43,  40,  41,  46,  47,  44,  45,  34,  35,  32,  33,  38,  39,  36,
   37,  57,  58,  55,  56,  61,  62,  59,  60,  49,  50,  47,  48,  53,  54,
   51,  52,  10,  11,   8,   9,  14,  15,  12,  13,   2,   3,   0,   1,   6,
    7,   4,   5,  26,  27,  24,  25,  30,  31,  28,  29,  18,  19,  16,  17,
   22,  23,  20,  21,  98,  99,  96,  97, 102, 103, 100, 101,  93,  93,  92,
   92,  95,  95,  94,  94, 116, 118, 112, 114, 124, 126, 120, 122, 106, 107,
  104, 105, 110, 111, 108, 109,  72,  73,  70,  71,  76,  77,  74,  75,  64,
   65,  63,  63,  68,  69,  66,  67,  86,  87,  84,  85,  90,  91,  88,  89,
   79,  79,  78,  78,  82,  83,  80,  81, 170, 171, 168, 169, 174, 175, 172,
  173, 162, 163, 160, 161, 166, 167, 164, 165, 185, 186, 183, 184, 189, 190,
  187, 188, 177, 178, 175, 176, 181, 182, 179, 180, 138, 139, 136, 137, 142,
  143, 140, 141, 130, 131, 128, 129, 134, 135, 132, 133, 154, 155, 152, 153,
  158, 159, 156, 157, 146, 147, 144, 145, 150, 151, 148, 149, 226, 227, 224,
  225, 230, 231, 228, 229, 221, 221, 220, 220, 223, 223, 222, 222, 244, 246,
  240, 242, 252, 254, 248, 250, 234, 235, 232, 233, 238, 239, 236, 237, 200,
  201, 198, 199, 204, 205, 202, 203, 192, 193, 191, 191, 196, 197, 194, 195,
  214, 215, 212, 213, 218, 219, 216, 217, 207, 207, 206, 206, 210, 211, 208,
  209
};

uint8_t	alaw2ulaw(uint8_t alaw)
{
    return alaw_to_ulaw_table[alaw];
}

uint8_t	ulaw2alaw(uint8_t ulaw)
{
    return ulaw_to_alaw_table[ulaw];
}
#endif
}