cmake_minimum_required(VERSION 3.16)
project(Sockets)

add_executable(
        tcp0
        main.cpp
)

target_compile_options(tcp0 PRIVATE -Wall)
target_compile_features(tcp0 PRIVATE cxx_std_11)
target_link_libraries(tcp0 pthread Socket)