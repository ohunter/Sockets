#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <poll.h>

#include "../Socket/socket.hpp"

namespace Sockets {
    class Polldev {
        struct pollfd *         pfd  = nullptr;
        std::shared_ptr<Socket> sock = nullptr;

        public:
        Polldev(Socket *sock, struct pollfd *pfd);
        ~Polldev();

        static bool error(const Polldev &p);
        static bool incoming(const Polldev &p);
        static bool outgoing(const Polldev &p);
        static bool silent(const Polldev &p);
    };

    class Poll {
        struct pollfd *      pfds = nullptr;
        std::vector<Polldev> devs;

        public:
        Poll();
        ~Poll();

        std::array<std::vector<std::reference_wrapper<Polldev>>, 3>
        poll(int timeout = 0);

        void enroll(Socket *s, short event = POLLIN | POLLOUT);
        void disenroll(int fd);
        void disenroll(Socket *s);
    };
} // namespace Sockets