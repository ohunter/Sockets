cmake_minimum_required(VERSION 3.16)

target_sources(
        ${libName}
        PRIVATE
        socket.cpp
        tcpsocket.cpp
        udpsocket.cpp
)

# target_sources_test(
#         ${libNameTest}
#         PRIVATE
# )