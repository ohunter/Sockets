#include <cstring>
#include <iostream>
#include <stdexcept>

#include <errno.h>
#include <unistd.h>

#include "socket.hpp"

namespace Sockets {

    struct addrinfo *resolve(std::string address, std::string service,
                             Domain dom, Type ty, int flags) {
        int err;
        struct addrinfo hints;
        struct addrinfo *result = nullptr;
        struct addrinfo *out = new struct addrinfo();

        std::memset((void *)&hints, 0, sizeof(struct addrinfo));

        hints.ai_family = static_cast<int>(dom);
        hints.ai_socktype = static_cast<int>(ty);
        hints.ai_flags = flags;

        if ((err = getaddrinfo(address.c_str(), service.c_str(), &hints,
                               &result)) != 0) {
            throw std::runtime_error(std::string(gai_strerror(err)));
        }

        std::memcpy(out, result, sizeof(struct addrinfo));
        freeaddrinfo(result);

        out->ai_next = nullptr;
        return out;
    }

    struct addrinfo *resolve(std::string address, uint16_t port, Domain dom,
                             Type ty, int flags) {
        int err;
        struct addrinfo hints;
        struct addrinfo *result = nullptr;
        struct addrinfo *out = new struct addrinfo();
        std::string service = std::to_string(port);

        std::memset((void *)&hints, 0, sizeof(struct addrinfo));

        hints.ai_family = static_cast<int>(dom);
        hints.ai_socktype = static_cast<int>(ty);
        hints.ai_flags = flags;

        if ((err = getaddrinfo(address.c_str(), service.c_str(), &hints,
                               &result)) != 0) {
            throw std::runtime_error(std::string(gai_strerror(err)));
        }

        std::memcpy(out, result, sizeof(struct addrinfo));
        freeaddrinfo(result);

        out->ai_next = nullptr;
        return out;
    }

    Socket::Socket(Socket &other) {
        std::cout << "here 17:\t" << valid_fd(other.fd()) << "\t" << other.fd() << std::endl;
        if ((this->_fd = dup(this->fd())) == -1) {
            perror("");
            throw std::runtime_error("Error when duplicating file descriptor");
        }
        std::cout << "here 20:\t" << valid_fd(this->fd()) << "\t" << this->fd() << std::endl;

        this->addr = other.addr;
        this->domain = other.domain;
        this->type = other.type;
        this->state = other.state;
        this->byteorder = other.byteorder;
        this->operation = other.operation;
    }

    Socket::Socket(Socket &&other) {
        std::cout << "here 18:\t" << valid_fd(other.fd()) << "\t" << other.fd() << std::endl;
        if ((this->_fd = dup(this->fd())) == -1) {
            perror("");
            throw std::runtime_error("Error when duplicating file descriptor");
        }
        std::cout << "here 21:\t" << valid_fd(this->fd()) << "\t" << this->fd() << std::endl;

        this->addr = other.addr;
        this->domain = other.domain;
        this->type = other.type;
        this->state = other.state;
        this->byteorder = other.byteorder;
        this->operation = other.operation;
    }

    Socket::Socket(Socket *other) {
        std::cout << "here 19:\t" << valid_fd(other->fd()) << "\t" << other->fd() << std::endl;
        if ((this->_fd = dup(this->fd())) == -1) {
            perror("");
            throw std::runtime_error("Error when duplicating file descriptor");
        }
        std::cout << "here 22:\t" << valid_fd(this->fd()) << "\t" << this->fd() << std::endl;

        this->addr = other->addr;
        this->domain = other->domain;
        this->type = other->type;
        this->state = other->state;
        this->byteorder = other->byteorder;
        this->operation = other->operation;
    }

    Socket::~Socket() {
        if (!valid_fd(this->_fd))
            return;

        if (::close(this->_fd) == -1){
            perror("Error when closing file descriptor");
        }
    }

    void Socket::close() {
        if (this->state == State::Undefined || this->state == State::Closed)
            return;

        if (shutdown(this->fd(), SHUT_RDWR) == -1 && errno != ENOTCONN)
            perror("Non-fatal error when shutting down socket");

        if (::close(this->fd()) != 0 && errno != ENOTCONN)
            perror("Non-fatal error when closing socket");

        this->state = State::Closed;
    }

} // namespace Sockets