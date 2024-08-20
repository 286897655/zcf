#include "zav/codec/g722_1.h"
#include "zcf/log/zcf_log.h"
#include "zcf/zcf_flags.hpp"
#include "zav/codec/g711.h"
#include "zav/av.h"
#include <iostream>

static size_t Resample_s16(const int16_t *input,size_t inputSize, 
   int16_t *output,size_t MAX_OUTPUT, 
   int inSampleRate, int outSampleRate,
   uint32_t channels){
   
   if(inSampleRate == outSampleRate && channels == 1){
      Z_ASSERT(inputSize <= MAX_OUTPUT);
      memcpy(output, input, inputSize * sizeof(int16_t));
      return inputSize;
   }

   size_t outputSize = (size_t) (inputSize * (double) outSampleRate / (double) inSampleRate);
   outputSize -= outputSize % channels;
   
   Z_ASSERT(outputSize <= MAX_OUTPUT);

   double stepDist = ((double) inSampleRate / (double) outSampleRate);
   const uint64_t fixedFraction = (1LL << 32);
   const double normFixed = (1.0 / (1LL << 32));
   uint64_t step = ((uint64_t) (stepDist * fixedFraction + 0.5));
   uint64_t curOffset = 0;
   for (uint32_t i = 0; i < outputSize; i += 1) {
      for (uint32_t c = 0; c < channels; c += 1) {
            *output++ = (int16_t) (input[c] + (input[c + channels] - input[c]) * (
                  (double) (curOffset >> 32) + ((curOffset & (fixedFraction - 1)) * normFixed)
               )
            );
      }
      curOffset += step;
      input += (curOffset >> 32) * channels;
      curOffset &= (fixedFraction - 1);
   }
   return outputSize;                  
}

int main(int argc,char** argv){
    // 输入g711a
    // 转出pcm
    // 8000采样率重采样为16000
    // 编码为g722.1
    // 解码为pcm

    zcf::logger::create_defaultLogger();
    zcf::OptionParser option_parser("g711a convert argument:");
    auto option_help = option_parser.add<zcf::Switch>("h","help","print socketproxy help");
    auto option_file = option_parser.add<zcf::Value<std::string>>("i","input","input g711a file");

    option_parser.parse(argc,argv);
    if(option_help->is_set()){
        std::cout << option_parser << std::endl;
        return 0;
    }
    if(!option_file->is_set()){
        zlog("g711a convert has no input file");
        return 0;
    }

    // 先读取所有g711
    std::string g711_file = option_file->value();
    size_t g711_size;
    int16_t* pcm_buffer;
    {
        FILE* rfile = fopen(g711_file.c_str(),"rb");
        Z_ASSERT(rfile);
        fseek(rfile, 0, SEEK_END);
        g711_size = ftell(rfile);
        fseek(rfile, 0, SEEK_SET);
        zlog("{} size {}",g711_file,g711_size);
        uint8_t* g711_buffer = new uint8_t[g711_size];
        fread(g711_buffer,1,g711_size,rfile);
        fclose(rfile);
        // convert to pcm
        // g711 uint8->pcm int16_t
        pcm_buffer = new int16_t[g711_size];
        for(int i=0;i<g711_size;i++){
            *(pcm_buffer+i)=zav::alaw2linear(*(g711_buffer+i));
        }
        FILE* wfile = fopen("g711_2_pcm.pcm","wb");
        fwrite(pcm_buffer,sizeof(int16_t),g711_size,wfile);
        fflush(wfile);
        fclose(wfile);
        delete[] g711_buffer;
    }
    
    // 8k pcm resample to 16k pcm
    int16_t* resample_pcm;
    {
        resample_pcm = new int16_t[2*g711_size];
        Resample_s16(pcm_buffer,g711_size,resample_pcm,2*g711_size,8000,16000,1);
        FILE* wfile = fopen("resample16khz.pcm","wb");
        fwrite(resample_pcm,sizeof(int16_t),2*g711_size,wfile);
        fflush(wfile);
        fclose(wfile);
        // now can delete pcm_buffer
        delete[] pcm_buffer;
    }
    
    // 转码为g722.1 
    // 每320个int16_t 转为80字节的uint8 640字节->80字节 8倍压缩
    Z_ASSERT((2*g711_size)%320 == 0);
    {
        zav::G722_1_Encoder* g7221_encoder = new zav::G722_1_Encoder(zav::G722_1_SAMPLE_RATE_16000,zav::G722_1_BIT_RATE_32000);
        FILE* wfile = fopen("pcm_2_g7221.g7221","wb");
        uint8_t g7221_buffer[80];
        for(int i=0;i < 2*g711_size ; i += 320){
            size_t encoded = g7221_encoder->Encode(resample_pcm + i,320,g7221_buffer);
            zlog("encodec g7221 {}",i,encoded);
            Z_ASSERT(encoded == 80);
            fwrite(g7221_buffer,1,80,wfile);
        }
        fflush(wfile);
        fclose(wfile);
        delete[] resample_pcm;
        delete g7221_encoder;
        zlog("pcm->g7221 end");
    }

    // 读取编码的g7221 解码为pcm
    {
        zav::G722_1_Decoder* g7221_decoder = new zav::G722_1_Decoder(
                zav::G722_1_SAMPLE_RATE_16000,
                zav::G722_1_BIT_RATE_32000,
                zav::G722_1_BITSTREAM_PACKED_LE);
        FILE* rfile = fopen("pcm_2_g7221.g7221","rb");
        FILE* wfile = fopen("g7221_2_pcm.pcm","wb");
        // 一次读取80字节 解码的话是320字节写入pcm
        uint8_t g7221_buffer[80];
        while(!feof(rfile)){
            fread(g7221_buffer,1,80,rfile);
            zav::pcm_buf pcmbuf;
            if(g7221_decoder->Decode(g7221_buffer,80,&pcmbuf) != Z_INT_SUCCESS){
                zlog("decode g7221 fail");
                return;
            }
            zlog("decode g7221 {}",pcmbuf.size);
            Z_ASSERT(pcmbuf.size == 320);
            fwrite(pcmbuf.pcm,sizeof(int16_t),pcmbuf.size,wfile);
        }
        fflush(wfile);
        fclose(wfile);
        fclose(rfile);
        delete g7221_decoder;
        zlog("conver end");
    }
}