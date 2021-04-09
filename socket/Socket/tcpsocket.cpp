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

    TCPSocket::TCPSocket(int fd, sockaddr_storage &info, Domain dom, Operation op)
        : Socket(fd, info, dom, Type::Stream, op) { }

    TCPSocket::TCPSocket(struct addrinfo &info, Domain dom, Operation op)
        : Socket(info, dom, Type::Stream, op) { }

    TCPSocket::TCPSocket(TCPSocket *other) : Socket(other) { }

    TCPSocket::TCPSocket(TCPSocket &other) : Socket(other) { }

    TCPSocket::TCPSocket(TCPSocket &&other) : Socket(other) { }

    TCPSocket::~TCPSocket() { }

    void TCPSocket::connect() {
        if (this->state != State::Instantiated)
            throw std::runtime_error("Cannot connect with a busy socket");

        if (::connect(this->_fd, (struct sockaddr *)&this->addr, sizeof(this->addr)) < 0) {
            perror("TCPSocket::connect(): ");
            throw std::runtime_error("Error when trying to connect to destination");
        }

        this->state = State::Connected;
    }

    void TCPSocket::service(int backlog) {
        if (this->state != State::Instantiated)
            throw std::runtime_error("Cannot service with a busy socket");

        if (bind(this->_fd, (struct sockaddr *)&this->addr, sizeof(this->addr)) < 0) {
            perror("TCPSocket::service(int): ");
            throw std::runtime_error("Error when binding socket to address");
        }

        if (listen(this->_fd, backlog)) {
            perror("TCPSocket::service(int): ");
            throw std::runtime_error("Error when trying to listen on socket");
        }

        this->state = State::Open;
    }

    TCPSocket *TCPSocket::connect(std::string address, uint16_t port, Domain dom, Operation op) {
        auto addr = resolve(address, port, dom, Type::Stream);

        TCPSocket *sock = new TCPSocket(*addr, dom, op);

        freeaddrinfo(addr);

        sock->connect();
        return sock;
    }

    TCPSocket *TCPSocket::service(std::string address, uint16_t port, Domain dom, Operation op,
                                  int backlog) {
        auto addr = resolve(address, port, dom, Type::Stream);

        TCPSocket *sock = new TCPSocket(*addr, dom, op);

        freeaddrinfo(addr);

        sock->service(backlog);
        return sock;
    }

    TCPSocket *TCPSocket::accept(Operation op, int flag) {
        int              fd;
        sockaddr_storage info;
        socklen_t        len = sizeof(struct sockaddr_in);

        if (this->state != State::Open)
            throw std::runtime_error("Cannot accept connection on a socket that is not open");

        if (op == Operation::Non_blocking)
            flag |= SOCK_NONBLOCK;

        if ((fd = ::accept4(this->fd(), (struct sockaddr *)&info, &len, flag)) == -1) {
            perror("TCPSocket::accept(Operation, int): ");
            throw std::runtime_error("Error on accepting connection");
        }

        TCPSocket *out = new TCPSocket(fd, addr, this->domain, op);
        out->state     = State::Connected;

        // Close the accepted file descriptor as it is duplicated in the
        // constructor
        if (::close(fd) != 0 && errno != ENOTCONN)
            perror("Non-fatal error when closing file descriptor");

        return out;
    }

    void TCPSocket::close() { Socket::close(); }

    size_t TCPSocket::send(const char *buf, size_t buflen) {
        std::lock_guard<std::mutex> lock(this->mtx);
        size_t                      n = 0;
        ssize_t                     m = 0;

        while (n < buflen) {
            if ((m = ::send(this->_fd, &buf[n], buflen - n, 0)) < 0) {
                perror("TCPSocket::send(const char *, size_t): ");
                throw std::runtime_error("Error when sending data");
            }
            n += m;
        }

        return n;
    }

    size_t TCPSocket::recv(char *buf, size_t buflen) {
        std::lock_guard<std::mutex> lock(this->mtx);
        size_t                      n = 0;
        ssize_t                     m = 0;

        do {
            if ((m = ::recv(this->_fd, &buf[n], buflen - n, 0)) < 0) {
                perror("TCPSocket::recv(char *, size_t): ");
                throw std::runtime_error("Error when receiving data");
            } else if (m == 0) {
                // Connection has been shut down
            }
            n += m;
        } while (n < buflen && this->operation == Operation::Blocking);

        return n;
    }
} // namespace Sockets
