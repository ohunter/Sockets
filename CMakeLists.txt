cmake_minimum_required(VERSION 3.16)
project(Sockets)

add_executable(
        Server
        src/main.cpp
        src/server.cpp
)

target_compile_options(Server PRIVATE -Wall)
target_compile_features(Server PRIVATE cxx_std_11)
target_link_libraries(Server pthread)