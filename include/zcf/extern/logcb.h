#ifndef ZCF_EXTERN_LOG_CALLBACK_H_
#define ZCF_EXTERN_LOG_CALLBACK_H_

#ifdef logcb
        #undef logcb
#endif//logcb

#ifdef __cplusplus
    extern "C" {
#endif
        extern void log_callback(const char* func,const char* file,int line, ...);
#ifdef __cplusplus
    }
#endif

#define logcb(...) log_callback(__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)

#endif //ZCF_EXTERN_LOG_CALLBACK_H_