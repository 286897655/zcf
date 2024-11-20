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

#include "zcf/zcf_sys.hpp"
#include "zcf/zcf_config.hpp"
#include <zlog/log.h>
#include "zcf/strings.hpp"
#include "zcf/zcf_datetime.hpp"
#include <sstream>
#include <iostream>
#if defined(ZCF_SYS_WINDOWS)
#include <windows.h>
#define PATH_MAX MAX_PATH
#elif defined(__MACH__) || defined(__APPLE__)
#error "not support now"
#elif defined(ZCF_SYS_LINUX)
#include <limits.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/syslog.h>
#include <execinfo.h>
#endif

namespace zcf{

FILE* sys_popen(const std::string& command, const std::string& mode)
{
#ifdef ZCF_SYS_WINDOWS
    return ::_popen(command.c_str(), mode.c_str());
#else
    return ::popen(command.c_str(), mode.c_str());
#endif
}
int sys_pclose(FILE* stream) 
{
#ifdef ZCF_SYS_WINDOWS
    return ::_pclose(stream);
#else
    return ::pclose(stream);
#endif
}

    namespace sys{

    std::string getExePath(){
        char buffer[PATH_MAX * 2 + 1] = {0};
        int n = -1;
    #if defined(ZCF_SYS_WINDOWS)
        n = GetModuleFileNameA(NULL, buffer, sizeof(buffer));
    #elif defined(__MACH__) || defined(__APPLE__)
        n = sizeof(buffer);
        if (uv_exepath(buffer, &n) != 0) {
            n = -1;
        }
    #elif defined(ZCF_SYS_LINUX)
        n = readlink("/proc/self/exe", buffer, sizeof(buffer));
    #endif

        std::string filePath;
        if (n <= 0) {
            filePath = "./";
        } else {
            filePath = buffer;
        }

    #if defined(ZCF_SYS_WINDOWS)
        //windows->unix
        for (auto &ch : filePath) {
            if (ch == '\\') {
                ch = '/';
            }
        }
    #endif //defined(_WIN32)

        return filePath;
    }

    std::string getExeDir(){
        std::string path = getExePath();
        return path.substr(0, path.rfind('/') + 1);
    }

    std::string getExeName(){
        std::string path = getExePath();
        return path.substr(path.rfind('/') + 1);
    }

    size_t getPageSize(){
        static size_t syspage = 0;
        if (syspage)
            return syspage;
        else {
        #ifdef ZCF_SYS_WINDOWS
            SYSTEM_INFO si;
            GetSystemInfo(&si);
            syspage = si.dwPageSize;
        #else
            syspage = getpagesize();
        #endif 
        }

        return syspage;
    }

    size_t alignOfPageSize(size_t want_size){
        size_t PAGE_SIZE = getPageSize();
        Z_ASSERT(PAGE_SIZE);

        size_t increments = want_size / PAGE_SIZE + (want_size % PAGE_SIZE > 0);

        return increments * PAGE_SIZE;
    }

