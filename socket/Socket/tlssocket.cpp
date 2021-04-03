#include <cstring>
#include <iostream>
#include <stdexcept>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../Exceptions/exceptions.hpp"
#include "socket.hpp"

namespace Sockets {

    TLSSocket::TLSSocket(TCPSocket &tcp, SSL_CTX *ctx) : TCPSocket(tcp) {
        if ((this->ssl = SSL_new(ctx)) == NULL) {
            throw std::runtime_error("Error when creating TLS connection");
        }

        if (SSL_set_fd(this->ssl, this->fd()) == 0) {
            throw std::runtime_error("Error when attempting to bind file "
                                     "descriptor to TLS connection");
        }
    }

    TLSSocket::TLSSocket(TCPSocket *tcp, SSL_CTX *ctx) : TCPSocket(tcp) {
        if ((this->ssl = SSL_new(ctx)) == NULL) {
            throw std::runtime_error("Error when creating TLS connection");
        }

        if (SSL_set_fd(this->ssl, this->fd()) == 0) {
            throw std::runtime_error("Error when attempting to bind file "
                                     "descriptor to TLS connection");
        }
    }

    TLSSocket::~TLSSocket() {
        if (this->state != State::Undefined || this->state != State::Closed)
            this->close();
    }

    TLSSocket TLSSocket::Service(std::string address, uint16_t port,
                                 SSL_CTX *ctx, Domain dom, ByteOrder bo,
                                 Operation op, int backlog) {
        auto tcp = TCPSocket::Service(address, port, dom, bo, op, backlog);
        return TLSSocket(tcp, ctx);
    }

    TLSSocket TLSSocket::Connect(std::string address, uint16_t port,
                                 SSL_CTX *ctx, Domain dom, ByteOrder bo,
                                 Operation op) {
        auto tcp = TCPSocket::Connect(address, port, dom, bo, op);
        return TLSSocket(tcp, ctx);
    }

    TLSSocket TLSSocket::accept(SSL_CTX *ctx, Operation op, int flag) {
        auto tcp = TCPSocket::accept(Operation::Blocking, flag ^ SOCK_NONBLOCK);
        TLSSocket out(tcp, ctx);

        switch (SSL_accept(out.ssl)) {
        case 1:
            // Handshake was successful
            break;
        case 0:
            // Handshake was gracefully rejected
            throw std::runtime_error("Graceful rejection of SSL handshake");
            break;
        default:
            // Shit hit the fan during handshake
            throw std::runtime_error("Error when performing SSL handshake");
            break;
        }

        if (op == Operation::Non_blocking)
            if (fcntl(this->fd(), F_SETFL,
                      fcntl(this->fd(), F_GETFL, 0) | O_NONBLOCK) == -1) {
                perror("");
                throw std::runtime_error(
                    "Error when making socket non-blocking");
            }

        return out;
    }

    void TLSSocket::close() {
        if (this->state == State::Undefined || this->state == State::Closed)
            return;

        TCPSocket::close();

        SSL_free(this->ssl);
    }

    size_t TLSSocket::send(const char *buf, size_t buflen) {
        std::lock_guard<std::mutex> lock(this->mtx);
        size_t n = 0;
        ssize_t m = 0;

        while(n < buflen)
        {
            if ((m = SSL_write(this->ssl, &buf[n], buflen - n)) <= 0) {
                switch (SSL_get_error(this->ssl, m)) {
                case SSL_ERROR_WANT_WRITE:
                    throw ssl_error_want_write();
                    break;
                case SSL_ERROR_WANT_READ:
                    throw ssl_error_want_read();
                    break;
                case SSL_ERROR_ZERO_RETURN:
                    throw ssl_error_zero_return();
                    break;
                case SSL_ERROR_SYSCALL:
                    throw ssl_error_syscall();
                    break;
                case SSL_ERROR_SSL:
                    throw ssl_error_ssl();
                    break;
                default:
                    throw ssl_error();
                    break;
                }
            }

            n += m;
        }

        return n;
    }

    size_t TLSSocket::recv(char *buf, size_t buflen) {
        std::lock_guard<std::mutex> lock(this->mtx);
        size_t n = 0;
        ssize_t m = 0;

        while (n < buflen) {
            if ((m = SSL_read(this->ssl, &buf[n], buflen - n)) <= 0) {
                switch (SSL_get_error(this->ssl, m)) {
                case SSL_ERROR_WANT_WRITE:
                    throw ssl_error_want_write();
                    break;
                case SSL_ERROR_WANT_READ:
                    throw ssl_error_want_read();
                    break;
                case SSL_ERROR_ZERO_RETURN:
                    throw ssl_error_zero_return();
                    break;
                case SSL_ERROR_SYSCALL:
                    throw ssl_error_syscall();
                    break;
                case SSL_ERROR_SSL:
                    throw ssl_error_ssl();
                    break;
                default:
                    throw ssl_error();
                    break;
                }
            }

            n += m;
        }

        return 0;
    }

} // namespace Sockets