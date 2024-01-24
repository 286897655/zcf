#ifndef ZCF_DATETIME_HPP_
#define ZCF_DATETIME_HPP_

#include <string>

namespace zcf{

/**
 * format time string
 * @param fmt timeformat，eg %Y-%m-%d %H:%M:%S
 * @return format time string
 */
std::string getTimeStr(const char* fmt, time_t time = 0);

}//!namespace zcf

#endif //!ZCF_DATETIME_HPP_