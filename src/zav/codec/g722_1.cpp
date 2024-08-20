#include "zav/codec/g722_1.h"
#include "g722_1/g722_1.h"
#include <memory>
#include <zcf/log/zcf_log.h>
#include <zcf/zcf_buffer.hpp>

namespace zav{

struct g722_1_decoder : public g722_1_decode_state_t{};
struct g722_1_encoder : public g722_1_encode_state_t{};

G722_1_Decoder::G722_1_Decoder(G722_1_SupportSampleRate samplerate,G722_1_BitRateMode bitrate,G722_1_BitStream_PackMode packmode):amp_buf_(nullptr){
    decoder_ = (g722_1_decoder*)calloc(1,sizeof(g722_1_decoder));
    Reset(samplerate,bitrate,packmode);
}

G722_1_Decoder::~G722_1_Decoder(){
    free(decoder_);
    decoder_ = NULL;
    delete[] amp_buf_;
    amp_buf_ = nullptr;
}

void G722_1_Decoder::Reset(G722_1_SupportSampleRate samplerate,G722_1_BitRateMode bitrate,G722_1_BitStream_PackMode packmode){
    g722_1_decode_init(decoder_,bitrate,samplerate);
    pack_mode_ = packmode;
    if(amp_buf_)
    {
        delete[] amp_buf_;
    }
    amp_frame_len_ = samplerate == G722_1_SAMPLE_RATE_16000 ? 320 : 640;
    amp_buf_ = new int16_t[MAX_G722_1_FRAME * amp_frame_len_];
    g7221_frame_len_ = bitrate / 50 / 8;
    zlog("G722_1_Decoder:packmode {} , samplerate {} , bitrate {} , frame_size {} ,g7221_frame_len {}",
        pack_mode_,samplerate,bitrate,amp_frame_len_,g7221_frame_len_);
}

size_t G722_1_Decoder::Decode(const uint8_t* g722_1_data,size_t len,const int16_t* amp){
    return g722_1_decode(decoder_,const_cast<int16_t*>(amp),g722_1_data,len);
}

int G722_1_Decoder::Decode(const uint8_t* g722_1_data,size_t len,pcm_buf* pcmbuf){
    if((len % g7221_frame_len_) != 0){
        zlog("G722_1_Decoder input g7221 data with size {} not aligend with {}",len,g7221_frame_len_);
        return Z_INT_FAIL;
    }
    if(len > (MAX_G722_1_FRAME * g7221_frame_len_)){
        zlog("G722_1_Decoder input g7221 data more than max,len:{}",len);
        return Z_INT_FAIL;
    }
    pcmbuf->pcm = amp_buf_;
    pcmbuf->size = 0;
    int decode_count = len / g7221_frame_len_;
    for(int i = 0 ; i < decode_count ; i++){
        if(pack_mode_ == G722_1_BITSTREAM_PACKED_LE){
            // TODO zhaoj do simd shuffle in future
            for(int j = 0; j < g7221_frame_len_ /2 ; j++){
                int16_t ad_pcm = Z_RLE16(g722_1_data + i * g7221_frame_len_ + 2*j);
                Z_WBE16(g722_1_data + i * g7221_frame_len_ + 2*j,ad_pcm);
            }
        }
        pcmbuf->size += g722_1_decode(decoder_,amp_buf_ + i*amp_frame_len_,g722_1_data + i * g7221_frame_len_,g7221_frame_len_);
    }
    return Z_INT_SUCCESS;
}

G722_1_Encoder::G722_1_Encoder(G722_1_SupportSampleRate samplerate,G722_1_BitRateMode bitrate){
    encoder_ = (g722_1_encoder*)calloc(1,sizeof(g722_1_encoder));
    Reset(samplerate,bitrate);
}
G722_1_Encoder::~G722_1_Encoder(){
    free(encoder_);
    encoder_ = NULL;
}

void G722_1_Encoder::Reset(G722_1_SupportSampleRate samplerate,G722_1_BitRateMode bitrate){
    g722_1_encode_init(encoder_,bitrate,samplerate);
}

size_t G722_1_Encoder::Encode(const int16_t* amp,size_t len,const uint8_t* g722_1_data){
    return g722_1_encode(encoder_,const_cast<uint8_t*>(g722_1_data),amp,len);
}

};//!namespace zav