    std::string execute(const std::string& command){
        FILE* fPipe = sys_popen(command, "r");
        if(!fPipe){
            return "";
        }
        std::string ret;
        char buff[1024] = {0};
        while(fgets(buff, sizeof(buff) - 1, fPipe)){
            ret.append(buff);
        }
        sys_pclose(fPipe);
        return ret;
    }
#ifdef ZCF_SYS_LINUX
    void runDaemon(){
        // Fork the process and have the parent exit. If the process was started
        // from a shell, this returns control to the user. Forking a new process is
        // also a prerequisite for the subsequent call to setsid().
        if(pid_t pid = fork()){
            if(pid > 0){
                // grandpa
                // We're in the grandpa process and need to exit.
                int status;
                waitpid(pid,&status,0);
                zlog("grandpa process exit.");
                exit(EXIT_SUCCESS);
            }

            if(pid < 0){
                // < 0 error fork
                zlog_error("First fork failed");
                exit(EXIT_FAILURE);
            }
        }
        zlog("father process running....");
        // pid = 0 new process
        // Make the process a new session leader. This detaches it from the
        // terminal.
        setsid();
        // A process inherits its working directory from its parent. This could be
        // on a mounted filesystem, which means that the running daemon would
        // prevent this filesystem from being unmounted. Changing to the root
        // directory avoids this problem.
        chdir("/");

        // The file mode creation mask is also inherited from the parent process.
        // We don't want to restrict the permissions on files created by the
        // daemon, so the mask is cleared.
        umask(0);

        // A second fork ensures the process cannot acquire a controlling terminal.
        if (pid_t pid = fork()){
            if (pid > 0){
                // We're in the parent process and need to exit.
                zlog("father process exit,son(daemon) process running {}",pid);
                exit(0);
            }else{
                zlog_error("Second fork failed");
                exit(1);
            }
        }
        zlog("son(daemon) process running...");
        // Close the standard streams. This decouples the daemon from the terminal
        // that started it.
        // attach stdin, stdout, stderr to /dev/null
	    // instead of just closing them. This avoids
	    // issues with third party/legacy code writing
	    // stuff to stdout/stderr.
        FILE* fin  = freopen("/dev/null", "r+", stdin);
        FILE* fout = freopen("/dev/null", "r+", stdout);
        FILE* ferr = freopen("/dev/null", "r+", stderr);
        Z_UNUSED(fin);
        Z_UNUSED(fout);
        Z_UNUSED(ferr);
        // close(0);
        // close(1);
        // close(2);

        // now chroot to running dir
        std::string running_dir = getExeDir();
        zlog("process running in:{}",running_dir);
        chdir(running_dir.c_str());
    }
#endif
    void setThreadName(const std::string& name){
        std::string set_name = name;

        if(set_name.size() > 16){
            set_name = set_name.substr(0,16);
        }
    #ifdef ZCF_SYS_LINUX
        pthread_setname_np(pthread_self(), set_name.c_str());
    #else
        zlog_warn("setThreadName not support un_linux");
    #endif
    }

    void writeSyslog(const std::string& log){
        #if !defined(ANDROID) && !defined(_WIN32)
        std::string exeName = getExeName();
        ::openlog(exeName.c_str(), LOG_PID | LOG_CONS, LOG_USER);
        ::syslog(LOG_ERR | LOG_LOCAL0,"%s",log.c_str());
        ::closelog();
        #endif
    }

    static constexpr int MAX_STACK_FRAMES = 128;
    static void sig_crash(int sig) {
    #ifdef ZCF_SYS_LINUX
        signal(sig, SIG_DFL);
        void *array[MAX_STACK_FRAMES];
        int size = backtrace(array, MAX_STACK_FRAMES);
        char ** strings = backtrace_symbols(array, size);
        std::vector<std::vector<std::string> > stack(size);
        for (int i = 0; i < size; ++i) {
            auto &ref = stack[i];
            std::string symbol(strings[i]);
            ref.emplace_back(symbol);
            static auto addr2line = [](const std::string &address) {
                std::string exePath = getExePath();
                std::string command = strings::format("addr2line -C -f -e %s %s",exePath.c_str(),address.c_str());

                return execute(command);
            };
            size_t pos1 = symbol.find_first_of("[");
            size_t pos2 = symbol.find_last_of("]");
            std::string address = symbol.substr(pos1 + 1, pos2 - pos1 - 1);
            ref.emplace_back(addr2line(address));
    
        }
        free(strings);

        std::stringstream ss;
        ss << "## crash date:" << getTimeStr("%Y-%m-%d %H:%M:%S") << std::endl;
        ss << "## exe:       " << sys::getExeName() << std::endl;
        ss << "## signal:    " << sig << std::endl;
        ss << "## stack:     " << std::endl;
        for (size_t i = 0; i < stack.size(); ++i) {
            ss << "[" << i << "]: ";
            for (auto &str : stack[i]){
                ss << str << std::endl;
            }
        }
        std::string stack_info = ss.str();
        // write to log
        zlog(stack_info);
        // write to cerr
        std::cerr << stack_info << std::endl;
        // write to syslog
        writeSyslog(stack_info);
#endif//ZCF_SYS_LINUX
    }

