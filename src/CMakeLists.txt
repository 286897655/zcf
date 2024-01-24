INCLUDE_DIRECTORIES(${ZCF_ROOT}/src/)
INCLUDE_DIRECTORIES(${ZCF_ROOT}/include/)

set(ZCF_SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(ZCF_SRC_LIST ${ZCF_SRC_ROOT}/zcf_sys.cpp
                 ${ZCF_SRC_ROOT}/log/zcf_log.cpp
                 ${ZCF_SRC_ROOT}/net/zcf_net.cpp)

add_library(zcf STATIC ${ZCF_SRC_LIST})