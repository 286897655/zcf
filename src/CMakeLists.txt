set(ZCF_SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(ZCF_SRC_LIST ${ZCF_SRC_ROOT}/zcf_sys.cpp
                 ${ZCF_SRC_ROOT}/zcf_buffer.cpp
                 ${ZCF_SRC_ROOT}/zcf_datetime.cpp
                 ${ZCF_SRC_ROOT}/zcf_filesystem.cpp
                 ${ZCF_SRC_ROOT}/zcf_md5.cpp
                 ${ZCF_SRC_ROOT}/zcf_string.cpp
                 ${ZCF_SRC_ROOT}/zcf_utility.cpp
                 ${ZCF_SRC_ROOT}/zcf_flags.cpp
                 ${ZCF_SRC_ROOT}/log/zcf_log.cpp
                 ${ZCF_SRC_ROOT}/net/zcf_net.cpp
                 ${ZCF_SRC_ROOT}/extern/assert.cpp)

add_library(zcf STATIC ${ZCF_SRC_LIST})
target_include_directories(zcf PRIVATE ${ZCF_ROOT}/src/)
target_include_directories(zcf PUBLIC ${ZCF_ROOT}/include/)

if(ARCH_AMD64)
    target_compile_options(zcf PUBLIC -mavx2)
endif()

add_subdirectory(zav)