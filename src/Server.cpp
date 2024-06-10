#include "Server.h"
#include "ThreadPool.h"
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

Server::Server(int port, Cache& cache) : port(port), cache(cache) {}

void Server::start() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
#else
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
#endif
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);
    std::cout << "Server listening on port " << port << std::endl;
    
    ThreadPool pool(4); // 4 worker threads
    
    while(true) {
        sockaddr_in client_addr;
#ifdef _WIN32
        int addrlen = sizeof(client_addr);
        int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
#else
        socklen_t addrlen = sizeof(client_addr);
        int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
#endif
        pool.enqueue([this, client_socket] {
            this->handle_client(client_socket);
        });
    }
}

void Server::handle_client(int client_socket) {
    char buffer[1024] = {0};
#ifdef _WIN32
    recv(client_socket, buffer, 1024, 0);
#else
    read(client_socket, buffer, 1024);
#endif
    std::istringstream iss(buffer);
    std::string cmd;
    iss >> cmd;
    std::string response = "OK\\n";
    if (cmd == "SET") {
        std::string key, val;
        iss >> key >> val;
        cache.set(key, val);
    } else if (cmd == "GET") {
        std::string key;
        iss >> key;
        std::string val = cache.get(key);
        response = val.empty() ? "(nil)\\n" : val + "\\n";
    }
#ifdef _WIN32
    send(client_socket, response.c_str(), response.length(), 0);
    closesocket(client_socket);
#else
    send(client_socket, response.c_str(), response.length(), 0);
    close(client_socket);
#endif
}

