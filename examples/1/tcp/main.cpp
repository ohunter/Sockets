#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#include <Sockets/selector.hpp>
#include <Sockets/socket.hpp>

std::string process(std::shared_ptr<Sockets::Socket> p) {
    char   buf[256] = {0};
    size_t n;

    n = p->recv(buf, 256);

    std::cout << "Socket received " << n << " bytes" << std::endl;

    return std::string(buf);
}

void server(std::string address, uint16_t port) {
    // Establish the listening socket
    std::shared_ptr<Sockets::TCPSocket> listener =
        std::make_shared<Sockets::TCPSocket>(Sockets::TCPSocket::service(
            address, port, Sockets::Domain::IPv4, Sockets::Operation::Non_blocking));

    // Create the polling device
    auto pd = Sockets::Poll();

    // Register the listening socket on the polling device
    // This socket only needs to be registered with the `POLLIN` event as it
    // will only listen for new connections
    pd.enroll(listener, POLLIN);

    std::cout << "Waiting\n";

    while (true) {
        auto activity = pd.poll();

        // The connections with errors
        for (auto it : activity[0]) {
            // Is there an issue with the listening connection
            if (it == listener) {
                perror("server(std::string address, uint16_t port): ");
                throw std::runtime_error("Something went wrong with the listening socket");
            } else {
                // Close the faulty connection
                pd.disenroll(it);
                it->close();
            }
        }

        // The connections with incoming data
        for (auto it : activity[1]) {
            // There is an incoming connection
            if (it == listener) {
                pd.enroll(std::make_shared<Sockets::TCPSocket>(
                    listener->accept(Sockets::Operation::Non_blocking)));
                std::cout << "Accepting connection\n";
            } else
                std::cout << process(it) << std::endl;
        }

        // The connections able to send outgoing
        // for (auto it : activity[2]) {}
    }
}

int main(int argc, char *argv[]) {
    // Start server
    std::thread t0(server, "127.0.0.1", 54321);

    // // Sleep for 1 second to let the server get to the `accept` call
    // std::this_thread::sleep_for(std::chrono::seconds(1));

    // // Connect to the server
    // auto sock = Sockets::TCPSocket::connect("127.0.0.1", 12345, Sockets::Domain::IPv4);

    // // Close the connection
    // sock->close();

    t0.join();
}