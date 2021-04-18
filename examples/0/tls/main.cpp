#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include <Sockets/socket.hpp>

#include "../../utility/headers/tls.hpp"

void server(std::string address, uint16_t port) {
    SSL_CTX *ctx = setup_server_ctx();

    size_t n;
    char   buf[256] = {0};

    try {

        std::shared_ptr<Sockets::TLSSocket> sock =
            Sockets::TLSSocket::service(address, port, Sockets::Domain::IPv4, ctx);

        std::cout << "Waiting\n";

        // Accept the incoming connection
        std::shared_ptr<Sockets::TLSSocket> connection = sock->accept(ctx);

        // Read the size of the incoming message
        connection->recv(buf, 1);
        n = std::atoi(buf);
        connection->recv(buf, n);

        std::cout << "Server recieved " << n << " bytes containing: " << std::string(buf)
                  << std::endl;

        connection->send(buf, n);

        // Close the sockets
        connection->close();
        sock->close();
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
        std::shared_ptr<Sockets::TLSSocket> sock =
            Sockets::TLSSocket::connect("127.0.0.1", 12345, Sockets::Domain::IPv4, ctx);

        std::cout << "Enter the message: \n";

        std::getline(std::cin, s);
        sock->send(std::to_string(s.size()).c_str(), 1);
        sock->send(s.c_str(), s.size());

        sock->recv(buf, s.size());

        std::cout << "Client recieved " << s.size() << " bytes containing: " << std::string(buf)
                  << std::endl;

        // Close the connection
        sock->close();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        ERR_print_errors_fp(stderr);
    }

    SSL_CTX_free(ctx);

    t0.join();

    // TODO: check if there are other things to clean up

    cleanup_openssl();
}