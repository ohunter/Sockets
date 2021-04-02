#pragma once

#include <mutex>
#include <unordered_map>

#include <endian.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

namespace Sockets {
    enum class Domain {
        Undefined = AF_UNSPEC,
        UNIX = AF_UNIX,
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
    };
    enum class Type { Undefined, Stream = SOCK_STREAM, Datagram = SOCK_DGRAM };
    enum class State { Undefined, Closed, Open, Connected, Error, Servicing };
    enum class ByteOrder { Native, Little, Big };
    enum class Operation { Blocking, Non_blocking };

    struct addrinfo *resolve(std::string address, std::string service,
                             Domain dom = Domain::Undefined,
                             Type ty = Type::Undefined, int flags = 0);
    struct addrinfo *resolve(std::string address, uint16_t port,
                             Domain dom = Domain::Undefined,
                             Type ty = Type::Undefined, int flags = 0);

    class Socket {
      protected:
        std::mutex mtx;
        int _fd;
        sockaddr_storage addr;

        Domain domain = Domain::Undefined;
        Type type = Type::Undefined;
        State state = State::Undefined;
        ByteOrder byteorder = ByteOrder::Native;
        Operation operation = Operation::Blocking;

      public:
        Socket(int fd, sockaddr_storage addr, Domain dom, Type ty,
               State st = State::Undefined, ByteOrder by = ByteOrder::Native,
               Operation op = Operation::Blocking)
            : _fd(fd), addr(addr), domain(dom), type(ty), state(st),
              byteorder(by), operation(op){};

        Socket(Socket &other)
            : _fd(other._fd), addr(other.addr), domain(other.domain),
              type(other.type), state(other.state), byteorder(other.byteorder),
              operation(other.operation) { }
        Socket(Socket &&other)
            : _fd(other._fd), addr(other.addr), domain(other.domain),
              type(other.type), state(other.state), byteorder(other.byteorder),
              operation(other.operation) { }
        Socket(Socket *other)
            : _fd(other->_fd), addr(other->addr), domain(other->domain),
              type(other->type), state(other->state),
              byteorder(other->byteorder), operation(other->operation) { }

        ~Socket();

        void close();
        size_t send(const char *buf, size_t buflen);
        size_t recv(char *buf, size_t buflen);

        const int &fd() { return this->_fd; }
    };

    class TCPSocket : public Socket {
      public:
        TCPSocket(int fd, sockaddr_storage addr, Domain dom, State st,
                  ByteOrder by, Operation op)
            : Socket(fd, addr, dom, Type::Stream, st, by, op) { }

        static TCPSocket Service(std::string address, uint16_t port, Domain dom,
                                 ByteOrder bo = ByteOrder::Native,
                                 Operation op = Operation::Blocking,
                                 int backlog = 100);

        static TCPSocket Connect(std::string address, uint16_t port, Domain dom,
                                 ByteOrder bo = ByteOrder::Native,
                                 Operation op = Operation::Blocking);

        TCPSocket accept(Operation op = Operation::Blocking, int flag = 0);
    };

    class UDPSocket : Socket { };

    class TLSSocket : public TCPSocket { };

    class SocketStream {
      public:
        SocketStream(Socket &s);
        SocketStream(Socket *s);
    };
} // namespace Sockets