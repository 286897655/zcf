#ifndef ZCF_REMAP_ASSERT_HPP_
#define ZCF_REMAP_ASSERT_HPP_

#include <stdio.h>

#ifndef NDEBUG
    #ifdef assert
        #undef assert
    #endif//assert

    #ifdef __cplusplus
    extern "C" {
    #endif
        extern void assert2throw(int failed, const char *exp, const char *func, const char *file, int line, const char *str);
    #ifdef __cplusplus
    }
    #endif

    #define assert(exp) assert2throw(!(exp), #exp, __FUNCTION__, __FILE__, __LINE__, NULL)
#else
    #define	assert(e)	((void)0)
#endif//NDEBUG

#endif//!ZCF_REMAP_ASSERT_HPP_