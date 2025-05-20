// TCPServer.hpp

#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include "CommandTimer.hpp"

#define COMMAND_PORT_DEFAULT 8124

class TCPServer {
public:
    TCPServer() : sockfd(-1), clientfd(-1), timer(nullptr) {}

    ~TCPServer() {
        shutdown();
    }

    void setCommandTimer(CommandTimer* commandTimer) {
        this->timer = commandTimer;
    }

    bool start(int port = COMMAND_PORT_DEFAULT) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("socket");
            return false;
        }

        int opt = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in serv_addr {};
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);

        if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("bind");
            return false;
        }

        if (listen(sockfd, 1) < 0) {
            perror("listen");
            return false;
        }

        std::cout << "TCPServer listening on port " << port << "..." << std::endl;
        acceptLoop();
        return true;
    }

private:
    int sockfd;
    int clientfd;
    CommandTimer* timer;

    void acceptLoop() {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
        if (clientfd < 0) {
            perror("accept");
            return;
        }

        std::cout << "Client connected." << std::endl;

        char buffer[1024];
        std::string lineBuffer;

        while (true) {
            ssize_t bytesRead = read(clientfd, buffer, sizeof(buffer));
            if (bytesRead <= 0) {
                std::cout << "Client disconnected or read error." << std::endl;
                break;
            }

            for (ssize_t i = 0; i < bytesRead; ++i) {
                char c = buffer[i];
                if (c == '\n') {
                    processLine(lineBuffer);
                    lineBuffer.clear();
                } else if (c != '\r') {
                    lineBuffer += c;
                }
            }
        }

        close(clientfd);
        clientfd = -1;
    }

    void processLine(const std::string& line) {
        if (!timer) return;

        int seconds;
        std::istringstream iss(line);
        if (!(iss >> seconds)) {
            std::cerr << "invalid command: " << line << std::endl;
            return;
        }

        if (seconds == -1) {
            timer->cancel();
        } else if (seconds > 0) {
            timer->schedule(seconds);
        } else {
            std::cerr << "ignored non-positive command: " << line << std::endl;
        }
    }

    void shutdown() {
        if (clientfd != -1) close(clientfd);
        if (sockfd != -1) close(sockfd);
        clientfd = -1;
        sockfd = -1;
    }
};

#endif // TCP_SERVER_HPP

