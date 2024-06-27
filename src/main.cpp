#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <csignal>
#include <fstream>
#include <thread>
#include <cstring>
#include "Cache.h"
#include "Server.h"
#include "ConsistentHash.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

Server* global_server = nullptr;

void signal_handler(int signal) {
    std::cout << "\nCaught signal " << signal << ". Shutting down gracefully...\n";
    if (global_server) {
        global_server->stop();
    }
    exit(0);
}

void load_persistence(Cache& cache) {
    std::ifstream snap("snapshot.bin");
    if (snap.good()) {
        std::cout << "Loading snapshot...\n";
        cache.load_snapshot("snapshot.bin");
    }
    std::ifstream aof("aof.log");
    if (aof.good()) {
        std::cout << "Replaying AOF...\n";
        std::string cmd, key, val;
        while (aof >> cmd >> key) {
            if (cmd == "SET") {
                aof >> val;
                cache.set(key, val);
            } else if (cmd == "DEL") {
                cache.del(key);
            }
        }
    }
}

void run_client(const std::vector<int>& cluster_ports) {
    std::cout << "CacheX REPL Client Started. Ports: ";
    for (int p : cluster_ports) std::cout << p << " ";
    std::cout << "\n";

#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    ConsistentHash hasher;
    for (int p : cluster_ports) {
        hasher.add_node("127.0.0.1:" + std::to_string(p));
    }

    std::string line;
    while (true) {
        std::cout << "cachex> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string cmd, key;
        iss >> cmd;
        if (cmd != "STATS" && cmd != "SUBSCRIBE" && cmd != "PUBLISH") {
            iss >> key;
        }

        std::string target_node = hasher.get_node(key.empty() ? cmd : key);
        if (target_node.empty()) {
            std::cout << "No nodes available.\n";
            continue;
        }
        
        int port = std::stoi(target_node.substr(target_node.find(":") + 1));
        
#ifdef _WIN32
        int sock = socket(AF_INET, SOCK_STREAM, 0);
#else
        int sock = socket(AF_INET, SOCK_STREAM, 0);
#endif
        sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

        if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cout << "Could not connect to node " << target_node << "\n";
            continue;
        }

        std::string request = line + "\n";
        send(sock, request.c_str(), request.length(), 0);
        
        char buffer[1024];
        memset(buffer, 0, 1024);
        recv(sock, buffer, 1024, 0);
        std::cout << buffer;

#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
    }
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);

    bool is_server = false;
    int port = 6379;
    std::string replica_host = "";
    int replica_port = 0;
    std::vector<int> cluster_ports = {6379};

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--server") {
            is_server = true;
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--replicaof" && i + 2 < argc) {
            replica_host = argv[++i];
            replica_port = std::stoi(argv[++i]);
        } else if (arg == "--cluster" && i + 1 < argc) {
            cluster_ports.clear();
            std::string ports = argv[++i];
            std::stringstream ss(ports);
            std::string p;
            while (std::getline(ss, p, ',')) {
                cluster_ports.push_back(std::stoi(p));
            }
        }
    }

    if (is_server) {
        Cache cache;
        load_persistence(cache);
        Server server(port, cache);
        global_server = &server;
        if (!replica_host.empty()) {
            server.set_replicaof(replica_host, replica_port);
        }
        server.start();
    } else {
        run_client(cluster_ports);
    }

    return 0;
}
