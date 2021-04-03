#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <Sockets/socket.hpp>

void server(std::string address, uint16_t port) {
    SSL_CTX *ctx = SSL_CTX_new(SSLv23_server_method());

    size_t n;
    char buf[256] = {0};

    auto sock =
        Sockets::TLSSocket::Service(address, port, ctx, Sockets::Domain::IPv4);

    // Accept the incoming connection
    auto connection = sock.accept(ctx);

    // Read the size of the incoming message
    connection.recv(buf, 1);
    n = (uint8_t)buf[0];
    connection.recv(buf, n);

    std::cout << "Server recieved " << n
              << " bytes containing: " << std::string(buf) << std::endl;

    char b[] = {(char)n, 0};
    connection.send(b, 1);
    connection.send(buf, n);

    // Close the connection
    connection.close();
    sock.close();

    SSL_CTX_free(ctx);
}

int main(int argc, char *argv[]) {
    // Initialize OpenSSL
    SSL_library_init();
    SSL_load_error_strings();

    // Start server
    std::thread t0(server, "127.0.0.1", 12345);

    // Sleep for 1 second to let the server get to the `accept` call
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Initialize the SSL context
    SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());

    // Connect to the server
    auto sock =
        Sockets::TLSSocket::Connect("127.0.0.1", 12345, ctx, Sockets::Domain::IPv4);

    char buf[256] = {0};
    std::string s = "";

    std::getline(std::cin, s);
    char b[] = {(char)s.size(), 0};
    sock.send(b, 1);
    sock.send(s.c_str(), s.size());

    sock.recv(b, 1);
    sock.recv(buf, (int)b[0]);

    std::cout << "Client recieved " << (int)b[0]
              << " bytes containing: " << std::string(buf) << std::endl;

    // Close the connection
    sock.close();

    t0.join();

    SSL_CTX_free(ctx);
    // TODO: check if there are other things to clean up
}