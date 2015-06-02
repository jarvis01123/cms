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


using asio::ip::tcp;

int main(int argc, char const *argv[]) {



    trading::market market;
    trading::order test;
    std::string message;
    std::string output;


    asio::io_service io_service;
    asio::io_service::work work(io_service);
    int port_no = 2000;

    market_server ms(io_service, port_no);

    std::vector<std::thread> threadPool;

    for(size_t t = 0; t < std::thread::hardware_concurrency(); t++){
        threadPool.push_back(thread([&] { io_service.run();  } ));
    }

    io_service.run();

    io_service.stop();
    for(std::thread& t : threadPool) {
        t.join();
    }
    // while (getline(std::cin, message)) {
    //
    //     if (message.length() > my::MAX) {
    //         std::cout << "Message too long" << std::endl;
    //         exit(1);
    //     }
    //
    //     output = market.str_exec(message);
    //     std::cout << output << std::endl;
    // }

    return 0;
}
