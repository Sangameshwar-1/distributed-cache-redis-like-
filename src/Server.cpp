#include "Server.h"
#include "ThreadPool.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <thread>

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

Server::Server(int port, Cache& cache) : port(port), cache(cache), running(false) {}

void Server::set_replicaof(const std::string& host, int port) {
    master_host = host;
    master_port = port;
    is_replica = true;
}

void Server::connect_to_master() {
    while (running) {
        std::cout << "Connecting to master " << master_host << ":" << master_port << std::endl;
#ifdef _WIN32
        int sock = socket(AF_INET, SOCK_STREAM, 0);
#else
        int sock = socket(AF_INET, SOCK_STREAM, 0);
#endif
        sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(master_port);
        inet_pton(AF_INET, master_host.c_str(), &serv_addr.sin_addr);

        if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) >= 0) {
            std::cout << "Connected to master, sending SYNC" << std::endl;
            std::string sync_cmd = "SYNC\n";
            send(sock, sync_cmd.c_str(), sync_cmd.length(), 0);
            
            char buffer[1024];
            while (running) {
                memset(buffer, 0, 1024);
                int bytes = recv(sock, buffer, 1024, 0);
                if (bytes <= 0) break;
                
                std::istringstream iss(buffer);
                std::string cmd;
                iss >> cmd;
                if (cmd == "SET") {
                    std::string key, val;
                    iss >> key >> val;
                    cache.set(key, val, 0);
                } else if (cmd == "DEL") {
                    std::string key;
                    iss >> key;
                    cache.del(key);
                }
            }
        }
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        std::this_thread::sleep_for(std::chrono::seconds(5)); // reconnect delay
    }
}

void Server::stop() {
    running = false;
    cache.snapshot("snapshot.bin");
    std::cout << "Graceful shutdown complete. Snapshot saved." << std::endl;
}

void Server::start() {
    running = true;
    if (is_replica) {
        std::thread(&Server::connect_to_master, this).detach();
    }

#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
#else
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);
    std::cout << "Server listening on port " << port << std::endl;
    
    ThreadPool pool(4);
    
    while(running) {
        sockaddr_in client_addr;
#ifdef _WIN32
        int addrlen = sizeof(client_addr);
        int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
#else
        socklen_t addrlen = sizeof(client_addr);
        int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
#endif
        if (client_socket >= 0) {
            pool.enqueue([this, client_socket] {
                this->handle_client(client_socket);
            });
        }
    }
}

void Server::handle_client(int client_socket) {
    char buffer[1024];
    while (running) {
        memset(buffer, 0, 1024);
        int valread = recv(client_socket, buffer, 1024, 0);
        if (valread <= 0) break; // disconnect
        
        std::istringstream iss(buffer);
        std::string cmd;
        iss >> cmd;
        std::string response = "OK\n";
        
        if (cmd == "SET") {
            std::string key, val;
            iss >> key >> val;
            std::string ex, ttl_str; int ttl = 0; 
            if (iss >> ex >> ttl_str && ex == "EX") ttl = std::stoi(ttl_str); 
            cache.set(key, val, ttl);
        } else if (cmd == "GET") {
            std::string key;
            iss >> key;
            std::string val = cache.get(key);
            response = val.empty() ? "(nil)\n" : val + "\n";
        } else if (cmd == "DEL" || cmd == "DELETE") {
            std::string key;
            iss >> key;
            cache.del(key);
        } else if (cmd == "SUBSCRIBE") {
            std::string channel;
            iss >> channel;
            std::lock_guard<std::mutex> lock(sub_mtx);
            subscribers[channel].push_back(client_socket);
            response = "SUBSCRIBED\n";
        } else if (cmd == "PUBLISH") {
            std::string channel, msg;
            iss >> channel >> msg;
            std::lock_guard<std::mutex> lock(sub_mtx);
            for (int sock : subscribers[channel]) {
                std::string pub_msg = "MESSAGE " + channel + " " + msg + "\n";
                send(sock, pub_msg.c_str(), pub_msg.length(), 0);
            }
            response = "PUBLISHED\n";
        } else if (cmd == "STATS") {
            response = "Hits: " + std::to_string(cache.hits) + "\nMisses: " + std::to_string(cache.misses) + "\nWrites: " + std::to_string(cache.writes) + "\n";
        } else if (cmd == "SYNC") {
            // Replication sync request
            cache.snapshot("snapshot.bin");
            replica_sockets.push_back(client_socket);
            response = "SYNCED\n";
            send(client_socket, response.c_str(), response.length(), 0);
            continue; // Keep connection open for replica updates
        } else if (cmd == "exit" || cmd == "quit") {
            break;
        } else {
            response = "Unknown command\n";
        }
        
        if (cmd == "SET" || cmd == "DEL" || cmd == "DELETE") {
            for (int r_sock : replica_sockets) {
                send(r_sock, buffer, valread, 0);
            }
        }

        send(client_socket, response.c_str(), response.length(), 0);
    }
#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif
}
