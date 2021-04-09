cmake_minimum_required(VERSION 3.16)
project(Sockets)

add_executable(
        udp0
        main.cpp
)

target_compile_options(udp0 PRIVATE -Wall)
target_compile_features(udp0 PRIVATE cxx_std_11)
target_link_libraries(udp0 pthread Socket)