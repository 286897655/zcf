#ifndef ZCF_SYS_HPP_
#define ZCF_SYS_HPP_

#include <string>

namespace zcf{
/**
 * @brief some util func of system for portable use
 * 
 */
namespace sys{
    std::string getExeDir();
    std::string getExePath();
    std::string getExeName();

    /**
    * get system page size
    */
    size_t getPageSize();

    /**
     * @brief size to aligned by page size
     * 
     * @param want_size 
     * @return size_t 
     */
    size_t alignOfPageSize(size_t want_size);

    /**
     * @brief execute a command
     * 
     * @param command 
     * @return std::string 
     */
    std::string execute(const std::string& command);

    /**
     * @brief run as daemon service
     * 
     */
    void runDaemon();

    /**
     * @brief Set the Limit(core and fd limit) And Sig Crash
     * 
     */
    void setLimitAndSigCrash();

    /**
     * @brief set current thread name
     * 
     */
    void setThreadName(const std::string& name);

    void writeSyslog(const std::string& log);
};//!namespace sys

class process_mutex{
public:
    process_mutex();
    ~process_mutex();
public:
    void lock();
    void unlock();

private:
    int lock_fd;
};
}//!namespace zcf

#endif //!ZCF_SYS_HPP_