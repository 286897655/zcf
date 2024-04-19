#ifndef ZCF_NET_HPP_
#define ZCF_NET_HPP_

#include <memory>
#include <sys/socket.h>
#include <zcf/zcf_utility.hpp>

namespace zcf{

enum class socket_type_t{
    SOCKET_INVALID          = -1,
    SOCKET_STREAM           = 1,
    SOCKET_DATAGRAMS        = 2,
    SOCKET_RAW              = 3
};

enum class socket_end_t{
    SOCKET_END_LOCAL,
    SOCKET_END_PEER
};

namespace socket{
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
    static std::shared_ptr<socket_addr> from(int fd,socket_end_t end_type,socket_type_t type);

public:
    std::string ip() const;
    int port() const;
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