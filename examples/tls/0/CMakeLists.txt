cmake_minimum_required(VERSION 3.16)
project(Sockets)

add_executable(
        tls0
        main.cpp
)

target_compile_options(tls0 PRIVATE -Wall)
target_compile_features(tls0 PRIVATE cxx_std_11)
target_link_libraries(tls0 pthread Socket)