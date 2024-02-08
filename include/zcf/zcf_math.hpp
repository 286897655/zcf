#ifndef ZCF_MATH_HPP_
#define ZCF_MATH_HPP_

#include <zcf/zcf_utility.hpp>
#include <type_traits>

namespace zcf{
#if __cplusplus >= 201402L // C++14 and beyond
    using std::enable_if_t;
#else
    template<bool B, class T = void>
    using enable_if_t = typename std::enable_if<B, T>::type;
#endif

// check add overflow
template<typename T,typename = enable_if_t<std::is_unsigned<T>::value>>
bool check_add(T* result, T t1, T t2){
    Z_ASSERT(result != nullptr);
    if (ZLikely(t1 < std::numeric_limits<T>::max() - t2)) {
        *result = t1 + t2;
        return true;
    } else {
        *result = {};
        return false;
    }
};

// roundup a value aligned by align
template<typename T,typename = enable_if_t<std::is_unsigned<T>::value>>
T roundUp_align(T round,T align){
    auto maxAl = align - 1;
    T realSize;
    if (!check_add<T>(&realSize, round, maxAl)) {
        assert(0);//abort
    }
    return realSize & ~maxAl;
};

}//!namespace zcf


#endif //!ZCF_MATH_HPP_