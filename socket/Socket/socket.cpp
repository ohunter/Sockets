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

} // namespace Sockets