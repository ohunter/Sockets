#include "../include/server.hpp"

int main(int argc, char* argv[])
{
        Sockets::Server s("127.0.0.1", 54321);

        s.serve();
}