cmake_minimum_required(VERSION 3.16)

target_sources(
        ${libName}
        PRIVATE
        poll.cpp
        polldev.cpp
)