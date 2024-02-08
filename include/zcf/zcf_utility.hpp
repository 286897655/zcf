#ifndef ZCF_UTILITY_HPP_
#define ZCF_UTILITY_HPP_

#include <zcf/zcf_config.hpp>

#define ZCF_DISABLE_COPY(Class) \
    Class(const Class &) = delete;\
    Class &operator=(const Class &) = delete;

#define ZCF_DISABLE_MOVE(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;

#define ZCF_DISABLE_COPY_MOVE(Class) \
    ZCF_DISABLE_COPY(Class) \
    ZCF_DISABLE_MOVE(Class)


#define Z_UNUSED(x) (void)x;

#ifndef ZLikely
    #ifdef _MSC_VER
    #define ZLikely(cond) (cond)
    #elif defined(__GNUC__)
    #define ZLikely(cond) __builtin_expect(!!(cond), 1) 
    #endif
#endif

#ifndef ZUnlikely
    #ifdef _MSC_VER
    #define ZUnlikely(cond) (cond)
    #elif defined(__GNUC__)
    #define ZUnlikely(cond) __builtin_expect(!!(cond), 0) 
    #endif
#endif

#define ZDELETE_P(p) \
    if (p) { \
        delete p; \
        p = NULL; \
    } \
    (void)0

#define ZDELETE_PA(pa) \
    if (pa) { \
        delete[] pa; \
        pa = NULL; \
    } \
    (void)0

#endif //!ZCF_UTILITY_HPP_