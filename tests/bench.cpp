#include <zcf/log/zcf_log.h>
#include <zcf/zcf_buffer.hpp>
#include <chrono>
int main(int argc,char** argv){
    zcf::logger::create_defaultLogger();

    uint8_t origin[320];
    for(int i = 0;i<320;i++){
        origin[i] = rand() & 0xFF;
    }

    // bench cross byte c
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i< 100000;i++){
        zcf::cross_byte_u8_c(origin,sizeof(origin));
    }
    auto end = std::chrono::high_resolution_clock::now();
    zlog("c cross byte time count:{} ms",std::chrono::duration_cast<std::chrono::milliseconds>(end -start).count());

    // bench cross byte simd
    start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i< 100000;i++){
        zcf::cross_byte_u8(origin,sizeof(origin));
    }
    end = std::chrono::high_resolution_clock::now();
    zlog("simd cross byte time count:{} ms",std::chrono::duration_cast<std::chrono::milliseconds>(end -start).count());
}