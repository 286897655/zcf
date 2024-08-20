#include "zav/codec/g722.h"

#include "zcf/log/zcf_log.h"
#include "zcf/zcf_flags.hpp"
#include <iostream>

int main(int argc,char** argv){
    const static std::string pcm_file = "pcm.dat";
    zcf::logger::create_defaultLogger();

    zcf::OptionParser option_parser("g722 convert argument:");
    auto option_help = option_parser.add<zcf::Switch>("h","help","print g722 help");
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
    std::string g722_file = option_file->value();

    FILE* rfile = fopen(g722_file.c_str(), "rb");
    Z_ASSERT(rfile);
    fseek(rfile, 0, SEEK_END);
    size_t g722_size = ftell(rfile);
    fseek(rfile, 0, SEEK_SET);
    zlog("{} size {}",g722_file,g722_size);

    // 测试数据文件是240字节一个frame
    size_t g722_frame_count = g722_size / 240;
    Z_ASSERT(g722_size % 240 == 0);
    zlog("{} has {} fram for 240 bytes per frame",g722_file,g722_frame_count);
    zav::G722Decoder* decoder = new zav::G722Decoder(zav::g722_64000_bps,zav::G722_PACKED);
    FILE* wfile = fopen(pcm_file.c_str(),"wb");
    Z_ASSERT(wfile);

    uint8_t* g722_buffer = new uint8_t[240];
    int16_t* pcm_buffer = new int16_t[4096];
    for(int i=0;i<g722_frame_count;i++){
        fread(g722_buffer,1,240,rfile);
        size_t decoded_pcm = decoder->Decode(g722_buffer,240,pcm_buffer);
        zlog("{} frame {} decoded {} pcm",g722_file,i,decoded_pcm);
        fwrite(pcm_buffer,sizeof(int16_t),decoded_pcm,wfile);
    }
    delete decoder;
    fflush(wfile);
    fclose(wfile);
    fclose(rfile);
}