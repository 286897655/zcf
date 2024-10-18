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
#ifndef ZAV_CODEC_H26X_H_
#define ZAV_CODEC_H26X_H_

#include <stdint.h>
#include <stddef.h>

//for nalu data first byte
#define H265_NALU_TYPE(v) (((uint8_t)(v) >> 1) & 0x3f)// equals (((uint8_t)(v) & 0x7E) >> 1)
#define H264_NALU_TYPE(v) ((uint8_t)(v) & 0x1F)

#define FORBIDDEN_ZERO_BIT(v) (uint8_t(v) >> 7)

namespace zav{

/**
 * Table 7-1 - NAL unit type codes, syntax element categories, and NAL unit type classes
 * T-REC-H.264-202408-P!!PDF-E.pdf, page 87.
 */
enum H264_NAL_UNIT_TYPE{
    H264_NALU_UNSPECIFIED = 0,
    // slice_layer_without_partitioning_rbsp( )
    H264_NALU_CODED_SLICE_NON_IDR = 1,
    // slice_data_partition_a_layer_rbsp( )
    H264_NALU_CODED_SLICE_DATAPARTITIONA = 2,
    // slice_data_partition_b_layer_rbsp( )
    H264_NALU_CODED_SLICE_DATAPARTITIONB = 3,
    // slice_data_partition_c_layer_rbsp( )
    H264_NALU_CODED_SLICE_DATAPARTITIONC = 4,
    // slice_layer_without_partitioning_rbsp( )
    H264_NALU_CODED_SLICE_IDR = 5,
    // Supplemental enhancement information (SEI)
    H264_NALU_SEI = 6,
    // Sequence parameter set
    H264_NALU_SPS = 7,
    // Picture parameter set
    H264_NALU_PPS = 8,
    // Access unit delimiter
    H264_NALU_AUD = 9, 
    // End of sequence
    H264_NALU_EOS = 10, 
    // End of bitstream
    H264_NALU_EOB = 11,   
    // Filler data
    H264_NALU_FD = 12,   
    // Sequence parameter set extension
    H264_NALU_SPS_EXT = 13,   
    // Prefix NAL unit
    H264_NALU_PREFIX_NAL = 14,   
    // Subset Sequence parameter set
    H264_NALU_SUBSET_SPS = 15,   
    // Depth Parameter Set
    H264_NALU_DPS = 16,   
    // 17..18    // Reserved
    // Coded slice of an auxiliary coded picture without partitioning
    H264_NALU_CODED_SLICE_AUX = 19,
    // Coded slice extension
    H264_NALU_CODED_SLICE_EXTENSION = 20,
    // Coded slice extension for a depth view component or a 3D-AVC texture view component
    H264_NALU_CODEC_SLICE_EXTENSION_3D_AVC = 21,
    // 22..23    // Reserved
    // 24..31    // Unspecified
};

/**
 * Table 7-1 - NAL unit type codes and NAL unit type classes
 * T-REC-H.265-202407-P!!PDF-E.pdf, page 83.
 */
enum H265_NAL_UNIT_TYPE{
    // Coded slice segment of a non-TSA, non-STSA trailing picture
    H265_NALU_TRAIL_N = 0,
    H265_NALU_TRAIL_R,              //1
    // Coded slice segment of a TSA picture
    H265_NALU_TSA_N,                //2
    H265_NALU_TSA_R,                //3
    // Coded slice segment of an STSA picture
    H265_NALU_STSA_N,               //4
    H265_NALU_STSA_R,               //5
    // Coded slice segment of a RADL picture
    H265_NALU_RADL_N,               //6
    H265_NALU_RADL_R,               //7
    // Coded slice segment of a RASL picture
    H265_NALU_RASL_N,               //8
    H265_NALU_RASL_R,               //9
    // Reserved non-IRAP SLNR VCL NAL unit types
    H265_NALU_RSV_VCL_N10,          //10
    // Reserved non-IRAP sub-layer reference VCL NAL unit types
    H265_NALU_RSV_VCL_R11,          //11
    // Reserved non-IRAP SLNR VCL NAL unit types
    H265_NALU_RSV_VCL_N12,          //12
    // Reserved non-IRAP sub-layer reference VCL NAL unit types
    H265_NALU_RSV_VCL_R13,          //13
    // Reserved non-IRAP SLNR VCL NAL unit types
    H265_NALU_RSV_VCL_N14,          //14
    // Reserved non-IRAP sub-layer reference VCL NAL unit types
    H265_NALU_RSV_VCL_R15,          //15
    // Coded slice segment of a BLA picture
    H265_NALU_BLA_W_LP,             //16
    H265_NALU_BLA_W_RADL,           //17
    H265_NALU_BLA_N_LP,             //18
    // Coded slice segment of an IDR picture
    H265_NALU_IDR_W_RADL,           //19
    H265_NALU_IDR_N_LP,             //20
    // Coded slice segment of a CRA picture
    H265_NALU_CRA_NUT,              //21
    // Reserved IRAP VCL NAL unit types
    H265_NALU_RSV_IRAP_VCL22,       //22
    H265_NALU_RSV_IRAP_VCL23,       //23
    // Reserved non-IRAP VCL NAL unit types
    H265_NALU_RSV_VCL24,            //24
    H265_NALU_RSV_VCL25,            //25
    H265_NALU_RSV_VCL26,            //26
    H265_NALU_RSV_VCL27,            //27
    H265_NALU_RSV_VCL28,            //28
    H265_NALU_RSV_VCL29,            //29
    H265_NALU_RSV_VCL30,            //30
    H265_NALU_RSV_VCL31,            //31
    // Video parameter set
    H265_NALU_VPS = 32,             // 32
    // Sequence parameter set
    H265_NALU_SPS = 33,             // 33
    // Picture parameter set
    H265_NALU_PPS = 34,             // 34
    // Access unit delimiter
    H265_NALU_AUD = 35,             // 35
    // End of sequence
    H265_NALU_EOS,                   // 36
    // End of bitstream
    H265_NALU_EOB,                   // 37
    // Filler data
    H265_NALU_FD,                    // 38
    // Supplemental enhancement information
    H265_NALU_PREFIX_SEI ,           // 39
    H265_NALU_SUFFIX_SEI,            // 40
    // 41...47 Reserved
    H265_NALU_RSV_NVCL41,
    H265_NALU_RSV_NVCL42,
    H265_NALU_RSV_NVCL43,
    H265_NALU_RSV_NVCL44,
    H265_NALU_RSV_NVCL45,
    H265_NALU_RSV_NVCL46,
    H265_NALU_RSV_NVCL47,
    // 48...63 Unspecified
    H265_NALU_UNSPEC48,
    H265_NALU_UNSPEC49,
    H265_NALU_UNSPEC50,
    H265_NALU_UNSPEC51,
    H265_NALU_UNSPEC52,
    H265_NALU_UNSPEC53,
    H265_NALU_UNSPEC54,
    H265_NALU_UNSPEC55,
    H265_NALU_UNSPEC56,
    H265_NALU_UNSPEC57,
    H265_NALU_UNSPEC58,
    H265_NALU_UNSPEC59,
    H265_NALU_UNSPEC60,
    H265_NALU_UNSPEC61,
    H265_NALU_UNSPEC62,
    H265_NALU_UNSPEC63,
    // Invalide
    H265_NALU_INVALID,
};

/**
 * T-REC-H.264-202408-P!!PDF-E.pdf
The order of byte stream NAL units in the byte stream shall follow the decoding order of the NAL units contained in the
byte stream NAL units (see clause 7.4.1.2). The content of each byte stream NAL unit is associated with the same access
unit as the NAL unit contained in the byte stream NAL unit (see clause 7.4.1.2.3).
leading_zero_8bits is a byte equal to 0x00.
NOTE – The leading_zero_8bits syntax element can only be present in the first byte stream NAL unit of the bitstream, because (as
shown in the syntax diagram of clause B.1.1) any bytes equal to 0x00 that follow a NAL unit syntax structure and precede the four-
byte sequence 0x00000001 (which is to be interpreted as a zero_byte followed by a start_code_prefix_one_3bytes) will be
considered to be trailing_zero_8bits syntax elements that are part of the preceding byte stream NAL unit.
zero_byte is a single byte equal to 0x00.
When any of the following conditions are true, the zero_byte syntax element shall be present:
–
the nal_unit_type within the nal_unit( ) is equal to 7 (sequence parameter set) or 8 (picture parameter set),
–
the byte stream NAL unit syntax structure contains the first NAL unit of an access unit in decoding order, as specified
in clause 7.4.1.2.3.
start_code_prefix_one_3bytes is a fixed-value sequence of 3 bytes equal to 0x000001. This syntax element is called a
start code prefix.
trailing_zero_8bits is a byte equal to 0x00.
*/
enum NALU_PREFIX_SIZE{
    NALU_INVALID_PREFIX = 0,
    // The size of a shortened NALU start sequence {0x00 0x00 0x01}, that may be used if
    // not the first NALU of an access unit or an SPS or PPS block.
    NALU_SHORT_PREFIX = 3,
    // The size of a full NALU start sequence {0x00 0x00 0x00 0x01}, used for the first NALU
    // of an access unit, and for SPS and PPS blocks.
    NALU_LONG_PREFIX = 4,
};

struct h26x_nalu{
    NALU_PREFIX_SIZE prefix;
    const uint8_t* start;
    const uint8_t* end;
};

namespace h26x{
    /**
     * some implementation of find annexb nalu start
     * faster:avx2>memmem>sbm>3byte>seq>memcmp
    */
    uint8_t* annexb_find_start_memmem(const uint8_t* bytes,size_t sizeBytes);
    uint8_t* annexb_find_start_memcmp(const uint8_t* bytes,size_t sizeBytes);
    uint8_t* annexb_find_start_seq(const uint8_t* bytes,size_t sizeBytes);
    const uint8_t* annexb_find_start_sbm(const uint8_t* bytes,size_t sizeBytes);
    uint8_t* annexb_find_start_3byte(const uint8_t* bytes,size_t sizeBytes);
#ifdef __AVX2__
    uint8_t* annexb_find_start_avx2(const uint8_t* bytes,size_t sizeBytes);
#endif
#ifdef __ARM_NEON
    uint8_t* annexb_find_start_neon(const uint8_t* bytes,size_t sizeBytes);
#endif

    /**
     * default use sbm 
    */
    const uint8_t* annexb_find_next_nalu_start(const uint8_t* bytes,size_t sizeBytes,NALU_PREFIX_SIZE* prefix);

    /**
     * find next nalu
     * founded 1,not found 0
    */
    int annexb_find_next_nalu(const uint8_t* bytes,size_t sizeBytes,h26x_nalu* nalu);
};

class h264{
public:
    static const uint8_t* annexb_skip_unsupported_nalu(const uint8_t* bytes,size_t sizeBytes);


};

class h265{
public:
    static const uint8_t* annexb_skip_unsupported_nalu(const uint8_t* bytes,size_t sizeBytes);
};

struct h266{

};

// inline void test()
// {
//     constexpr int size = sizeof(NALU_PREFIX);
// }


}//!namespace zav


#endif//!ZAV_CODEC_H26X_H_