#include <zcf/zcf_buffer.hpp>
#include <zcf/zcf_string.hpp>
#include <zcf/log/zcf_log.h>

#include <arm_neon.h>

int main(int argc,char** argv){
    zcf::logger::create_defaultLogger();

    uint8_t origin[16] = {0x00,0x01,0x02,0x03,
                                0x04,0x05,0x06,0x07,
                                0x08,0x09,0x0A,0x0B,
                                0x0C,0x0D,0x0E,0x0F};
    
    zlog("dump origin:\n{}",zcf::hexDump(origin,sizeof(origin)));

    // test vtbl
    static constexpr uint8_t data[16] = {1,  0,  3,  2,
                                             5,  4,  7,  6,
                                             9,  8, 11, 10,
                                            13, 12, 15, 14};
    // load in little endian
    const static uint8x16_t ShuffleRev = vld1q_u8(data);
    // Load 16 bytes at once into one 16-byte register
    uint8x16_t load = vld1q_u8(origin);
    uint8x16_t cross = vqtbl1q_u8(load,ShuffleRev);
    vst1q_u8(origin,cross);
    zlog("dump shuffle:\n{}",zcf::hexDump(origin,sizeof(origin)));

    // test rev
    vst1q_u8(origin,vrev16q_u8(vld1q_u8(origin)));
    zlog("dump reverse:\n{}",zcf::hexDump(origin,sizeof(origin)));
}