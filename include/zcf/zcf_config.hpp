#ifndef ZCF_CONFIG_HPP_
#define ZCF_CONFIG_HPP_

#if defined(NDEBUG) && defined(DEBUG)
#error "NDEBUG and DEBUG can't defined at same time"
#endif


#if (defined(linux) || defined(__linux) || defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)) && !defined(_CRAYC)

#define ZCF_SYS_LINUX 1

#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)

#define ZCF_SYS_WINDOWS 1

#endif

#if defined(ZCF_SYS_LINUX) && defined(ZCF_SYS_WINDOWS)
#error "ZCF_SYS_LINUX and ZCF_SYS_WINDOWS can't defined at same time"
#endif

//ASSERT
#ifdef NDEBUG // release
    #define ZCF_ASSERT(x) ((void)0)
#else
    #include <assert.h>
    #define ZCF_ASSERT(x) assert(x)
#endif

#endif //!ZCF_CONFIG_HPP_