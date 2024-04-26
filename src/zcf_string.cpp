#include "zcf/zcf_string.hpp"
#include <iomanip>

namespace zcf{
std::string hexDump(const uint8_t* data,size_t size,size_t max_size){
    Z_ASSERT(data);
    size = std::min(size,max_size);

    constexpr static size_t PRINT_NUM = 16;

    std::ostringstream oss;
    {
        // title
        oss << "OFFSET_H" << " ";
        for(size_t i = 0;i < PRINT_NUM;i++){
            oss << std::hex << std::setw(2) << std::setiosflags(std::ios::uppercase) << i << " ";
        }
        oss << "|" << " " << "ASCII" << std::endl;
    }

    {
        // data
        for(size_t i = 0;i < size;i += PRINT_NUM){
            // head of pos
            oss << std::hex << std::setfill('0') << std::setw(8) << std::setiosflags(std::ios::uppercase) << i << " ";
            // print data
            {
                for(size_t j = 0;j< PRINT_NUM;j++){
                    if(i + j < size){
                        oss << std::hex << std::setfill('0') << std::setw(2) << std::setiosflags(std::ios::uppercase) << static_cast<int>(data[i + j]);
                    }else{
                        oss << "  ";
                    }
                    oss << " ";
                }
            }                
            // print ascii
            oss << "|" << " ";
            for(size_t j = 0;j < PRINT_NUM;j++){
                if(i + j < size){
                    if(std::isprint(data[i+j])){
                        oss << static_cast<char>(data[i+j]);
                    }else{
                        oss << ".";
                    }
                }
            }
            oss << std::endl;
        }
    }

    return oss.str();
}
};//!namespace zcf