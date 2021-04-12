#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#include <Sockets/polling.hpp>
#include <Sockets/socket.hpp>

std::string process(std::shared_ptr<Sockets::Socket> p) {
    char   buf[256] = {0};
    size_t n;

    n = p->recv(buf, 256);

    std::cout << "Socket received " << n << " bytes" << std::endl;

    return std::string(buf);
}

void server(std::string address, uint16_t port) {
    bool state = true;

    // Establish the listening socket
    std::shared_ptr<Sockets::TCPSocket> listener = Sockets::TCPSocket::service(
        address, port, Sockets::Domain::IPv4, Sockets::Operation::Non_blocking);

    // Create the polling device
    auto pd = Sockets::Poll();

    // Register the listening socket on the polling device
    // This socket only needs to be registered with the `POLLIN` event as it
    // will only listen for new connections
    pd.enroll(listener, POLLIN);

    std::cout << "Waiting for connection\n";

    while (state) {
        auto activity = pd.poll();

        // The connections with errors
        for (auto it : activity[0]) {
            // Is there an issue with the listening connection
            if (it == listener) {
                perror("server(std::string address, uint16_t port): ");
                state = false;
                break;
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
                pd.enroll(listener->accept(Sockets::Operation::Non_blocking));
                std::cout << "Accepting connection\n";
            } else {
                auto msg = process(it);

                std::cout << msg << std::endl;
                if (!std::strncmp(msg.c_str(), "stop", 4)) {
                    std::cout << "Stopping server" << std::endl;
                    state = false;
                    break;
                }
            }
        }

        // The connections able to send outgoing
        // for (auto it : activity[2]) {}
    }

    listener->close();
}

int main(int argc, char *argv[]) {
    // Start server
    std::thread t0(server, "127.0.0.1", 12345);

    // Sleep for 1 second to let the server get to the `accept` call
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::string s;

    // Connect to the server
    auto sock = Sockets::TCPSocket::connect("127.0.0.1", 12345, Sockets::Domain::IPv4);

    std::cout << "Enter 'stop' to terminate connection." << std::endl;
    while (std::getline(std::cin, s)) {
        if (std::strncmp(s.c_str(), "stop", 4)) {
            sock->send(s.c_str(), s.size());
        } else {
            std::cout << "Stopping client" << std::endl;
            sock->send("stop", 4);
            break;
        }
    }

    // Close the connection
    sock->close();

    t0.join();
}