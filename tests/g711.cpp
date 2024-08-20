#include "zav/codec/g711.h"
#include "zav/av.h"
#include <zcf/log/zcf_log.h>
#include <zcf/zcf_flags.hpp>
#include <iostream>
using namespace zav;
int main(int argc,char** argv){
    zcf::logger::create_defaultLogger();
    zcf::OptionParser option_parser("g711 convert argument:");
    auto option_help = option_parser.add<zcf::Switch>("h","help","print socketproxy help");
    auto option_file = option_parser.add<zcf::Value<std::string>>("i","input","input ");
    auto option_g711_format = option_parser.add<zcf::Implicit<std::string>>("f","format","default format is alaw,another is mulaw","alaw");
    
    option_parser.parse(argc,argv);
    if(option_help->is_set()){
        std::cout << option_parser << std::endl;
        return 0;
    }
    if(!option_file->is_set()){
        zlog("g711 convert has no input file");
        return 0;
    }
    std::string g711_file = option_file->value();

    AVCodecID input_codec = option_g711_format->value() == "alaw" ? AV_CODEC_AUDIO_G711_ALAW : AV_CODEC_AUDIO_G711_MULAW;
    FILE* rfile = fopen(g711_file.c_str(),"rb");
    Z_ASSERT(rfile);
    fseek(rfile, 0, SEEK_END);
    size_t g711_size = ftell(rfile);
    fseek(rfile, 0, SEEK_SET);
    zlog("{} size {}",g711_file,g711_size);
    uint8_t* g711_buffer = new uint8_t[g711_size];
    fread(g711_buffer,1,g711_size,rfile);
    fclose(rfile);
    // convert to pcm
    // g711 uint8->pcm int16_t
    int16_t* pcm_buffer = new int16_t[g711_size];
    for(int i=0;i<g711_size;i++){
        *(pcm_buffer+i)=zav::alaw2linear(*(g711_buffer+i));
    }
    FILE* wfile = fopen("g711_pcm.dat","wb");
    fwrite(pcm_buffer,sizeof(int16_t),g711_size,wfile);
    fflush(wfile);
    fclose(wfile);
    delete[] pcm_buffer;
    delete[] g711_buffer;
}