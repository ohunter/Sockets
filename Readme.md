# libSock

A simple git repo for socket development in C++ without the need for a huge amount of external dependencies such as Boost. OpenSSL support is planned. The underlying implementation is based on C++11, but ideally it would be updated to support more modern features such as those in C++ 17 and 20.

## Examples

### Example 0

A simple echo server which terminates after the client sends a message and receives it in return again. It is currently implemented using the following protocols:

- [TCP](https://github.com/ohunter/Sockets/tree/master/examples/tcp/0)
- [UDP](https://github.com/ohunter/Sockets/tree/master/examples/udp/0)
- [TLS](https://github.com/ohunter/Sockets/tree/master/examples/tls/0)

There are slight neuances to them. However I have tried my best to keep a consistent interface for all of them.