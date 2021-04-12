#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <poll.h>

namespace Sockets {
    // S has to be aderivative of from Sockets::Socket for the program to compile
    template <class S>
    class Poll {
        std::vector<struct pollfd>      fds;
        std::vector<std::shared_ptr<S>> devs;

        public:
        Poll<S>();
        ~Poll<S>();

        std::array<std::vector<std::shared_ptr<S>>, 3> poll(int timeout = -1);

        void enroll(std::shared_ptr<S> s, short event = POLLIN | POLLOUT);
        void disenroll(int fd);
        void disenroll(std::shared_ptr<S> s);
    };
} // namespace Sockets