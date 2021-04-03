#pragma once

#include <exception>
#include <stdexcept>

#include <openssl/ssl.h>

namespace Sockets {

    class ssl_error : public std::exception {
      public:
        const char *what() const throw() { return "Unknown OpenSSL error"; }
    };

    // class ssl_error_none : public ssl_error {
    //   public:
    //     const char *what() const throw() { return ""; }
    // };

    class ssl_error_zero_return : public ssl_error {
      public:
        const char *what() const throw() {
            return "TLS connection received close_notify alert";
        }
    };

    class ssl_error_want_read : public ssl_error {
      public:
        const char *what() const throw() {
            return "Not enough data available in buffer";
        }
    };

    class ssl_error_want_write : public ssl_error {
      public:
        const char *what() const throw() {
            return "Unable to write all the data to buffer";
        }
    };

    class ssl_error_want_connect : public ssl_error {
      public:
        const char *what() const throw() {
            return "Connection was not established";
        }
    };

    class ssl_error_want_accept : public ssl_error {
      public:
        const char *what() const throw() {
            return "Could not accept incoming connection";
        }
    };

    class ssl_error_want_x509_lookup : public ssl_error {
      public:
        const char *what() const throw() {
            return "TLS I/O function should be called again";
        }
    };

    class ssl_error_want_async : public ssl_error {
      public:
        const char *what() const throw() {
            return "Asynchronous engine is still processing";
        }
    };

    class ssl_error_want_async_job : public ssl_error {
      public:
        const char *what() const throw() {
            return "No available async job in pool";
        }
    };

    class ssl_error_want_client_hello_cb : public ssl_error {
      public:
        const char *what() const throw() {
            return "TLS I/O function should be called again";
        }
    };

    class ssl_error_syscall : public ssl_error {
      public:
        const char *what() const throw() { return "Fatal I/O error"; }
    };

    class ssl_error_ssl : public ssl_error {
      public:
        const char *what() const throw() { return "Fatal SSL error"; }
    };

} // namespace Sockets