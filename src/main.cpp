#include <iostream>
#include <string>
#include "market/market.h"
#include <asio.hpp>
#include <cstring>
#include "server/async_server.h"

using asio::ip::tcp;

int main(int argc, char const *argv[]) {

    bool BASE = false, EXT1 = false, EXT2 = false;
    int port_no = -1;

    if (strcmp(argv[1], "base") == 0 ) {
        BASE = true;
    } else if (strcmp(argv[1], "ext1") == 0 ) {
        EXT1 = true;
    } else if (strcmp(argv[1], "ext2") == 0) {
        EXT2 = true;
    } else {
        std::cout << "usage: cms [<ex1>|<ex2>]" << std::endl;
        exit(1);
    }

    trading::market market;
    asio::io_service io_service;
    servers::server_pointer market_server;
    std::string message;
    std::string output;

    if (!BASE) {
        try {
            port_no = (int)atoi(argv[2]);
            market_server = servers::async_server::create(io_service, port_no, market);
        } catch (...) {
            std::cout << "usage: cms [<ex1>|<ex2>]" << std::endl;
            exit(1);
        }
    }

    if (BASE) {

        while (getline(std::cin, message)) {

            if (message.length() > 256) {
                std::cout << "Message too long" << std::endl;
                exit(1);
            }

            output = market.execute(message);
            std::cout << output << std::endl;
        }

    } else if (EXT1) {

        // limit connections to 1
        market_server->set_max_connections(1);

        // kick off server to accept tcp connections
        market_server->start();

    } else { // EXT2

        // no limit on connections
        market_server->start();
    }

    return 0;
}
