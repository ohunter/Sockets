# libSock

libSock is an experimental library meant to broaden my understanding of working with sockets while also providing a convenience layer for the programmer which wants to play around with them. Currently only Linux is supported and tested due to limits to my spare time. The code is written to be C++11 complient, but I would love to add version specific functionality such as what is available in C++17 and C++20.

## Dependencies

libSock is a very minimal library with only a few dependencies so far. Unfortunately they are both required at this time, but ideally they would both be made optional based on what functionality the user desires.

- pthread
- openssl >= 1.1.0

## Examples

Currently there are only two examples which are somewhat done, and both are quite simple.

The first one, is a very simple echo server where the user inputs anything with a character length less than 256, the server reads it, prints it, and sends it back.

The second one is a polling server which can accept any number of concurrent connections. It will poll for incoming connections and data and process them appropriately. The server is stopped when it receives the phrase `stop`.

- [Example 0](/examples/0)
- [Example 1](/examples/1)
