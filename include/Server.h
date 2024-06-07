#pragma once
#include "Cache.h"
#include <string>

class Server {
private:
    int server_fd;
    int port;
    Cache& cache;
public:
    Server(int port, Cache& cache);
    void start();
    void handle_client(int client_socket);
};

