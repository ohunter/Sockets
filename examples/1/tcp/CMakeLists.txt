cmake_minimum_required(VERSION 3.16)
project(Sockets)

add_executable(
        tcp1
        main.cpp
)

target_compile_options(tcp1 PRIVATE -Wall)
target_compile_features(tcp1 PRIVATE cxx_std_11)
target_link_libraries(tcp1 pthread Socket)