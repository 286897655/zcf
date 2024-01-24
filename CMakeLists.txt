cmake_minimum_required(VERSION 3.18.5)

project(zcf)

include (cmake/arch.cmake)
include (cmake/buildtype.cmake)

#使能c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # C++ 11 是强制要求，不会衰退至低版本
# 编译输出详细信息
set(CMAKE_VERBOSE_MAKEFILE ON)

set(ZCF_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

#设置bin和lib库目录
set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/debug/)
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/debug/)
if(BUILD_RELEASE)
    set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/release/)
    set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/release/)
endif()
message(STATUS "BIN_DIR:${BIN_DIR},LIB_DIR:${LIB_DIR}")
SET(EXECUTABLE_OUTPUT_PATH ${BIN_DIR})
SET(LIBRARY_OUTPUT_PATH ${LIB_DIR})

add_subdirectory(src)

add_subdirectory(app)

