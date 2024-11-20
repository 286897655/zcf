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

#ifndef ZAV_AV_H_
#define ZAV_AV_H_

namespace zav{

enum AVCodecID{
    AV_CODEC_UNKNOWN = 0x000,
    // below audio codec
    /***pcm codec***/
    AV_CODEC_AUDIO_S16LE = 0x100,
    AV_CODEC_AUDIO_S16BE,
    AV_CODEC_AUDIO_G711_ALAW,
    AV_CODEC_AUDIO_G711_MULAW,
    /***adpcm codec***/
    AV_CODEC_AUDIO_G722 = 0x200,
    AV_CODEC_AUDIO_G722_1,
    /***other audio***/
    AV_CODEC_AUDIO_G723_1 = 0x300,
    AV_CODEC_AUDIO_G729,
    AV_CDEOC_AUDIO_AAC,
    AV_CODEC_AUDIO_OPUS,
    // below video codec
    AV_CODEC_VIDEO_H264 = 0x1000,
    AV_CODEC_VIDEO_H265,
    AV_CODEC_VIDEO_H266
};

struct pcm_buf{
    int16_t* pcm;
    size_t size;
};

struct audio_fmt{
    int sample_rate;
    int channel;
    int sample_bit;
    AVCodecID codec;
};

struct frame_buffer{
    uint8_t* buffer;
    size_t size;
};

typedef struct{
    struct audio_fmt fmt;
    struct frame_buffer frame;
}audio_frame;

/**
 * some constant strings
*/
namespace strings{
constexpr const char kGB28181[] = "gb28181";//gb28181
constexpr const char kSCHEMA_RTMP[] = "rtmp";//rtmp
constexpr const char kSCHEMA_RTMPS[] = "rtmps";//rtmps
constexpr const char kSCHEMA_RTSP[] = "rtsp";//rtsp
constexpr const char kSCHEMA_RTSPS[] = "rtsps";//rtsps
constexpr const char kSCHEMA_HTTP[] = "http";//http
constexpr const char kSCHEMA_HTTPS[] = "https";//https
};

};//!namespace zav

#endif //!ZAV_AV_H_