    void setLimitAndSigCrash(){
        zlog("setup system related settings");
    #ifdef ZCF_SYS_LINUX
        // core windows unhandled exception,linux core limit
        struct rlimit rlim,rlim_new;
        if (getrlimit(RLIMIT_CORE, &rlim)==0) {
            rlim_new.rlim_cur = rlim_new.rlim_max = RLIM_INFINITY;
            if (setrlimit(RLIMIT_CORE, &rlim_new)!=0) {
                rlim_new.rlim_cur = rlim_new.rlim_max = rlim.rlim_max;
                setrlimit(RLIMIT_CORE, &rlim_new);
            }
            zlog("core limit set to:{}",rlim_new.rlim_cur);
        }

        if (getrlimit(RLIMIT_NOFILE, &rlim)==0) {
            rlim_new.rlim_cur = rlim_new.rlim_max = 65535;
            if (setrlimit(RLIMIT_NOFILE, &rlim_new)!=0) {
                rlim_new.rlim_cur = rlim_new.rlim_max = rlim.rlim_max;
                setrlimit(RLIMIT_NOFILE, &rlim_new);
            }
            zlog("open fd limit set to:{}",rlim_new.rlim_cur);
        }

        // set signal 11 and 6
        signal(SIGSEGV, sig_crash);
        signal(SIGABRT, sig_crash);
        zlog("signal {},{} has add to crash callback.",SIGABRT,SIGSEGV);
    #endif//ZCF_SYS_LINUX
    }
};//!namespace sys

process_mutex::process_mutex():lock_fd(-1){
    
}

process_mutex::~process_mutex(){
    unlock();
}

void process_mutex::lock(){
#ifdef ZCF_SYS_LINUX
    // open and write lock file
    std::string lock_file = sys::getExeDir() + "lock.pid";
    pid_t now_pid = ::getpid();
    lock_fd = ::open(lock_file.c_str(),O_WRONLY | O_CREAT,S_IRUSR | S_IWUSR |  S_IRGRP | S_IROTH/*-rw-r--r--644*/);
    if(lock_fd < 0){
        zlog_error("open lock file {} error,procees {} can't lock,exit!!!",lock_file,now_pid);
        ::exit(0);
    }
    // lock file
    // require write lock
    struct flock lock;
    
    lock.l_type = F_WRLCK; // F_RDLCK, F_WRLCK, F_UNLCK
    lock.l_start = 0; // type offset, relative to l_whence
    lock.l_whence = SEEK_SET;  // SEEK_SET, SEEK_CUR, SEEK_END
    lock.l_len = 0;

    if (::fcntl(lock_fd, F_SETLK, &lock) == -1) {
        ::close(lock_fd);
        zlog_error("process {} lock pid fail,can't run this",now_pid);
        ::exit(0);
    }

    // truncate file
    if(::ftruncate(lock_fd,0) != 0){
        ::close(lock_fd);
        zlog_error("process {} truncate lock pid fail,do not run with lock mode.",now_pid);
        ::exit(0);
    }

    // write pid
    std::string str_pid = std::to_string(now_pid);
    ::write(lock_fd,str_pid.c_str(),str_pid.size());
#endif
}

void process_mutex::unlock(){
#ifdef ZCF_SYS_LINUX
    if(lock_fd > 0){
        ::close(lock_fd);
        pid_t now_pid = ::getpid();
        zlog("process {} unlock lock pid.",now_pid);
    }
    lock_fd = -1;
#endif
}

};//!namespace zcf
