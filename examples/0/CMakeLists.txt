cmake_minimum_required(VERSION 3.16)
project(Sockets)

add_executable(
        Example0
        main.cpp
)

target_compile_options(Example0 PRIVATE -Wall)
target_compile_features(Example0 PRIVATE cxx_std_11)
target_link_libraries(Example0 pthread Socket)