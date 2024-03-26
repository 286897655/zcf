add_library(assert2throw STATIC assert.cpp)

target_include_directories(assert2throw PUBLIC ${ZCF_ROOT}/include/)
