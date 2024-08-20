#include "zav/codec/g722_1.h"
#include "zcf/log/zcf_log.h"
#include "zcf/zcf_flags.hpp"
#include <iostream>

#ifndef NGM_RBE16
#   define NGM_RBE16(x)                            \
    (((uint16_t)((const uint8_t*)(x))[0] << 8) |   \
                ((const uint8_t*)(x))[1])
#endif
#ifndef NGM_WBE16
#   define NGM_WBE16(p, val) do {               \
        uint16_t d = (val);                     \
        ((uint8_t*)(p))[1] = (d);               \
        ((uint8_t*)(p))[0] = (d)>>8;            \
    } while(0)
#endif
#ifndef NGM_RLE16
#   define NGM_RLE16(x)                           \
    (((uint16_t)((const uint8_t*)(x))[1] << 8) |  \
                ((const uint8_t*)(x))[0])
#endif
#ifndef NGM_WLE16
#   define NGM_WLE16(p, val) do {               \
        uint16_t d = (val);                     \
        ((uint8_t*)(p))[0] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
    } while(0)
#endif

int main(int argc,char** argv){
    const static std::string pcm_file = "pcm.dat";
    zcf::logger::create_defaultLogger();
    zcf::OptionParser option_parser("g7221 convert argument:");
    auto option_help = option_parser.add<zcf::Switch>("h","help","print g7221 help");
    auto option_file = option_parser.add<zcf::Value<std::string>>("i","input","input ");

    option_parser.parse(argc,argv);
    if(option_help->is_set()){
        std::cout << option_parser << std::endl;
        return 0;
    }
    if(!option_file->is_set()){
        zlog("g7221 convert has no input file");
        return 0;
    }
    std::string g7221_file = option_file->value();

    FILE* rfile = fopen(g7221_file.c_str(), "rb");
    Z_ASSERT(rfile);
    fseek(rfile, 0, SEEK_END);
    size_t g7221_size = ftell(rfile);
    fseek(rfile, 0, SEEK_SET);
    zlog("{} size {}",g7221_file,g7221_size);

    // g7221 16k 32kbps 按照80字节一次去解码 20ms的数据
    static constexpr size_t G7221_20MS_BUFFER = 80;
    size_t g7221_frame_count = g7221_size / G7221_20MS_BUFFER;
    Z_ASSERT(g7221_size % G7221_20MS_BUFFER == 0);
    zlog("{} has {} fram for 80 bytes per frame",g7221_file,g7221_frame_count);
    zav::G722_1_Decoder* decoder = new zav::G722_1_Decoder(zav::G722_1_SAMPLE_RATE_16000,zav::G722_1_BIT_RATE_32000,zav::G722_1_BITSTREAM_PACKED_LE);
    FILE* wfile = fopen(pcm_file.c_str(),"wb");
    Z_ASSERT(wfile);
    
    uint8_t* g7221_buffer = new uint8_t[G7221_20MS_BUFFER];
    for(int i=0;i<g7221_frame_count;i++){
        fread(g7221_buffer,1,G7221_20MS_BUFFER,rfile);
        zav::pcm_buf decoded_pcm;
        if(decoder->Decode(g7221_buffer,G7221_20MS_BUFFER,&decoded_pcm) != Z_INT_SUCCESS){
            throw std::runtime_error("decoded error");
        }
        zlog("{} frame {} decoded {} pcm",g7221_file,i,decoded_pcm.size);
        fwrite(decoded_pcm.pcm,sizeof(int16_t),decoded_pcm.size,wfile);
    }
    delete decoder;
    fflush(wfile);
    fclose(wfile);
    fclose(rfile);
}