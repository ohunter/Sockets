#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <mutex>

#include <netinet/in.h>
#include <poll.h>

#include "threadpool.hpp"

namespace Sockets {
        class socket_t{
                int _fd;
                struct sockaddr_in addr;

                std::mutex state;

                public:
                socket_t(int fd) : _fd(fd) {}
                socket_t(int fd, struct sockaddr_in addr) : _fd(fd), addr(addr) {}

                int send(uint8_t* buf, uint64_t buflen);
                int recv(uint8_t* buf, uint64_t buflen);

                int& fd() {return this->_fd;}
        };

        class Server {
                socket_t* server_sock = nullptr;
                ThreadPool* pool = nullptr;

                std::unordered_map<int, socket_t> connections;
                std::vector<struct pollfd> items;

                void accept();

                public:
                Server(std::string addr, uint16_t port, size_t workers = 10);
                ~Server();

                void serve(int backlog = 100);
        };
}