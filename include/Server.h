#pragma once
#include "Cache.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>

class Server {
private:
    int server_fd;
    int port;
    Cache& cache;
    std::unordered_map<std::string, std::vector<int>> subscribers;
    std::mutex sub_mtx;
    std::atomic<bool> running;
    
    std::vector<int> replica_sockets;

    std::string master_host;
    int master_port;
    bool is_replica = false;

    void connect_to_master();
    
public:
    Server(int port, Cache& cache);
    void start();
    void stop();
    void handle_client(int client_socket);
    void set_replicaof(const std::string& host, int port);
};
