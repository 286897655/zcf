add_executable(g7221 g7221.cpp)
target_link_libraries(g7221 zav pthread)

add_executable(g711 g711.cpp)
target_link_libraries(g711 zav pthread)

add_executable(g722 g722.cpp)
target_link_libraries(g722 zav pthread)

add_executable(audio_convert audio_convert.cpp)
target_link_libraries(audio_convert zav pthread)