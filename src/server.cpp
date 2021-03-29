#include <iostream>
#include <cstring>

#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>

#include "../include/server.hpp"

namespace Sockets {
        Server::Server(std::string addr, uint16_t port, size_t workers)
        {
                int fd;
                struct sockaddr_in serv_addr;

                if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                        perror("Error when establishing socket");
                        exit(EXIT_FAILURE);
                }

                // Enforce that all the fields in the struct are 0
                std::memset((void *)&serv_addr, 0, sizeof(struct sockaddr_in));

                serv_addr.sin_family = AF_INET;         // IPv4 connection
                serv_addr.sin_addr.s_addr = INADDR_ANY; // The system's current IP address
                serv_addr.sin_port = htons(port);       // The port converted to little-endian

                this->server_sock = new socket_t(fd, serv_addr);

                // Bind to port to ensure that it is available
                if (bind(fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
                        perror("Error when binding socket to address");
                        exit(EXIT_FAILURE);
                }

                struct pollfd tmp = {fd, POLLIN, 0};
                this->items.push_back(tmp);

                this->pool = new ThreadPool(workers);
        }

        Server::~Server()
        {
                this->pool->~ThreadPool();
        }

        void Server::serve(int backlog)
        {
                int num = 0;

                if (listen(this->server_sock->fd(), backlog)) {
                        perror("Error when trying to listen on socket");
                        exit(EXIT_FAILURE);
                }

                if (fcntl(this->server_sock->fd(), F_SETFL, fcntl(this->server_sock->fd(), F_GETFL) | O_NONBLOCK) < 0) {
                        perror("Error when making socket non-blocking");
                        exit(EXIT_FAILURE);
                }

                while (true) {
                        if ((num = poll(this->items.data(), this->items.size(), 0)) < 0) {
                                perror("Error when polling current connections");
                                exit(EXIT_FAILURE);
                        } else if (num > 0) {
                                if (this->items[0].revents & POLLIN) {
                                        this->accept();
                                }

                                for (auto it = std::next(this->items.begin()); it != this->items.end(); it++) {
                                        if ((*it).revents & POLLERR)
                                        {}
                                        if ((*it).revents & POLLIN)
                                        {}
                                        if ((*it).revents & POLLOUT)
                                        {}
                                }
                        }
                }

        }

        void Server::accept()
        {

        }
}