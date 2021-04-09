#pragma once

#include <mutex>
#include <unordered_map>

#include <endian.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#define valid_fd(fd) (fcntl(fd, F_GETFD) != -1 || errno != EBADF)

namespace Sockets {

    // These enums are used to track information about what kind of socket a
    // socket object is
    enum class Domain {
        Undefined = AF_UNSPEC,
        UNIX      = AF_UNIX,
        IPv4      = AF_INET,
        IPv6      = AF_INET6,
    };
    enum class Type { Undefined, Stream = SOCK_STREAM, Datagram = SOCK_DGRAM };
    enum class State { Undefined, Closed, Connected, Open };
    enum class ByteOrder { Native, Little, Big };
    enum class Operation { Blocking, Non_blocking };

    // This function are used to convert DNS resolvable names and IPs to a
    // structure that is more suitable for the sockets
    struct addrinfo *resolve(std::string address, std::string service,
                             Domain dom = Domain::Undefined,
                             Type ty = Type::Undefined, int flags = 0);

    // This function are used to convert DNS resolvable names and IPs to a
    // structure that is more suitable for the sockets
    struct addrinfo *resolve(std::string address, uint16_t port,
                             Domain dom = Domain::Undefined,
                             Type ty = Type::Undefined, int flags = 0);

    /**
     * @brief The base socket class. Should not be instantiated by itself, but
     * rather through one of it's derived classes. Essentially acts as a
     * convenience layer between the user and the standard POSIX sockets.
     */
    class Socket {
        protected:
        std::mutex       mtx;
        int              _fd;
        sockaddr_storage addr;

        Domain    domain    = Domain::Undefined;
        Type      type      = Type::Undefined;
        State     state     = State::Undefined;
        ByteOrder byteorder = ByteOrder::Native;
        Operation operation = Operation::Blocking;

        Socket(int fd, sockaddr_storage addr, Domain dom, Type ty,
               State st = State::Undefined, Operation op = Operation::Blocking,
               ByteOrder by = ByteOrder::Native)
            : _fd(fd), addr(addr), domain(dom), type(ty), state(st),
              byteorder(by), operation(op){};

        public:
        Socket(Socket &other);
        Socket(Socket &&other);
        Socket(Socket *other);

        ~Socket();

        void           close();
        virtual size_t send(const char *buf, size_t buflen) = 0;
        virtual size_t recv(char *buf, size_t buflen)       = 0;

        const int &fd() { return this->_fd; }
    };

    /**
     * @brief A class which handles basic TCP socket. All the data is streamed
     * to the other end with all the standard TCP guarantees.
     *
     */
    class TCPSocket : public Socket {
        protected:
        TCPSocket(int fd, sockaddr_storage addr, Domain dom, State st,
                  Operation op, ByteOrder by)
            : Socket(fd, addr, dom, Type::Stream, st, op, by) { }

        public:
        TCPSocket(TCPSocket &other) : Socket(other) { }
        TCPSocket(TCPSocket &&other) : Socket(other) { }
        TCPSocket(TCPSocket *other) : Socket(other) { }

        ~TCPSocket();

        static TCPSocket Service(std::string address, uint16_t port, Domain dom,
                                 Operation op      = Operation::Blocking,
                                 ByteOrder bo      = ByteOrder::Native,
                                 int       backlog = 100);

        static TCPSocket Connect(std::string address, uint16_t port, Domain dom,
                                 Operation op = Operation::Blocking,
                                 ByteOrder bo = ByteOrder::Native);

        TCPSocket accept(Operation op = Operation::Blocking, int flag = 0);

        void   close();
        size_t send(const char *buf, size_t buflen) override;
        size_t recv(char *buf, size_t buflen) override;
    };

    /**
     * @brief A class which handles basic UDP sockets. The user is in charge of
     * handiling issues such as packages being lost or arriving out of order. No
     * guarantees whatsoever.
     *
     */
    class UDPSocket : public Socket {
        protected:
        UDPSocket(int fd, sockaddr_storage addr, Domain dom, State st,
                  Operation op, ByteOrder by)
            : Socket(fd, addr, dom, Type::Datagram, st, op, by) { }

        public:
        UDPSocket(UDPSocket &other) : Socket(other) { }
        UDPSocket(UDPSocket &&other) : Socket(other) { }
        UDPSocket(UDPSocket *other) : Socket(other) { }

        ~UDPSocket();

        static UDPSocket Service(std::string address, uint16_t port, Domain dom,
                                 Operation op = Operation::Blocking,
                                 ByteOrder bo = ByteOrder::Native);

        static UDPSocket Connect(std::string address, uint16_t port, Domain dom,
                                 Operation op = Operation::Blocking,
                                 ByteOrder bo = ByteOrder::Native);

        UDPSocket accept(Operation op = Operation::Blocking, int flag = 0);

        void   close();
        size_t send(const char *buf, size_t buflen) override;
        size_t recv(char *buf, size_t buflen) override;
    };

    /**
     * @brief A class which provides a TLS layer around the standard TCP socket.
     * The user should supply all the needed certificates if security is
     * critical. Don't rely on your default CA settings unless you are
     * prototyping.
     *
     * Before initializing this class, make sure to have initialized OpenSSL as
     * this will not take responsibility for that and the cleanup which comes
     * from it.
     *
     */
    class TLSSocket : public TCPSocket {
        SSL *ssl = nullptr;

        protected:
        TLSSocket(int fd, sockaddr_storage addr, Domain dom, State st,
                  Operation op, ByteOrder by)
            : TCPSocket(fd, addr, dom, st, op, by) { }

        TLSSocket(TCPSocket &tcp, SSL_CTX *ctx);
        TLSSocket(TCPSocket *tcp, SSL_CTX *ctx);

        public:
        TLSSocket(TLSSocket &other) : TCPSocket(other) { }
        TLSSocket(TLSSocket &&other) : TCPSocket(other) { }
        TLSSocket(TLSSocket *other) : TCPSocket(other) { }

        ~TLSSocket();

        static TLSSocket Service(std::string address, uint16_t port,
                                 SSL_CTX *ctx, Domain dom,
                                 Operation op      = Operation::Blocking,
                                 ByteOrder bo      = ByteOrder::Native,
                                 int       backlog = 100);

        static TLSSocket Connect(std::string address, uint16_t port,
                                 SSL_CTX *ctx, Domain dom,
                                 Operation op = Operation::Blocking,
                                 ByteOrder bo = ByteOrder::Native);

        TLSSocket accept(SSL_CTX *ctx, Operation op = Operation::Blocking,
                         int flag = 0);

        void   close();
        size_t send(const char *buf, size_t buflen);
        size_t recv(char *buf, size_t buflen);
    };

    /**
     * @brief A class which provides a TLS layer around the standard UDP socket.
     * The user should supply all the needed certificates if security is
     * critical. Don't rely on your default CA settings unless you are
     * prototyping.
     *
     * Before initializing this class, make sure to have initialized OpenSSL as
     * this will not take responsibility for that and the cleanup which comes
     * from it.
     *
     */
    class DTLSSocket : public UDPSocket { };
} // namespace Sockets