#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include <Sockets/socket.hpp>

void server(std::string address, uint16_t port) {
    size_t n;
    char   buf[256] = {0};

    std::shared_ptr<Sockets::TCPSocket> listener =
        Sockets::TCPSocket::service(address, port, Sockets::Domain::IPv4);

    std::cout << "Waiting\n";

    // Accept the incoming connection
    std::shared_ptr<Sockets::TCPSocket> connection = listener->accept();

    std::cout << "Accepted connection\n";

    // Read the size of the incoming message
    connection->recv(buf, 1);
    n = std::atoi(buf);
    connection->recv(buf, n);

    std::cout << "Server recieved " << n << " bytes containing: " << std::string(buf) << std::endl;

    connection->send(buf, n);

    // Close the connection
    connection->close();
    listener->close();

    connection->~TCPSocket();
    listener->~TCPSocket();
}

int main(int argc, char *argv[]) {
    // Start server
    std::thread t0(server, "127.0.0.1", 12345);

    // Sleep for 1 second to let the server get to the `accept` call
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Connect to the server
    std::shared_ptr<Sockets::TCPSocket> sock =
        Sockets::TCPSocket::connect("127.0.0.1", 12345, Sockets::Domain::IPv4);

    char        buf[256] = {0};
    std::string s        = "";

    std::cout << "Enter the message: \n";

    std::getline(std::cin, s);
    sock->send(std::to_string(s.size()).c_str(), 1);
    sock->send(s.c_str(), s.size());

    sock->recv(buf, s.size());

    std::cout << "Client recieved " << s.size() << " bytes containing: " << std::string(buf)
              << std::endl;

    // Close the connection
    sock->close();

    t0.join();
}