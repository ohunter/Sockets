#include <cstring>
#include <iostream>
#include <stdexcept>

#include <errno.h>
#include <unistd.h>

#include "socket.hpp"

namespace Sockets {

    struct addrinfo *resolve(std::string address, std::string service, Domain dom, Type ty,
                             int flags) {
        int              err;
        struct addrinfo  hints;
        struct addrinfo *result = nullptr;
        struct addrinfo *out    = new struct addrinfo();

        std::memset((void *)&hints, 0, sizeof(struct addrinfo));

        hints.ai_family   = static_cast<int>(dom);
        hints.ai_socktype = static_cast<int>(ty);
        hints.ai_flags    = flags;

        if ((err = getaddrinfo(address.c_str(), service.c_str(), &hints, &result)) != 0) {
            throw std::runtime_error(std::string(gai_strerror(err)));
        }

        std::memcpy(out, result, sizeof(struct addrinfo));
        freeaddrinfo(result);

        out->ai_next = nullptr;
        return out;
    }

    struct addrinfo *resolve(std::string address, uint16_t port, Domain dom, Type ty, int flags) {
        int              err;
        struct addrinfo  hints;
        struct addrinfo *result  = nullptr;
        struct addrinfo *out     = new struct addrinfo();
        std::string      service = std::to_string(port);

        std::memset((void *)&hints, 0, sizeof(struct addrinfo));

        hints.ai_family   = static_cast<int>(dom);
        hints.ai_socktype = static_cast<int>(ty);
        hints.ai_flags    = flags;

        if ((err = getaddrinfo(address.c_str(), service.c_str(), &hints, &result)) != 0) {
            throw std::runtime_error(std::string(gai_strerror(err)));
        }

        std::memcpy(out, result, sizeof(struct addrinfo));
        freeaddrinfo(result);

        out->ai_next = nullptr;
        return out;
    }

    Socket::Socket(int fd, sockaddr_storage &info, Domain dom, Type ty, Operation op) {

        if ((this->_fd = dup(fd)) < 0) {
            perror("Socket::Socket(int, struct addrinfo &, Domain, Type, "
                   "Operation): ");
            throw std::runtime_error("Error when making socket non-blocking");
        }

        this->addr      = info;
        this->domain    = dom;
        this->type      = ty;
        this->operation = op;
    }

    Socket::Socket(struct addrinfo &info, Domain dom, Type ty, Operation op) {

        if ((this->_fd = socket(info.ai_family, info.ai_socktype, info.ai_protocol)) < 0) {
            perror("Socket::Socket(struct addrinfo&, Domain, Type "
                   ", Operation): ");
            throw std::runtime_error("Error when establishing socket");
        }

        std::memset(&this->addr, 0, sizeof(sockaddr_storage));

        switch (dom) {
        case Domain::IPv4:
        case Domain::IPv6:
            std::memcpy(&this->addr, info.ai_addr, sizeof(struct sockaddr_storage));
            break;
        default:
            // TODO: Handle unix and undefined domains
            break;
        }

        if (op == Operation::Non_blocking) {
            if (fcntl(this->_fd, F_SETFL, fcntl(this->_fd, F_GETFL) | O_NONBLOCK) < 0) {
                perror("Socket::Socket(struct addrinfo&, Domain, Type "
                       ", Operation): ");
                throw std::runtime_error("Error when making socket non-blocking");
            }
        }

        this->domain    = dom;
        this->type      = ty;
        this->operation = op;
    }

    Socket::Socket(Socket *other) {
        if ((this->_fd = dup(other->fd())) == -1) {
            perror("Socket::Socket(Socket*): ");
            throw std::runtime_error("Error when duplicating file descriptor");
        }

        this->addr      = other->addr;
        this->domain    = other->domain;
        this->type      = other->type;
        this->state     = other->state;
        this->operation = other->operation;
    }

    Socket::Socket(Socket &other) {

        if ((this->_fd = dup(other.fd())) == -1) {
            perror("Socket::Socket(Socket&): ");
            throw std::runtime_error("Error when duplicating file descriptor");
        }

        this->addr      = other.addr;
        this->domain    = other.domain;
        this->type      = other.type;
        this->state     = other.state;
        this->operation = other.operation;
    }

    Socket::Socket(Socket &&other) {
        if ((this->_fd = dup(other.fd())) == -1) {
            perror("Socket::Socket(Socket&&): ");
            throw std::runtime_error("Error when duplicating file descriptor");
        }

        this->addr      = other.addr;
        this->domain    = other.domain;
        this->type      = other.type;
        this->state     = other.state;
        this->operation = other.operation;
    }

    Socket::~Socket() {
        if (!valid_fd(this->_fd))
            return;

        if (::close(this->_fd) == -1) {
            perror("Error when closing file descriptor");
        }
    }

    void Socket::close() {
        if (this->state == State::Closed)
            return;

        if (shutdown(this->fd(), SHUT_RDWR) == -1 && errno != ENOTCONN)
            perror("Non-fatal error when shutting down socket");

        if (::close(this->fd()) != 0 && errno != ENOTCONN)
            perror("Non-fatal error when closing socket");

        this->state = State::Closed;
    }

} // namespace Sockets