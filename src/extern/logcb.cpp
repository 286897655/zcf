#include "zcf/extern/logcb.h"
#include "zcf/log/zcf_log.h"
extern "C" void log_callback(const char* func,const char* file,int line, ...){
    va_list vlist;
    va_start(vlist,11);
    // va_arg(vlist,1);
    va_end(vlist);
    zlog("logcb:{}-{}-{}:{}",func,file,line);
}