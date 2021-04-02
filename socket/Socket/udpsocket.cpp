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

    UDPSocket::~UDPSocket() {
        if (this->state != State::Undefined || this->state != State::Closed)
            this->close();
    }

    UDPSocket UDPSocket::Service(std::string address, uint16_t port, Domain dom,
                                 ByteOrder bo, Operation op) {
        int fd;
        struct addrinfo *addr = resolve(address, port, dom, Type::Datagram);
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

        freeaddrinfo(addr);

        return UDPSocket(fd, info, dom, State::Open, bo, op);
    }

    UDPSocket UDPSocket::Connect(std::string address, uint16_t port, Domain dom,
                                 ByteOrder bo, Operation op) {
        int fd;
        struct addrinfo *addr = resolve(address, port, dom, Type::Datagram);
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

        freeaddrinfo(addr);

        return UDPSocket(fd, info, dom, State::Open, bo, op);
    }

    void UDPSocket::close() {
        if (this->state == State::Undefined || this->state == State::Closed)
            return;

        if (shutdown(this->_fd, SHUT_RDWR) == -1 && errno != ENOTCONN)
            perror("Non-fatal error when shutting down socket");

        if (::close(this->fd()) != 0 && errno != ENOTCONN)
            perror("Non-fatal error when closing socket");

        this->state = State::Closed;
    }

    size_t UDPSocket::send(const char *buf, size_t buflen) {
        std::lock_guard<std::mutex> lock(this->mtx);
        size_t n = 0;
        ssize_t m = 0;

        while (n < buflen) {
            if ((m = ::sendto(this->_fd, &buf[n], buflen - n, 0,
                              (struct sockaddr *)&this->addr,
                              this->addr.ss_family ==
                                      static_cast<int>(Domain::IPv4)
                                  ? sizeof(struct sockaddr_in)
                                  : sizeof(struct sockaddr_in6))) < 0) {
                perror("");
                throw std::runtime_error("Error when sending data");
            }
            n += m;
        }

        return n;
    }

    size_t UDPSocket::recv(char *buf, size_t buflen) {
        std::lock_guard<std::mutex> lock(this->mtx);
        size_t n = 0;
        ssize_t m = 0;

        socklen_t len = 0;

        while (n < buflen) {
            if ((m = ::recvfrom(this->_fd, &buf[n], buflen - n, 0,
                                (struct sockaddr *)&this->addr, &len)) < 0) {
                perror("");
                throw std::runtime_error("Error when receiving data");
            }
            n += m;
        }

        return n;
    }
} // namespace Sockets