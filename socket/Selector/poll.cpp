
#include <algorithm>
#include <cstring>

#include "../Socket/socket.hpp"
#include "selector.hpp"

namespace Sockets {
    Poll::Poll() { }

    Poll::~Poll() { }

    std::array<std::vector<std::shared_ptr<Socket>>, 3> Poll::poll(int timeout) {
        int n = 0;
        int i = 0;

        std::array<std::vector<std::shared_ptr<Socket>>, 3> out;

        if ((n = ::poll(this->fds.data(), this->fds.size(), timeout)) < 0) {
            perror("Poll::poll(int)");
            throw std::runtime_error("Error when polling sockets");
        }

        auto it_fd  = this->fds.begin();
        auto it_dev = this->devs.begin();
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

    void Poll::enroll(std::shared_ptr<Socket> s, short event) {
        pollfd tmp = {s->fd(), event, 0};
        this->fds.push_back(tmp);
        this->devs.push_back(s);
    }

    void Poll::disenroll(int fd) {
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

    void Poll::disenroll(std::shared_ptr<Socket> s) {
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

} // namespace Sockets