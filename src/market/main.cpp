#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
#include <functional>
#include "market.h"
#include <asio.hpp>
#include <thread>
#include <chrono>
#include <future>
#include <cstring>
#include "../server/async_server.h"

using asio::ip::tcp;

int main(int argc, char const *argv[]) {

    bool STANDARD = false, EXT1 = false, EXT2 = false;
    int port_no;

    if (argc <= 2) {
        STANDARD = true;
    } else if (strcmp(argv[2], "ex1") == 0 ) {
        std::cout << "ext 1" << std::endl;
        EXT1 = true;
    } else if (strcmp(argv[2], "ex2") == 0) {
        std::cout << "ext 2" << std::endl;
        EXT2 = true;
    } else {
        std::cout << "usage: cms [<ex1>|<ex2>]" << std::endl;
        exit(1);
    }

    try {
        port_no = (int)atoi(argv[1]);
    } catch (...) {
        std::cout << "usage: cms [<ex1>|<ex2>]" << std::endl;
        exit(1);
    }

    trading::market market;
    asio::io_service io_service;

    servers::server_pointer market_server = servers::async_server::create(
        io_service, port_no, market);


    std::string message;
    std::string output;

    if (STANDARD) {

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

    } else { // EX2

        // no limit on connections
        market_server->start();
    }

    return 0;
}
