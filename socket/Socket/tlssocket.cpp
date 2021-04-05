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

    TLSSocket::~TLSSocket() { }

    TLSSocket::TLSSocket(TCPSocket &tcp, SSL_CTX *ctx) : TCPSocket(tcp) {
        if ((this->ssl = SSL_new(ctx)) == NULL) {
            throw std::runtime_error("Error when creating SSL state");
        }

        if (SSL_set_fd(this->ssl, this->fd()) == 0) {
            throw std::runtime_error("Error when attempting to bind file "
                                     "descriptor to SSL state");
        }
    }

    TLSSocket::TLSSocket(TCPSocket *tcp, SSL_CTX *ctx) : TCPSocket(tcp) {
        if ((this->ssl = SSL_new(ctx)) == NULL) {
            throw std::runtime_error("Error when creating SSL state");
        }

        if (SSL_set_fd(this->ssl, this->fd()) == 0) {
            throw std::runtime_error("Error when attempting to bind file "
                                     "descriptor to SSL state");
        }
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
        auto out = TLSSocket(tcp, ctx);
        int  m   = 0;

        // Start handshaking process
        if ((m = SSL_connect(out.ssl)) != 1)
            throw_ssl_error(SSL_get_error(out.ssl, m));

        // Check if the socket received a certificate
        X509 *cert = SSL_get_peer_certificate(out.ssl);

        if (!cert)
            throw std::runtime_error(
                "No X509 certificate received from server");

        X509_free(cert);

        // Verify the received certificate
        if (SSL_get_verify_result(out.ssl) != X509_V_OK)
            throw std::runtime_error("Failed to verify received certificate");

        return out;
    }

    TLSSocket TLSSocket::accept(SSL_CTX *ctx, Operation op, int flag) {
        auto tcp =
            TCPSocket::accept(Operation::Blocking, flag & ~SOCK_NONBLOCK);

        auto out = TLSSocket(tcp, ctx);
        int  m   = 0;

        if ((m = SSL_accept(out.ssl)) == 0)
            throw std::runtime_error("Graceful rejection of SSL handshake");
        else if (m < 0)
            throw std::runtime_error("Error when performing SSL handshake");

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
        int m;

        if (this->state != State::Undefined || this->state != State::Closed) {
            if ((m = SSL_shutdown(this->ssl)) == 0) {

            } else if (m < 0)
                throw_ssl_error(SSL_get_error(this->ssl, m));

            SSL_free(this->ssl);
        }

        TCPSocket::close();
    }

    size_t TLSSocket::send(const char *buf, size_t buflen) {
        std::lock_guard<std::mutex> lock(this->mtx);
        size_t                      n = 0;
        ssize_t                     m = 0;

        while (n < buflen) {
            if ((m = SSL_write(this->ssl, &buf[n], buflen - n)) <= 0)
                throw_ssl_error(SSL_get_error(this->ssl, m));

            n += m;
        }

        return n;
    }

    size_t TLSSocket::recv(char *buf, size_t buflen) {
        std::lock_guard<std::mutex> lock(this->mtx);
        size_t                      n = 0;
        ssize_t                     m = 0;

        while (n < buflen) {
            if ((m = SSL_read(this->ssl, &buf[n], buflen - n)) <= 0)
                throw_ssl_error(SSL_get_error(this->ssl, m));

            n += m;
        }

        return 0;
    }

} // namespace Sockets