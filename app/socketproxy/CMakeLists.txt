set(ASIO_ROOT ${ZCF_ROOT}/3rdparty/asio-1.28.0)

INCLUDE_DIRECTORIES(${ASIO_ROOT}/include/)

add_executable(socketproxy socketproxy.cpp)

target_link_libraries(socketproxy pthread zcf)
