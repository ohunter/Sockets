#include <cstring>
#include <iostream>
#include <stdexcept>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include "socket.hpp"

namespace Sockets {

    TCPSocket TCPSocket::Service(std::string address, uint16_t port, Domain dom,
                                 ByteOrder bo, Operation op, int backlog) {
        int fd;
        struct addrinfo *addr = resolve(address, port, dom, Type::Stream);
        sockaddr_storage info;

        if ((fd = socket(addr->ai_family, addr->ai_socktype,
                         addr->ai_protocol)) < 0) {
            perror("");
            throw std::runtime_error("Error when establishing socket");
        }

        std::memset(&info, 0, sizeof(sockaddr_storage));

        switch (dom) {
        case Domain::IPv4:
        case Domain::IPv6:
            std::memcpy(&info, addr->ai_addr, sizeof(struct sockaddr_storage));
            break;
        default:
            // TODO: Handle unix and undefined domains
            break;
        }

        if (op == Operation::Non_blocking) {
            if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) < 0) {
                perror("");
                throw std::runtime_error(
                    "Error when making socket non-blocking");
            }
        }

        if (bind(fd, (struct sockaddr *)&info, sizeof(info)) < 0) {
            perror("");
            throw std::runtime_error("Error when binding socket to address");
        }

        if (listen(fd, backlog)) {
            perror("");
            throw std::runtime_error("Error when trying to listen on socket");
        }

        freeaddrinfo(addr);

        return TCPSocket(fd, info, dom, State::Servicing, bo, op);
    }

    TCPSocket TCPSocket::Connect(std::string address, uint16_t port, Domain dom,
                                 ByteOrder bo, Operation op) {
        int fd;
        struct addrinfo *addr = resolve(address, port, dom, Type::Stream);
        sockaddr_storage info;

        if ((fd = socket(addr->ai_family, addr->ai_socktype,
                         addr->ai_protocol)) < 0) {
            perror("");
            throw std::runtime_error("Error when establishing socket");
        }

        std::memset(&info, 0, sizeof(sockaddr_storage));

        switch (dom) {
        case Domain::IPv4:
        case Domain::IPv6:
            std::memcpy(&info, addr->ai_addr, sizeof(struct sockaddr_storage));
            break;
        default:
            // TODO: Handle unix and undefined domains
            break;
        }

        if (op == Operation::Non_blocking) {
            if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK) < 0) {
                perror("");
                throw std::runtime_error(
                    "Error when making socket non-blocking");
            }
        }

        if (connect(fd, (struct sockaddr *)&info, sizeof(info)) < 0) {
            perror("");
            throw std::runtime_error(
                "Error when trying to connect to destination");
        }

        freeaddrinfo(addr);

        return TCPSocket(fd, info, dom, State::Connected, bo, op);
    }

    TCPSocket TCPSocket::accept(Operation op, int flag) {
        int fd;
        sockaddr_storage info;
        socklen_t len = sizeof(struct sockaddr_in);

        if (this->state != State::Servicing)
            throw std::runtime_error(
                "Cannot accept connection on a non-servicing socket");

        if (op == Operation::Non_blocking)
            flag |= SOCK_NONBLOCK;

        if ((fd = ::accept4(this->fd(), (struct sockaddr *)&info, &len,
                            flag)) == -1) {
            perror("");
            throw std::runtime_error("Error on accepting connection");
        }

        return TCPSocket(fd, addr, this->domain, State::Connected,
                         this->byteorder, op);
    }
} // namespace Sockets
