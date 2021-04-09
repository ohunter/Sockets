#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <poll.h>

namespace Sockets {
    class Socket;

    class Poll {
        std::vector<struct pollfd>           fds;
        std::vector<std::shared_ptr<Socket>> devs;

        public:
        Poll();
        ~Poll();

        std::array<std::vector<std::shared_ptr<Socket>>, 3> poll(int timeout = -1);

        void enroll(std::shared_ptr<Socket> s, short event = POLLIN | POLLOUT);
        void disenroll(int fd);
        void disenroll(std::shared_ptr<Socket> s);
    };
} // namespace Sockets