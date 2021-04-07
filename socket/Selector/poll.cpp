
#include <algorithm>
#include <cstring>

#include "selector.hpp"
namespace Sockets {
    Poll::Poll() { this->pfds = new struct pollfd[0]; }

    Poll::~Poll() { delete[] this->pfds; }

    std::array<std::vector<std::reference_wrapper<Polldev>>, 3>
    Poll::poll(int timeout) {
        std::array<std::vector<std::reference_wrapper<Polldev>>, 3> out;

        if (::poll(this->pfds, this->devs.size(), timeout) < 0) {
            perror("");
            throw std::runtime_error("Error when polling sockets");
        }

        auto it_end_err = std::partition(this->devs.begin(), this->devs.end(),
                                         Polldev::error);
        auto it_end_inc =
            std::partition(it_end_err, this->devs.end(), Polldev::incoming);
        auto it_end_out =
            std::partition(it_end_inc, this->devs.end(), Polldev::outgoing);

        out[0] = std::vector<std::reference_wrapper<Polldev>>(
            this->devs.begin(), it_end_err);
        out[1] = std::vector<std::reference_wrapper<Polldev>>(it_end_err,
                                                              it_end_inc);
        out[2] = std::vector<std::reference_wrapper<Polldev>>(it_end_inc,
                                                              it_end_out);

        return out;
    }

    void Poll::enroll(Socket *s, short event) {
        struct pollfd *new_arr = new struct pollfd[this->devs.size() + 1];

        std::memmove(new_arr, this->pfds,
                     sizeof(struct pollfd) * this->devs.size());

        delete[] this->pfds;

        this->pfds = new_arr;

        struct pollfd new_dev = {s->fd(), event, 0};

        std::memcpy(&this->pfds[this->devs.size()], &new_dev,
                    sizeof(struct pollfd));

        this->devs.emplace_back(s, &this->pfds[this->devs.size()]);
    }

    void Poll::disenroll(int fd) {
        // TODO: Implement these
        throw std::runtime_error("Poll::disenroll(int fd) has yet to be implemented");
    }

    void Poll::disenroll(Socket *s) {
        // TODO: Implement these
        throw std::runtime_error("Poll::disenroll(Socket *s) has yet to be implemented");
    }

} // namespace Sockets