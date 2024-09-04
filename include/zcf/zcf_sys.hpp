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

#ifndef ZCF_SYS_HPP_
#define ZCF_SYS_HPP_

#include <string>

namespace zcf{

    /**
    * warpper of popen and pclose for independent system
    */
    FILE* sys_popen(const std::string& command, const std::string& mode);
    int sys_pclose(FILE* stream);
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

#ifdef ZCF_SYS_LINUX
    /**
     * @brief run as daemon service for linux fork 
     * 
     */
    void runDaemon();
#endif

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