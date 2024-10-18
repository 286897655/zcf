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

#include "zcf/net/zcf_net.hpp"
#include "zcf/log/zcf_log.h"
#include "zcf/strings.hpp"
#include "zcf/memory.hpp"
#if defined(ZCF_SYS_WINDOWS)
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#elif defined(ZCF_SYS_LINUX)
#include <arpa/inet.h>
#endif

namespace zcf{

namespace socket{
socket_type_t type_of_str(const std::string& str_type)
{
    if(str_type == "TCP"){
        return socket_type_t::SOCKET_STREAM;
    }else if(str_type == "UDP"){
        return socket_type_t::SOCKET_DATAGRAMS;
    }else if(str_type == "RAW"){
        return socket_type_t::SOCKET_RAW;
    }
    return socket_type_t::SOCKET_UNKNOWN;
}
    int cmp_sockaddr(const struct sockaddr* first,const struct sockaddr* second){
        return cmp_sockaddr((const struct sockaddr_storage*)first,(const struct sockaddr_storage*)second);
    }

    int cmp_sockaddr(const struct sockaddr_storage* first,const struct sockaddr_storage* second){
        if (first->ss_family == AF_INET && second->ss_family == AF_INET) {
            struct sockaddr_in *sa, *sb;

            sa = (struct sockaddr_in *) first;
            sb = (struct sockaddr_in *) second;

            return sa->sin_addr.s_addr == sb->sin_addr.s_addr;
        } if (first->ss_family == AF_INET6 && second->ss_family == AF_INET6) {
            struct sockaddr_in6 *sa, *sb;

            sa = (struct sockaddr_in6 *) first;
            sb = (struct sockaddr_in6 *) second;

            return ::memcmp(sa->sin6_addr.s6_addr, sb->sin6_addr.s6_addr, sizeof(sa->sin6_addr.s6_addr)) == 0;
        }

        return 0;
    }

    static std::string inet_ntop_l(int AF,const void* addr){
        std::string ret;
        ret.resize(128);
        if (!inet_ntop(AF, addr, (char *) ret.data(), ret.size())) {
            ret.clear();
        } else {
            ret.resize(strlen(ret.data()));
        }
        return ret;
    }

    std::string retrieve_ip(const struct sockaddr *addr){
        switch (addr->sa_family)
        {
        case AF_INET:
            return inet_ntop_l(AF_INET, &(((struct sockaddr_in *)addr)->sin_addr));
            break;
        case AF_INET6:
            if (IN6_IS_ADDR_V4MAPPED(&((struct sockaddr_in6 *)addr)->sin6_addr)) {
                struct in_addr addr4;
                ::memcpy(&addr4, 12 + (char *)&(((struct sockaddr_in6 *)addr)->sin6_addr), 4);
                return inet_ntop_l(AF_INET,&addr4);
            }
            return inet_ntop_l(AF_INET6,&(((struct sockaddr_in6 *)addr)->sin6_addr));
            break;
        default:
            Z_ASSERT(0);
            break;
        }
        return "";
    }

    int retrieve_port(const struct sockaddr *addr){
        switch (addr->sa_family) {
            case AF_INET: 
                return ntohs(((struct sockaddr_in *)addr)->sin_port);
            break;
            case AF_INET6: 
                return ntohs(((struct sockaddr_in6 *)addr)->sin6_port);
            break;
            default: 
                Z_ASSERT(0);
            break;
        }
        return 0;
    }

    bool is_ip(const std::string& ip){
        throw std::runtime_error("not support now");
    }

    std::pair<std::string,int> parse_ip_colon_port(const std::string& ip_colon_port){
        std::vector<std::string> splits = strings::split(ip_colon_port,":");
        
        return std::make_pair(splits[0], strings::conv_to<int>(splits[1]));
    }
};

std::shared_ptr<socket_addr> socket_addr::from(const struct sockaddr* addr,socket_type_t type){
    if(!addr || type == socket_type_t::SOCKET_INVALID){
        return nullptr;
    }

    std::shared_ptr<socket_addr> shared_addr = std::make_shared<socket_addr>();
    shared_addr->socket_type_ = type;
    shared_addr->addr_storage_.reset(new sockaddr_storage(*(sockaddr_storage*)addr));

    return shared_addr;
}

std::unique_ptr<socket_addr> socket_addr::from(int fd,socket_end_t end_type,socket_type_t type){
    if(type == socket_type_t::SOCKET_INVALID){
        return nullptr;
    }

    std::unique_ptr<socket_addr> unique_addr = std::make_unique<socket_addr>();
    unique_addr->addr_storage_.reset(new sockaddr_storage());
    unique_addr->socket_type_ = type;
    
    socklen_t addr_len = sizeof(sockaddr_storage);
    if(end_type == socket_end_t::SOCKET_END_PEER){// get peer/remote
        if(::getpeername(fd,(struct sockaddr*)unique_addr->addr_storage_.get(),&addr_len) < 0){
            zlog("fd can't getpeername:{}",fd);
            return nullptr;
        }
    }else{// get local
        if(::getsockname(fd,(struct sockaddr*)unique_addr->addr_storage_.get(),&addr_len) < 0){
            zlog("fd can't getsockename:{}",fd);
            return nullptr;
        }
    }
    zlog("fd to ip port:{} -> {}:{}",fd,
        socket::retrieve_ip((struct sockaddr *)(unique_addr->addr_storage_.get())),
        socket::retrieve_port((struct sockaddr *)(unique_addr->addr_storage_.get())));
    

    return unique_addr;
}

std::string socket_addr::ip() const{
    return socket::retrieve_ip((struct sockaddr *)addr_storage_.get());
}

int socket_addr::port() const{
    return socket::retrieve_port((struct sockaddr *)addr_storage_.get());
}

std::string socket_addr::format() const{
    return strings::format("%s:%d",ip().c_str(),port());
}

socket_type_t socket_addr::socket_type() const{
    return socket_type_;
}

struct sockaddr* socket_addr::addr() const{
    return (struct sockaddr*)addr_storage_.get();
}

bool socket_addr::operator==(const socket_addr& that){
    // socket type
    if(socket_type_ != that.socket_type()){
        return false;
    }

    // address port
    if(port() != that.port()){
        return false;
    }
    // address family and ip
    return socket::cmp_sockaddr(addr(),that.addr());
}

bool socket_addr::operator!=(const socket_addr& that){
    // socket type
    if(socket_type_ != that.socket_type()){
        return true;
    }
    // address port
    if(port() != that.port()){
        return true;
    }
    // address family and ip
    return !socket::cmp_sockaddr(addr(),that.addr());
}   

};//!namespace zcf