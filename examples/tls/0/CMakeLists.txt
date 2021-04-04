cmake_minimum_required(VERSION 3.16)
project(Sockets)

find_package(OpenSSL REQUIRED)

find_path(
        CERT_PATH
        NAMES
        "server.crt"
        "server.key"
        "client.crt"
        "client.key"
        HINTS
        "../../certificates/"
)

string(CONCAT SERVER_CERT ${CERT_PATH} "/server.crt")
string(CONCAT SERVER_KEY ${CERT_PATH} "/server.key")
string(CONCAT CLIENT_CERT ${CERT_PATH} "/client.crt")
string(CONCAT CLIENT_KEY ${CERT_PATH} "/client.key")

add_executable(
        tls0
        main.cpp
)

target_compile_options(
        tls0
        PRIVATE
        -Wall

        -DSERVER_CERT="${SERVER_CERT}"
        -DSERVER_KEY="${SERVER_KEY}"
        -DCLIENT_CERT="${CLIENT_CERT}"
        -DCLIENT_KEY="${CLIENT_KEY}"
)
target_compile_features(
        tls0
        PRIVATE
        cxx_std_11
)
target_link_libraries(
        tls0
        pthread
        Socket
        ${OPENSSL_LIBRARIES}
)