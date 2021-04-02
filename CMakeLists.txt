cmake_minimum_required(VERSION 3.16)
project(Socket LANGUAGES CXX)
set(libName Sockets)

find_package(OpenSSL REQUIRED)

option(BUILD_EXAMPLES "BUILD_EXAMPLES" OFF)

add_library(${libName} OBJECT "")

set_property(TARGET ${libName} PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(Socket SHARED $<TARGET_OBJECTS:${libName}>)
target_link_libraries(Socket PUBLIC ${libName})

file(GLOB_RECURSE HEADERS socket/**/*.hpp)

add_subdirectory(socket)

target_include_directories(
    ${libName}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    "$<INSTALL_INTERFACE:include>"
)

install(
    TARGETS Socket
    LIBRARY DESTINATION lib
)

install(
    FILES ${HEADERS}
    DESTINATION "include/Sockets"
)

target_compile_options(${libName} PRIVATE -Wall -Werror)
target_compile_features(${libName} PRIVATE cxx_std_11)
target_link_libraries(${libName} pthread ${OPENSSL_LIBRARIES})

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()