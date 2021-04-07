#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include <Sockets/socket.hpp>

#include "../tls.hpp"

void server(std::string address, uint16_t port) {
    SSL_CTX *ctx = setup_server_ctx();

    size_t n;
    char   buf[256] = {0};

    try {
        auto sock = Sockets::TLSSocket::Service(address, port, ctx,
                                                Sockets::Domain::IPv4);

        std::cout << "Waiting\n";

        // Accept the incoming connection
        auto connection = sock.accept(ctx);

        // Read the size of the incoming message
        connection.recv(buf, 1);
        n = (uint8_t)std::atoi(buf);
        std::cout << "Expecting " << n << " bytes" << std::endl;
        connection.recv(buf, n);

        std::cout << "Server recieved " << n
                  << " bytes containing: " << std::string(buf) << std::endl;

        char b[] = {(char)n, 0};
        connection.send(b, 1);
        connection.send(buf, n);

        // Close the sockets
        connection.close();
        sock.close();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        ERR_print_errors_fp(stderr);
    }

    SSL_CTX_free(ctx);
}

int main(int argc, char *argv[]) {
    // Initialize OpenSSL
    setup_openssl();

    // Start server
    std::thread t0(server, "127.0.0.1", 12345);

    // Sleep for 1 second to let the server get to the `accept` call
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Initialize the SSL context
    SSL_CTX *ctx = setup_client_ctx();

    char        buf[256] = {0};
    std::string s        = "";

    try {
        // Connect to the server
        auto sock = Sockets::TLSSocket::Connect("127.0.0.1", 12345, ctx,
                                                Sockets::Domain::IPv4);

        std::cout << "Enter the message: \n";

        std::getline(std::cin, s);

        std::cout << "Sending a message of " << s.size() << " bytes\n";

        sock.send(std::to_string(s.size()).c_str(), 1);
        sock.send(s.c_str(), s.size());

        sock.recv(buf, 1);
        sock.recv(buf, (int)buf[0]);

        std::cout << "Client recieved " << std::strlen(buf)
                  << " bytes containing: " << std::string(buf) << std::endl;

        // Close the connection
        sock.close();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        ERR_print_errors_fp(stderr);
    }

    SSL_CTX_free(ctx);

    t0.join();

    // TODO: check if there are other things to clean up

    cleanup_openssl();
}