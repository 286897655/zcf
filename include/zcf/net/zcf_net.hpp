/** 
 * @copyright Copyright © 2020-2024 code by zhaoj
 * 
 * LICENSE
 * 
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

 /**
 * @author zhaoj 286897655@qq.com
 * @brief 
 */

#ifndef ZCF_NET_HPP_
#define ZCF_NET_HPP_

#include <string>
#include <zcf/zcf_utility.hpp>
#if defined(ZCF_SYS_WINDOWS)
#include <WinSock2.h>
#elif defined(ZCF_SYS_LINUX)
#include <sys/socket.h>
#endif

namespace zcf{

enum class socket_type_t{
    SOCKET_INVALID          = -1,// INVALID
    SOCKET_UNKNOWN          = 0, // UNKNOWN
    SOCKET_STREAM           = 1, // TCP
    SOCKET_DATAGRAMS        = 2, // UDP
    SOCKET_RAW              = 3  
};

enum class socket_end_t{
    SOCKET_END_LOCAL,
    SOCKET_END_PEER
};

namespace socket{
static constexpr const char* kSOCKET_TYPE_STRING[4] = {"UNKNOWN","TCP","UDP","RAW"};

inline std::string string_of_type(socket_type_t type){ return std::string(kSOCKET_TYPE_STRING[static_cast<int>(type)]); }
/**
 * upper case of "UNKNOWN TCP UDP RAW"
*/
socket_type_t type_of_str(const std::string& str_type);

    int cmp_sockaddr(const struct sockaddr* first,const struct sockaddr* second);
    /**
     * @brief compare socaddr storage
     * Attention:only compare ip and family
     * 
     * @param first 
     * @param second 
     * @return int 
     */
    int cmp_sockaddr(const struct sockaddr_storage* first,const struct sockaddr_storage* second);

    std::string retrieve_ip(const struct sockaddr *addr);

    int retrieve_port(const struct sockaddr *addr);

    bool is_ip(const std::string& ip);

    std::pair<std::string,int> parse_ip_colon_port(const std::string& ip_colon_port);
};

class socket_addr : public std::enable_shared_from_this<socket_addr>{
public:
    static std::shared_ptr<socket_addr> from(const struct sockaddr* addr,socket_type_t type);
    static std::unique_ptr<socket_addr> from(int fd,socket_end_t end_type,socket_type_t type);
public:
    std::string ip() const;
    int port() const;
    std::string format() const;
    socket_type_t socket_type() const;
    struct sockaddr* addr() const;

    /**
     * @brief compare socket type,port,ip/family
     * 
     * @param that 
     * @return true 
     * @return false 
     */
    bool operator==(const socket_addr& that);
    bool operator!=(const socket_addr& that);

public:
    socket_addr()=default;
    ~socket_addr()=default;

private:
    ZCF_DISABLE_COPY_MOVE(socket_addr);
private:
    std::unique_ptr<struct sockaddr_storage> addr_storage_;
    socket_type_t socket_type_;
};

}//!namespace zcf

#endif //!ZCF_NET_HPP_