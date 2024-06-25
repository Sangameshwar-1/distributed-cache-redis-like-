#pragma once
#include "Cache.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

class Server {
private:
    int server_fd;
    int port;
    Cache& cache;
    std::unordered_map<std::string, std::vector<int>> subscribers;
    std::mutex sub_mtx;
public:
    Server(int port, Cache& cache);
    void start();
    void handle_client(int client_socket);
};


