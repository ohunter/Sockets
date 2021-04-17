#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <poll.h>

namespace Sockets {
    class Socket;

    // S has to be aderivative of from Sockets::Socket for the program to compile
    template <class S>
    class Poll {
        std::vector<struct pollfd>      fds;
        std::vector<std::shared_ptr<S>> devs;

        public:
        Poll() {
            static_assert(std::is_base_of<Socket, S>::value,
                          "Templated class needs to inherit from Sockets::Socket");
        }

        ~Poll() { }

        std::array<std::vector<std::shared_ptr<S>>, 3> poll(int timeout = -1) {
            int n = 0;
            int i = 0;

            std::array<std::vector<std::shared_ptr<S>>, 3> out;

            if ((n = ::poll(this->fds.data(), this->fds.size(), timeout)) < 0) {
                perror("Poll::poll(int)");
                throw std::runtime_error("Error when polling sockets");
            }

            auto it_fd  = this->fds.begin();
            auto it_dev = this->devs.begin();
            // Do some bullshit
            while (i < n && it_fd != this->fds.end() && it_dev != this->devs.end()) {
                if ((*it_fd).revents & (POLLERR | POLLHUP | POLLNVAL)) {
                    out[0].emplace_back(*it_dev);
                    i++;
                }

                if ((*it_fd).revents & POLLIN) {
                    out[1].emplace_back(*it_dev);
                    i++;
                }

                if ((*it_fd).revents & POLLOUT) {
                    out[2].emplace_back(*it_dev);
                    i++;
                }

                it_fd++;
                it_dev++;
            }

            return out;
        }

        void enroll(std::shared_ptr<S> s, short event = POLLIN | POLLOUT) {
            // Gang gang
            pollfd tmp = {s->fd(), event, 0};
            this->fds.push_back(tmp);
            this->devs.push_back(s);
        }

        void disenroll(std::shared_ptr<S> s) {
            auto it_fd  = this->fds.begin();
            auto it_dev = this->devs.begin();

            // Locate the index which fits the description
            while (it_fd != this->fds.end() && it_dev != this->devs.end()) {
                if ((*it_dev) == s) {
                    this->fds.erase(it_fd);
                    this->devs.erase(it_dev);
                    break;
                }

                it_fd++;
                it_dev++;
            }
        }

        void disenroll(int fd) {
            auto it_fd  = this->fds.begin();
            auto it_dev = this->devs.begin();

            // Locate the index which fits the description
            while (it_fd != this->fds.end() && it_dev != this->devs.end()) {
                if ((*it_fd).fd == fd) {
                    this->fds.erase(it_fd);
                    this->devs.erase(it_dev);
                    break;
                }

                it_fd++;
                it_dev++;
            }
        }
    };

    template <class S>
    class Epoll { };
} // namespace Sockets