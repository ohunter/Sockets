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

    Socket::~Socket() {
        if (this->state != State::Undefined || this->state != State::Closed)
            this->close();
    }

    void Socket::close() {
        if (this->state == State::Undefined || this->state == State::Closed)
            return;

        if (shutdown(this->_fd, SHUT_RDWR) == -1 && errno != ENOTCONN)
            perror("Non-fatal error when shutting down socket");

        if (::close(this->fd()) != 0 && errno != ENOTCONN)
            perror("Non-fatal error when closing socket");

        this->state = State::Closed;
    }

    size_t Socket::send(const char *buf, size_t buflen) {
        std::lock_guard<std::mutex> lock(this->mtx);
        size_t n = 0;
        ssize_t m = 0;

        while (n < buflen) {
            // std::cout << "Socket::send:\t" << std::string(buf) << std::endl;
            if ((m = ::send(this->_fd, &buf[n], buflen - n, 0)) < 0) {
                perror("");
                throw std::runtime_error("Error when sending data");
            }
            n += m;
        }

        return n;
    }

    size_t Socket::recv(char *buf, size_t buflen) {
        std::lock_guard<std::mutex> lock(this->mtx);
        size_t n = 0;
        ssize_t m = 0;

        while (n < buflen) {
            if ((m = ::recv(this->_fd, &buf[n], buflen - n, 0)) < 0) {
                perror("");
                throw std::runtime_error("Error when receiving data");
            }
            n += m;
        }

        return n;
    }
} // namespace Sockets