#include "zcf/zcf_datetime.hpp"

#include <ctime>

namespace zcf{
std::string getTimeStr(const char* fmt, time_t time){
    std::tm tm_snapshot;
    if (!time) {
        time = ::time(NULL);
    }
#if defined(ZCF_SYS_WINDOWS)
    localtime_s(&tm_snapshot, &time); // thread-safe
#else
    localtime_r(&time, &tm_snapshot); // POSIX
#endif
    char buffer[1024];
    auto success = ::strftime(buffer, sizeof(buffer), fmt, &tm_snapshot);
    if (0 == success)
        return std::string(fmt);
    return buffer;
}

};//!namespace zcf