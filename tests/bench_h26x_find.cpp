#include <zlog/log.h>
#include <zcf/zcf_flags.hpp>
#include <chrono>
#include <iostream>
#include "zav/codec/h26x.h"

int main(int argc,char** argv){
    zlog::logger::create_defaultLogger();

    zcf::OptionParser option_parser("bench_h26x argument:");
    auto option_help = option_parser.add<zcf::Switch>("h","help","print bench_h26x help");
    auto option_file = option_parser.add<zcf::Value<std::string>>("i","input","input ");

    option_parser.parse(argc,argv);
    if(option_help->is_set()){
        std::cout << option_parser << std::endl;
        return 0;
    }
    if(!option_file->is_set()){
        zlog("bench_h26x has no input file");
        return 0;
    }
    std::string h26x_file = option_file->value();
    FILE* rfile = fopen(h26x_file.c_str(), "rb");
    Z_ASSERT(rfile);
    fseek(rfile, 0, SEEK_END);
    size_t h26x_size = ftell(rfile);
    fseek(rfile, 0, SEEK_SET);
    zlog("{} size {}",h26x_file,h26x_size);
    uint8_t* rbufer = new uint8_t[h26x_size];
    fread(rbufer,1,h26x_size,rfile);
    fclose(rfile);
    // 5634个
    int bench_times = 1000;
    // bench memmem
    size_t start_count = 0;
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0 ;i < bench_times ;i++){
        const uint8_t* p_memmem = rbufer;
        const uint8_t* pend_memmem = rbufer + h26x_size;
        while(p_memmem < pend_memmem){
            uint8_t* found = zav::h26x::annexb_find_start_memmem(p_memmem,pend_memmem - p_memmem);
            if(found){
                ++start_count;
                p_memmem = (found += 3);
            }else{
                break;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    zlog("memmem {} times:found {},cost:{} ms",bench_times,start_count,std::chrono::duration_cast<std::chrono::milliseconds>(end -start).count());
    start_count = 0;

    start = std::chrono::high_resolution_clock::now();
    for(int i = 0 ;i < bench_times ;i++){
        const uint8_t* p_memmem = rbufer;
        const uint8_t* pend_memmem = rbufer + h26x_size;
        while(p_memmem < pend_memmem){
            uint8_t* found = zav::h26x::annexb_find_start_memcmp(p_memmem,pend_memmem - p_memmem);
            if(found){
                ++start_count;
                p_memmem = (found += 3);
            }else{
                break;
            }
        }
    }
    end = std::chrono::high_resolution_clock::now();
    zlog("memcmp {} times:found {},cost:{} ms",bench_times,start_count,std::chrono::duration_cast<std::chrono::milliseconds>(end -start).count());
    start_count = 0;

    start = std::chrono::high_resolution_clock::now();
    for(int i = 0 ;i < bench_times ;i++){
        const uint8_t* p_memmem = rbufer;
        const uint8_t* pend_memmem = rbufer + h26x_size;
        while(p_memmem < pend_memmem){
            uint8_t* found = zav::h26x::annexb_find_start_seq(p_memmem,pend_memmem - p_memmem);
            if(found){
                ++start_count;
                p_memmem = (found += 3);
            }else{
                break;
            }
        }
    }
    end = std::chrono::high_resolution_clock::now();
    zlog("seq {} times:found {},cost:{} ms",bench_times,start_count,std::chrono::duration_cast<std::chrono::milliseconds>(end -start).count());
    start_count = 0;

    start = std::chrono::high_resolution_clock::now();
    for(int i = 0 ;i < bench_times ;i++){
        const uint8_t* p_memmem = rbufer;
        const uint8_t* pend_memmem = rbufer + h26x_size;
        while(p_memmem < pend_memmem){
            uint8_t* found = zav::h26x::annexb_find_start_3byte(p_memmem,pend_memmem - p_memmem);
            if(found){
                ++start_count;
                p_memmem = (found += 3);
            }else{
                break;
            }
        }
    }
    end = std::chrono::high_resolution_clock::now();
    zlog("3byte {} times:found {},cost:{} ms",bench_times,start_count,std::chrono::duration_cast<std::chrono::milliseconds>(end -start).count());
    start_count = 0;

    start = std::chrono::high_resolution_clock::now();
    for(int i = 0 ;i < bench_times ;i++){
        const uint8_t* p_memmem = rbufer;
        const uint8_t* pend_memmem = rbufer + h26x_size;
        while(p_memmem < pend_memmem){
            uint8_t* found = zav::h26x::annexb_find_start_avx2(p_memmem,pend_memmem - p_memmem);
            if(found){
                ++start_count;
                p_memmem = (found += 3);
            }else{
                break;
            }
        }
    }
    end = std::chrono::high_resolution_clock::now();
    zlog("avx2 {} times:found {},cost:{} ms",bench_times,start_count,std::chrono::duration_cast<std::chrono::milliseconds>(end -start).count());
    start_count = 0;

    start = std::chrono::high_resolution_clock::now();
    for(int i = 0 ;i < bench_times ;i++){
        const uint8_t* p_memmem = rbufer;
        const uint8_t* pend_memmem = rbufer + h26x_size;
        while(p_memmem < pend_memmem){
            const uint8_t* found = zav::h26x::annexb_find_start_sbm(p_memmem,pend_memmem - p_memmem);
            if(found){
                ++start_count;
                p_memmem = (found += 3);
            }else{
                break;
            }
        }
    }
    end = std::chrono::high_resolution_clock::now();
    zlog("sbm {} times:found {},cost:{} ms",bench_times,start_count,std::chrono::duration_cast<std::chrono::milliseconds>(end -start).count());
    delete[] rbufer;
    return 0;
}