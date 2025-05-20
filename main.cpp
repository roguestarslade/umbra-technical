#include "TCPServer.hpp"
#include "CommandTimer.hpp"
#include <iostream>
#include <cstdlib> // for std::atoi

int main(int argc, char* argv[]) {
    int port = COMMAND_PORT_DEFAULT;

    if (argc > 1) {
        port = std::atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            std::cerr << " Invalid port number: " << argv[1] << std::endl;
            return 1;
        }
    }

    CommandTimer timer;
    TCPServer server;

    server.setCommandTimer(&timer);

    if (!server.start(port)) {
        std::cerr << " Failed to start TCP server on port " << port << "." << std::endl;
        return 1;
    }

    return 0;
}
