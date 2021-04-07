
#include "selector.hpp"
namespace Sockets {

    Polldev::Polldev(Socket* sock, struct pollfd* pfd) {
        this->sock = std::shared_ptr<Socket>(sock);
        this->pfd = pfd;
    }

    Polldev::~Polldev() {

    }

    bool Polldev::error(const Polldev& p) {
        return p.pfd->revents & (POLLERR | POLLHUP | POLLNVAL);
    }

    bool Polldev::incoming(const Polldev& p) {
        return p.pfd->revents & POLLIN;
    }

    bool Polldev::outgoing(const Polldev& p) {
        return p.pfd->revents & POLLOUT;
    }

    bool Polldev::silent(const Polldev& p) {
        return p.pfd->revents == 0;
    }


} // namespace Sockets