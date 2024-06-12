#include <iostream>
#include <string>
#include "Cache.h"
#include "Server.h"

int main(int argc, char* argv[]) {
    Cache cache;
    if (argc > 1 && std::string(argv[1]) == "--server") {
        Server server(6379, cache);
        server.start();
    } else {
        std::cout << "Run with --server to start TCP server. Use telnet for now.\\n";
    }
    return 0;
}


