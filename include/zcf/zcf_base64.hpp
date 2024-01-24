#ifndef ZCF_BASE64_HPP_
#define ZCF_BASE64_HPP_

#include <string>
namespace zcf{
    
    std::string base64encode(const std::string& input);

    std::string base64decode(const std::string& input);
}//!namespace zcf

#endif//!ZCF_BASE64_HPP_