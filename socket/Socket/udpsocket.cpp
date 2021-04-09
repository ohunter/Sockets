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

    UDPSocket::UDPSocket(int fd, sockaddr_storage &info, Domain dom, Operation op)
        : Socket(fd, info, dom, Type::Datagram, op) { }

    UDPSocket::UDPSocket(UDPSocket *other) : Socket(other) { }

    UDPSocket::UDPSocket(struct addrinfo &info, Domain dom, Operation op)
        : Socket(info, dom, Type::Datagram, op) { }

    UDPSocket::UDPSocket(UDPSocket &other) : Socket(other) { }

    UDPSocket::UDPSocket(UDPSocket &&other) : Socket(other) { }

    UDPSocket::~UDPSocket() { }

    void UDPSocket::connect() {
        if (this->state != State::Instantiated)
            throw std::runtime_error("Cannot connect with a busy socket");

        if (::connect(this->_fd, (struct sockaddr *)&this->addr, sizeof(this->addr)) < 0) {
            perror("UDPSocket::connect(): ");
            throw std::runtime_error("Error when trying to connect to destination");
        }
    }

    void UDPSocket::service(int backlog) {
        if (this->state != State::Instantiated)
            throw std::runtime_error("Cannot service with a busy socket");

        if (bind(this->_fd, (struct sockaddr *)&this->addr, sizeof(this->addr)) < 0) {
            perror("UDPSocket::service(int): ");
            throw std::runtime_error("Error when binding socket to address");
        }

        if (listen(this->_fd, backlog)) {
            perror("UDPSocket::service(int): ");
            throw std::runtime_error("Error when trying to listen on socket");
        }
    }

    /* UDPSocket *UDPSocket::accept(Operation op, int flag) {
        throw std::runtime_error(
            "UDPSocket *UDPSocket::accept(Operation, int) has not been implemented yet");
    } */

    UDPSocket *UDPSocket::connect(std::string address, uint16_t port, Domain dom, Operation op) {
        auto addr = resolve(address, port, dom, Type::Datagram);

        UDPSocket *sock = new UDPSocket(*addr, dom, op);

        freeaddrinfo(addr);

        sock->connect();
        sock->state = State::Connected;
        return sock;
    }
    UDPSocket *UDPSocket::service(std::string address, uint16_t port, Domain dom, Operation op,
                                  int backlog) {
        auto addr = resolve(address, port, dom, Type::Datagram);

        UDPSocket *sock = new UDPSocket(*addr, dom, op);

        freeaddrinfo(addr);

        sock->service(backlog);
        sock->state = State::Open;
        return sock;
    }

    void UDPSocket::close() { Socket::close(); }

    size_t UDPSocket::send(const char *buf, size_t buflen) {
        std::lock_guard<std::mutex> lock(this->mtx);
        size_t                      n = 0;
        ssize_t                     m = 0;

        while (n < buflen) {
            if ((m = ::sendto(this->_fd, &buf[n], buflen - n, 0, (struct sockaddr *)&this->addr,
                              this->addr.ss_family == static_cast<int>(Domain::IPv4)
                                  ? sizeof(struct sockaddr_in)
                                  : sizeof(struct sockaddr_in6))) < 0) {
                perror("UDPSocket::send(const char *, size_t): ");
                throw std::runtime_error("Error when sending data");
            }
            n += m;
        }

        return n;
    }

    size_t UDPSocket::recv(char *buf, size_t buflen) {
        std::lock_guard<std::mutex> lock(this->mtx);
        size_t                      n = 0;
        ssize_t                     m = 0;

        socklen_t len = 0;

        while (n < buflen) {
            if ((m = ::recvfrom(this->_fd, &buf[n], buflen - n, 0, (struct sockaddr *)&this->addr,
                                &len)) < 0) {
                perror("UDPSocket::recv(char *, size_t): ");
                throw std::runtime_error("Error when receiving data");
            }
            n += m;
        }

        return n;
    }
} // namespace Sockets
