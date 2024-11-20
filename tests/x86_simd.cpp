#include <zcf/zcf_buffer.hpp>
#include <zcf/utility.hpp>
#include <zlog/log.h>

#include <immintrin.h>

int main(int argc,char** argv){
    zlog::logger::create_defaultLogger();

    uint8_t origin[16] = {0x00,0x01,0x02,0x03,
                                0x04,0x05,0x06,0x07,
                                0x08,0x09,0x0A,0x0B,
                                0x0C,0x0D,0x0E,0x0F};
    
    zlog("dump origin:\n{}",zcf::utility::hexdump_format(origin,sizeof(origin)));

    const __m128i ShuffleRev = _mm_setr_epi8(
			1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14
		);
    // Load 16 bytes at once into one 16-byte register
    __m128i load = _mm_loadu_si128(
        reinterpret_cast<__m128i*>(origin)
    );
    __m128i cross = _mm_shuffle_epi8(load,ShuffleRev);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(origin),cross);
    zlog("dump shuffle:\n{}",zcf::utility::hexdump_format(origin,sizeof(origin)));
}